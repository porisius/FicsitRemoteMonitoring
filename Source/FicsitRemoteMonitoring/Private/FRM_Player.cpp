#pragma once

#include "FRM_Player.h"
#include <FicsitRemoteMonitoring.h>

TArray<TSharedPtr<FJsonValue>> UFRM_Player::getPlayer(UObject* WorldContext) {

	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JPlayerArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGCharacterPlayer::StaticClass(), FoundActors);
	for (AActor* Player : FoundActors) {
		TSharedPtr<FJsonObject> JPlayer = UFRM_Library::CreateBaseJsonObject(Player);

		AFGCharacterPlayer* PlayerCharacter = Cast<AFGCharacterPlayer>(Player);
		APlayerState* PlayerState = PlayerCharacter->GetPlayerState();
		AFGPlayerState* FGPlayerState = Cast<AFGPlayerState>(PlayerState);

		// get player inventory
		TArray<FInventoryStack> InventoryStacks;
		PlayerCharacter->GetInventory()->GetInventoryStacks(InventoryStacks);
		TMap<TSubclassOf<UFGItemDescriptor>, int32> PlayerInventory = UFRM_Library::GetGroupedInventoryItems(InventoryStacks);

		//TODO: Find way to get player's name when they are offline
		FString PlayerName = PlayerCharacter->mCachedPlayerName;

		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(PlayerName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(Player->GetClass()->GetName()));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Player))); 
		//JPlayer->Values.Add("PlayerID", MakeShared<FJsonValueString>(PlayerState->GetUserID()));
		JPlayer->Values.Add("Online", MakeShared<FJsonValueBoolean>(PlayerCharacter->IsPlayerOnline()));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(PlayerCharacter->GetHealthComponent()->GetCurrentHealth()));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(PlayerCharacter->GetHealthComponent()->IsDead()));
		//JPlayer->Values.Add("PingTime", MakeShared<FJsonValueNumber>(int32::FGPlayerState->GetCompressedPing()));
		JPlayer->Values.Add("Inventory", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(PlayerInventory)));
		JPlayer->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Player, PlayerCharacter->mCachedPlayerName, "Player")));

		JPlayerArray.Add(MakeShared<FJsonValueObject>(JPlayer));
	};

	return JPlayerArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Player::getDoggo(UObject* WorldContext) {

	UClass* DoggoClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Character/Creature/Wildlife/SpaceRabbit/Char_SpaceRabbit.Char_SpaceRabbit_C"));
	TArray<AActor*> FoundActors;
	TArray<TSharedPtr<FJsonValue>> JDoggoArray;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), DoggoClass, FoundActors);
	for (AActor* Doggo : FoundActors) {
		TSharedPtr<FJsonObject> JDoggo = UFRM_Library::CreateBaseJsonObject(Doggo);

		AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
		fgcheck(ModSubsystem);

		FString DisplayName;

		// get doggo inventory and display name
		TArray<FInventoryStack> InventoryStacks;
		ModSubsystem->GetDoggoInfo_BIE(Doggo, DisplayName, InventoryStacks);
		TMap<TSubclassOf<UFGItemDescriptor>, int32> DoggoInventory = UFRM_Library::GetGroupedInventoryItems(InventoryStacks);

		JDoggo->Values.Add("Name", MakeShared<FJsonValueString>(DisplayName));
		JDoggo->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Doggo->GetClass())));
		JDoggo->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Doggo)));
		JDoggo->Values.Add("Inventory", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(DoggoInventory)));
		JDoggo->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Doggo, DisplayName, TEXT("Lizard Doggo"))));

		JDoggoArray.Add(MakeShared<FJsonValueObject>(JDoggo));
	};

	return JDoggoArray;
};