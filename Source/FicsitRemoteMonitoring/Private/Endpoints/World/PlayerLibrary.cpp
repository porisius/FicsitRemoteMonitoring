#include "Endpoints/World/PlayerLibrary.h"

#include "FGCharacterPlayer.h"
#include "FGCreatureSubsystem.h"
#include "FGPlayerController.h"
#include "FGPlayerState.h"
#include "FGSporeFlower.h"
#include "FicsitRemoteMonitoring.h"
#include "RemoteMonitoringLibrary.h"
#include "Kismet/GameplayStatics.h"

namespace {
	// Hand-built offline player entry (PLYR-03/D-01). The shared JSON helpers
	// (CreateBaseJsonObject / getActorJSON / getActorFeaturesJSON) all dereference
	// their actor argument unconditionally on the first line, so a disconnected
	// player (no actor) must be assembled field-by-field here instead of calling
	// them with a null/placeholder actor (03-RESEARCH.md Pattern 3 / Pitfall 2).
	//
	// Default convention (documented for the SUMMARY): no last-known state is
	// cached or replayed — every live-only field gets a neutral zero/false/empty
	// default, matching the live entry's field names/types exactly. Name is the
	// only field sourced from real data (the connect-time cache).
	TSharedPtr<FJsonObject> BuildOfflinePlayerJson(const FString& UserID, const FString& CachedName) {
		TSharedPtr<FJsonObject> JPlayer = MakeShared<FJsonObject>();
		JPlayer->Values.Add("ID", MakeShared<FJsonValueString>(UserID));
		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(CachedName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(AFGCharacterPlayer::StaticClass()->GetName()));

		TSharedPtr<FJsonObject> Location = MakeShared<FJsonObject>();
		Location->Values.Add("x", MakeShared<FJsonValueNumber>(0));
		Location->Values.Add("y", MakeShared<FJsonValueNumber>(0));
		Location->Values.Add("z", MakeShared<FJsonValueNumber>(0));
		Location->Values.Add("rotation", MakeShared<FJsonValueNumber>(0));
		Location->Values.Add("pitch", MakeShared<FJsonValueNumber>(0));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(Location));

		JPlayer->Values.Add("Speed", MakeShared<FJsonValueNumber>(0));
		JPlayer->Values.Add("Online", MakeShared<FJsonValueBoolean>(false));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(0));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(false));
		JPlayer->Values.Add("Inventory", MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{}));

		// features: mirror getActorFeaturesJSON's shape (RemoteMonitoringLibrary.cpp:215-244)
		// field-for-field, with zeroed/defaulted coordinates instead of a live actor location.
		TSharedPtr<FJsonObject> Properties = MakeShared<FJsonObject>();
		Properties->Values.Add("name", MakeShared<FJsonValueString>(CachedName));
		Properties->Values.Add("type", MakeShared<FJsonValueString>(TEXT("Player")));

		TSharedPtr<FJsonObject> Coordinates = MakeShared<FJsonObject>();
		Coordinates->Values.Add("x", MakeShared<FJsonValueNumber>(0));
		Coordinates->Values.Add("y", MakeShared<FJsonValueNumber>(0));
		Coordinates->Values.Add("z", MakeShared<FJsonValueNumber>(0));

		TSharedPtr<FJsonObject> Geometry = MakeShared<FJsonObject>();
		Geometry->Values.Add("coordinates", MakeShared<FJsonValueObject>(Coordinates));
		Geometry->Values.Add("type", MakeShared<FJsonValueString>(TEXT("Point")));

		TSharedPtr<FJsonObject> Features = MakeShared<FJsonObject>();
		Features->Values.Add("properties", MakeShared<FJsonValueObject>(Properties));
		Features->Values.Add("geometry", MakeShared<FJsonValueObject>(Geometry));
		JPlayer->Values.Add("features", MakeShared<FJsonValueObject>(Features));

		return JPlayer;
	}
}

void UPlayerLibrary::getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;

	// Stable GetUserID()s already emitted by the live loop below — the offline
	// enumeration loop (PlayerNameCache) skips any ID present here so a
	// live-and-cached player appears exactly once (PLYR-03/Pitfall 4).
	TSet<FString> VisitedIDs;

	// Subsystem pointer used both for the opportunistic in-loop cache refresh and
	// the offline-enumeration loop after it. IsValid()-guarded (never fgcheck) —
	// the subsystem can legitimately be null during world teardown/startup.
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	const bool bHasValidSubsystem = IsValid(ModSubsystem);

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGCharacterPlayer::StaticClass(), FoundActors);
	for (AActor* Player : FoundActors) {
		TSharedPtr<FJsonObject> JPlayer = CreateBaseJsonObject(Player);

		AFGCharacterPlayer* PlayerCharacter = Cast<AFGCharacterPlayer>(Player);

		FString PlayerName = GetPlayerName(PlayerCharacter);

		// Safe defaults for every field that depends on a component that can
		// legitimately be null while a player is mid-disconnect (PLYR-02/D-02).
		TArray<TSharedPtr<FJsonValue>> InventoryJsonArray;
		float Speed = 0.0f;
		bool bOnline = false;
		float PlayerHP = 0.0f;
		bool bDead = false;

		// Camera/look pitch (PLYR-01/D-03), signed -90..90, sourced from the
		// PlayerController's control rotation — NOT actor body rotation.
		// Defaults to 0.0 when there is no valid controller (mid-disconnect).
		float Pitch = 0.0f;

		if (IsValid(PlayerCharacter)) {
			Speed = PlayerCharacter->GetVelocity().Length() * 0.036;
			bOnline = PlayerCharacter->IsPlayerOnline();

			UFGInventoryComponent* PlayerInventoryComponent = PlayerCharacter->GetInventory();
			if (IsValid(PlayerInventoryComponent)) {
				TArray<FInventoryStack> InventoryStacks;
				PlayerInventoryComponent->GetInventoryStacks(InventoryStacks);
				TMap<TSubclassOf<UFGItemDescriptor>, int32> PlayerInventory = GetGroupedInventoryItems(InventoryStacks);
				InventoryJsonArray = GetInventoryJSON(PlayerInventory);
			}

			UFGHealthComponent* PlayerHealthComponent = PlayerCharacter->GetHealthComponent();
			if (IsValid(PlayerHealthComponent)) {
				PlayerHP = PlayerHealthComponent->GetCurrentHealth();
				bDead = PlayerHealthComponent->IsDead();
			}

			AFGPlayerController* PlayerController = PlayerCharacter->GetFGPlayerController();
			if (IsValid(PlayerController)) {
				float RawPitch = PlayerController->GetControlRotation().Pitch;
				if (RawPitch > 180.f) {
					RawPitch -= 360.f;
				}
				Pitch = FMath::Clamp(RawPitch, -90.f, 90.f);
			}

			// Dedupe (Pitfall 4) + opportunistic name-cache refresh (A1 mitigation,
			// Open Question 3): record this player's stable ID so the offline
			// enumeration loop below skips them, and refresh PlayerNameCache with
			// the name already computed here in case it was empty at PostLogin time.
			const APlayerState* PlayerStateBase = PlayerCharacter->GetPlayerState();
			if (IsValid(PlayerStateBase)) {
				const AFGPlayerState* PlayerState = Cast<AFGPlayerState>(PlayerStateBase);
				if (IsValid(PlayerState)) {
					const FString UserID = PlayerState->GetUserID();
					if (!UserID.IsEmpty()) {
						VisitedIDs.Add(UserID);
						if (bHasValidSubsystem) {
							ModSubsystem->PlayerNameCache.Add(UserID, PlayerName);
						}
					}
				}
			}
		}

		// getPlayer-local injection into the object getActorJSON returns — the
		// shared helper itself is NOT modified (D-03 blast-radius limit).
		TSharedPtr<FJsonObject> LocationJson = getActorJSON(Player);
		LocationJson->Values.Add("pitch", MakeShared<FJsonValueNumber>(Pitch));

		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(PlayerName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(Player->GetClass()->GetName()));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(LocationJson));
		JPlayer->Values.Add("Speed", MakeShared<FJsonValueNumber>(Speed));
		JPlayer->Values.Add("Online", MakeShared<FJsonValueBoolean>(bOnline));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(PlayerHP));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(bDead));
		JPlayer->Values.Add("Inventory", MakeShared<FJsonValueArray>(InventoryJsonArray));
		JPlayer->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Player, PlayerName, "Player")));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPlayer));
	};

	// Offline enumeration (PLYR-03/D-01): every cached player NOT visited by the
	// live loop above gets a hand-built offline entry (Pattern 3) — never via
	// CreateBaseJsonObject/getActorJSON/getActorFeaturesJSON, which would
	// dereference a null actor.
	if (bHasValidSubsystem) {
		for (const TPair<FString, FString>& CachedPlayer : ModSubsystem->PlayerNameCache) {
			const FString& UserID = CachedPlayer.Key;
			if (VisitedIDs.Contains(UserID)) {
				continue;
			}

			OutJsonArray.Add(MakeShared<FJsonValueObject>(BuildOfflinePlayerJson(UserID, CachedPlayer.Value)));
		}
	}
};

void UPlayerLibrary::getDoggo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* DoggoClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Creature/Wildlife/SpaceRabbit/Char_SpaceRabbit.Char_SpaceRabbit_C"));
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), DoggoClass, FoundActors);
	for (AActor* Doggo : FoundActors) {
		TSharedPtr<FJsonObject> JDoggo = CreateBaseJsonObject(Doggo);

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
		fgcheck(ModSubsystem);

		FString DisplayName;

		// get doggo inventory and display name
		TArray<FInventoryStack> InventoryStacks;
		ModSubsystem->GetDoggoInfo_BIE(Doggo, DisplayName, InventoryStacks);
		TMap<TSubclassOf<UFGItemDescriptor>, int32> DoggoInventory = GetGroupedInventoryItems(InventoryStacks);

		JDoggo->Values.Add("Name", MakeShared<FJsonValueString>(DisplayName));
		JDoggo->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Doggo->GetClass())));
		JDoggo->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Doggo)));
		JDoggo->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(DoggoInventory)));
		JDoggo->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Doggo, DisplayName, TEXT("Lizard Doggo"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JDoggo));
	};
};

void UPlayerLibrary::getCreatures(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	AFGCreatureSubsystem* CreatureSubsystem = AFGCreatureSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGCreature*> Creatures = CreatureSubsystem->GetAllCreatures();
	
	for (AFGCreature* Creature : Creatures) {

		TSharedPtr<FJsonObject> JCreature = CreateBaseJsonObject(Creature);
		FString ClassName = UKismetSystemLibrary::GetClassDisplayName(Creature->GetClass());
		
		TSharedPtr<FJsonObject> JRoaming = MakeShared<FJsonObject>();
		JRoaming->SetNumberField("MinRoaming", Creature->GetRoamingDistance().Min);
		JRoaming->SetNumberField("MaxRoaming", Creature->GetRoamingDistance().Max);
		
		JCreature->SetStringField("ClassName", ClassName);
		JCreature->SetObjectField("Roaming", JRoaming);
		JCreature->SetNumberField("SpawnDistance", Creature->GetSpawnDistance());
		JCreature->SetStringField("State", StaticEnum<ECreatureState>()->GetNameStringByValue((int64)Creature->GetCurrentBehaviorState()));
		JCreature->SetObjectField("location", getActorJSON(Creature));
		JCreature->SetObjectField("features", getActorFeaturesJSON(Creature, ClassName, ClassName));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JCreature));
	};
};

void UPlayerLibrary::getHazards(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	UClass* SporeFlowerClass = AFGSporeFlower::StaticClass(); 
	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SporeFlowerClass, FoundActors);
	for (AActor* SporeFlowerActor : FoundActors) {
		
		AFGSporeFlower* SporeFlower = Cast<AFGSporeFlower>(SporeFlowerActor);
		
		TSharedPtr<FJsonObject> JSporeFlower = CreateBaseJsonObject(SporeFlower);

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
		fgcheck(ModSubsystem);

		FString State;
		FString GasState;
		bool bIsSignificant;
		bool bIsDead;

		ModSubsystem->SporeFlower_BIE(SporeFlower, State, GasState, bIsSignificant, bIsDead);

		JSporeFlower->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SporeFlower->GetClass())));
		JSporeFlower->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(SporeFlower)));
		JSporeFlower->Values.Add("State", MakeShared<FJsonValueString>(State));
		JSporeFlower->Values.Add("GasState", MakeShared<FJsonValueString>(GasState));
		JSporeFlower->Values.Add("Significant", MakeShared<FJsonValueBoolean>(bIsSignificant));
		JSporeFlower->Values.Add("Dead", MakeShared<FJsonValueBoolean>(bIsDead));
		JSporeFlower->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(SporeFlower, TEXT("Spore Flower"), TEXT("Spore Flower"))));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSporeFlower));
	};
};