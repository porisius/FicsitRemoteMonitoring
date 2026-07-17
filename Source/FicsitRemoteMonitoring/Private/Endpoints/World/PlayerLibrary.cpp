#include "Endpoints/World/PlayerLibrary.h"

#include "FGCharacterPlayer.h"
#include "FGCreatureSubsystem.h"
#include "FGPlayerController.h"
#include "FGSporeFlower.h"
#include "FicsitRemoteMonitoring.h"
#include "RemoteMonitoringLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPlayerLibrary::getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;

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