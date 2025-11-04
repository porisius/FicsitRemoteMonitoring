#pragma once

#include "PlayerLibrary.h"

#include "FGCharacterPlayer.h"
#include "FGCreatureSubsystem.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "RemoteMonitoringLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPlayerLibrary::getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGCharacterPlayer::StaticClass(), FoundActors);
	for (AActor* Player : FoundActors) {
		TSharedPtr<FJsonObject> JPlayer = CreateBaseJsonObject(Player);

		AFGCharacterPlayer* PlayerCharacter = Cast<AFGCharacterPlayer>(Player);

		// get player inventory
		TArray<FInventoryStack> InventoryStacks;
		PlayerCharacter->GetInventory()->GetInventoryStacks(InventoryStacks);
		TMap<TSubclassOf<UFGItemDescriptor>, int32> PlayerInventory = GetGroupedInventoryItems(InventoryStacks);

		//TODO: Find way to get player's name when they are offline
		FString PlayerName = GetPlayerName(WorldContext, PlayerCharacter);
		
		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(PlayerName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(Player->GetClass()->GetName()));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Player))); 
		//JPlayer->Values.Add("PlayerID", MakeShared<FJsonValueString>(PlayerState->GetUserID()));
		JPlayer->Values.Add("Speed", MakeShared<FJsonValueNumber>(PlayerCharacter->GetVelocity().Length() * 0.036));
		JPlayer->Values.Add("Online", MakeShared<FJsonValueBoolean>(PlayerCharacter->IsPlayerOnline()));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(PlayerCharacter->GetHealthComponent()->GetCurrentHealth()));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(PlayerCharacter->GetHealthComponent()->IsDead()));
		JPlayer->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(PlayerInventory)));
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