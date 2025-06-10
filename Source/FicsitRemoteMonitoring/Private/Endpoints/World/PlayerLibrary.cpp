#pragma once

#include "PlayerLibrary.h"

#include "FGCharacterPlayer.h"
#include "FGPlayerState.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FRM_Request.h"
#include "RemoteMonitoringLibrary.h"
#include "StructuredLog.h"
#include "Kismet/GameplayStatics.h"

void UPlayerLibrary::getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<AActor*> FoundActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGCharacterPlayer::StaticClass(), FoundActors);
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	
	for (AActor* Player : FoundActors) {
		TSharedPtr<FJsonObject> JPlayer = CreateBaseJsonObject(Player);

		AFGCharacterPlayer* PlayerCharacter = Cast<AFGCharacterPlayer>(Player);

		// get player inventory
		TArray<FInventoryStack> InventoryStacks;
		PlayerCharacter->GetInventory()->GetInventoryStacks(InventoryStacks);
		TMap<TSubclassOf<UFGItemDescriptor>, int32> PlayerInventory = GetGroupedInventoryItems(InventoryStacks);
		AFGPlayerState* PlayerState = PlayerCharacter->GetControllingPlayerState();
		
		//TODO: Find way to get player's name when they are offline
		FString PlayerName = GetPlayerName(PlayerCharacter);

		TArray<TSharedPtr<FJsonValue>> PlayerToDoList;

		TArray<FShoppingList> PlayerToDo;
		ModSubsystem->GetShoppingList_BIE(PlayerState, PlayerToDo);
		for (FShoppingList PlayerToDoItem : PlayerToDo)
		{
			TSharedPtr<FJsonObject> JPlayerToDo = MakeShared<FJsonObject>();

			FString Recipe = UFGRecipe::GetRecipeName(PlayerToDoItem.RecipeClass).ToString();
		
			JPlayerToDo->Values.Add("Recipe", MakeShared<FJsonValueString>(Recipe));
			JPlayerToDo->Values.Add("RecipeClass", MakeShared<FJsonValueString>((PlayerToDoItem.RecipeClass)->GetClassPathName().ToString()));
			JPlayerToDo->Values.Add("Amount", MakeShared<FJsonValueNumber>(PlayerToDoItem.Amount));
			PlayerToDoList.Add(MakeShared<FJsonValueObject>(JPlayerToDo));
		}
		
		JPlayer->Values.Add("Name", MakeShared<FJsonValueString>(PlayerName));
		JPlayer->Values.Add("ClassName", MakeShared<FJsonValueString>(Player->GetClass()->GetName()));
		JPlayer->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Player))); 
		JPlayer->Values.Add("PlayerState", MakeShared<FJsonValueString>(PlayerState->GetName()));
		JPlayer->Values.Add("Online", MakeShared<FJsonValueBoolean>(PlayerCharacter->IsPlayerOnline()));
		JPlayer->Values.Add("PlayerHP", MakeShared<FJsonValueNumber>(PlayerCharacter->GetHealthComponent()->GetCurrentHealth()));
		JPlayer->Values.Add("Dead", MakeShared<FJsonValueBoolean>(PlayerCharacter->GetHealthComponent()->IsDead()));
		JPlayer->Values.Add("Inventory", MakeShared<FJsonValueArray>(GetInventoryJSON(PlayerInventory)));
		JPlayer->Values.Add("ToDoList", MakeShared<FJsonValueArray>(PlayerToDoList));
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

void UPlayerLibrary::setToDoList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<AActor*> PlayerActors;

	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), AFGPlayerState::StaticClass(), PlayerActors);
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	
	for (const auto& BodyObject : RequestData.Body)
	{
		FString RequestedPlayerState;
		auto JsonObject = BodyObject->AsObject();
		if (UFRM_RequestLibrary::TryGetStringField(JsonObject, "ID", RequestedPlayerState, OutJsonArray)) continue;

		UE_LOGFMT(LogFRMDebug, Warning, "Requested PlayerState: {RequestedPlayerState}", RequestedPlayerState);
		
		const TArray<TSharedPtr<FJsonValue>>* RecipesArray;
		// check if ID or status is present in this json object
		if (!JsonObject->TryGetArrayField("recipes", RecipesArray))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field recipes.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedPlayerState));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		FString Action;
		if (!JsonObject->TryGetStringField("action", Action))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field action.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedPlayerState));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		EActionType ActionType = EActionType::Add;
		if (Action.ToLower() == "add")
		{
			ActionType = EActionType::Add;	
		} else if (Action.ToLower() == "remove")
		{
			ActionType = EActionType::Remove;
		} else if (Action.ToLower() == "set")
		{
			ActionType = EActionType::Set;
		} else if (Action.ToLower() == "clear")
		{
			ActionType = EActionType::Clear;
		} else
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Invalid field action.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedPlayerState));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}
		
		for (AActor* PlayerActor : PlayerActors)
		{

			UE_LOGFMT(LogFRMDebug, Warning, "PlayerActor: {PlayerActor}", PlayerActor->GetName());
			if (PlayerActor && PlayerActor->GetName() == RequestedPlayerState)
			{
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
				AFGPlayerState* PlayerState = Cast<AFGPlayerState>(PlayerActor);

				for (const TSharedPtr<FJsonValue>& Recipe : *RecipesArray)
				{
					TSharedPtr<FJsonObject> RecipeObject = Recipe->AsObject();
					if (RecipeObject.IsValid())
					{
						FString ClassName;
						int32 Amount;
						
						if (!RecipeObject->TryGetStringField("class", ClassName))
						{
							JResponse->Values.Add("Error", MakeShared<FJsonValueString>("Missing field class"));
							break;
						};

						UE_LOGFMT(LogFRMDebug, Warning, "Recipe Class: {ClassName}", ClassName);
						
						if (!RecipeObject->TryGetNumberField("amount", Amount))
						{
							JResponse->Values.Add("Error", MakeShared<FJsonValueString>("Missing field amount"));
							break;
						};

						UE_LOGFMT(LogFRMDebug, Warning, "Recipe Amount: {Amount}", Amount);

						bool foundClass = false;

						TSubclassOf<UFGRecipe> SubClass;
						
						for (TObjectIterator<UFGRecipe> It; It; ++It)
						{
							UFGRecipe* Class = *It;
							UE_LOGFMT(LogFRMDebug, Warning, "Checking against UClass: {1}", Class->GetName());
							if (Class->GetName() == ClassName)
							{
								UE_LOGFMT(LogFRMDebug, Warning, "Found UClass: {1}", Class->GetName());
								SubClass = Class->GetClass();
								foundClass = true;
								break;
							}
						}

						if (!foundClass){ break; }
						
						UE_LOGFMT(LogFRMDebug, Warning, "Attempting Update");
						ModSubsystem->UpdateShoppingList_BIE(PlayerState, ActionType, SubClass, Amount);
						
						JResponse->Values.Add("Recipe", MakeShared<FJsonValueString>(ClassName));
						JResponse->Values.Add("Success", MakeShared<FJsonValueBoolean>(true));
						OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
					}
				}
			}
		}
	}	
}