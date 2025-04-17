#include "FRM_World.h"

#include "FGItemPickup.h"
#include "FGResearchTree.h"
#include "FGSchematicCategory.h"
#include "FGResearchManager.h"
#include "FicsitRemoteMonitoring.h"
#include "FRM_Library.h"
#include "FRM_Request.h"
#include "Kismet/GameplayStatics.h"

TArray<TSharedPtr<FJsonValue>> UFRM_World::GetArtifacts(UObject* WorldContext)
{
	UClass* SphereClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Prototype/WAT/BP_WAT2.BP_WAT2_C"));
	UClass* SloopClass = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Prototype/WAT/BP_WAT1.BP_WAT1_C"));

	TArray<AActor*> Pickups;
	TArray<AActor*> Sloops;
	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SphereClass, Pickups);
	UGameplayStatics::GetAllActorsOfClass(WorldContext->GetWorld(), SloopClass, Sloops);

	TArray<TSharedPtr<FJsonValue>> JSlugArray;

	Pickups.Append(Sloops);
	
	for (AActor* Actor : Pickups) {
		const auto Pickup = Cast<AFGItemPickup>(Actor);
		if (!Pickup) continue;
		
		auto PowerSlug = Pickup->GetPickupItems().Item;
		const auto ItemClass = PowerSlug.GetItemClass();
		if (!ItemClass) continue;
		
		TSharedPtr<FJsonObject> JPowerSlug = UFRM_Library::CreateBaseJsonObject(Actor);
		FString SlugName;
		JPowerSlug->Values.Add("Name", MakeShared<FJsonValueString>(SlugName));
		JPowerSlug->Values.Add("NameTest", MakeShared<FJsonValueString>(ItemClass->GetName()));
		JPowerSlug->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Actor->GetClass())));
		JPowerSlug->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Actor)));
		JPowerSlug->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Actor), SlugName, "Artifact")));

		JSlugArray.Add(MakeShared<FJsonValueObject>(JPowerSlug));
	};

	return JSlugArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_World::GetResearchTrees(UObject* WorldContext)
{
	TArray<TSharedPtr<FJsonValue>> JResearchTrees;

	// get the research manager
	const auto ResearchManager = AFGResearchManager::Get(WorldContext);
	if (!ResearchManager) return JResearchTrees;

	// get the mod subsystem, to call the blueprint function
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	// get all reseacrh trees
	TArray<TSubclassOf<UFGResearchTree>> ResearchTrees;
	ResearchManager->GetAllResearchTrees(ResearchTrees);

	for (const auto ResearchTree : ResearchTrees)
	{
		TSharedPtr<FJsonObject> JResearchTree = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JResearchNodes;

		for (const auto ResearchNode : UFGResearchTree::GetNodes(ResearchTree))
		{
			FIntPoint Coordinates;
			TArray<FIntPoint> ParentCoordinates;
			TArray<FIntPoint> UnhiddenByCoordinates;
			TSharedPtr<FJsonObject> JResearchNode = UFRM_Library::CreateBaseJsonObject(ResearchNode);

			ModSubsystem->ResearchTreeNodeUnlockData_BIE(ResearchNode, ParentCoordinates, UnhiddenByCoordinates, Coordinates);

			TSubclassOf<UFGSchematic> NodeSchematic = ResearchNode->GetNodeSchematic();

			// build own coordinates
			TSharedPtr<FJsonObject> JCoordinates = MakeShared<FJsonObject>();
			JCoordinates->Values.Add("x", MakeShared<FJsonValueNumber>(Coordinates[0]));
			JCoordinates->Values.Add("y", MakeShared<FJsonValueNumber>(Coordinates[1]));

			// build parent coordinates
			TArray<TSharedPtr<FJsonValue>> JParents;
			for ( auto Parent : ParentCoordinates)
			{
				TSharedPtr<FJsonObject> JParent = MakeShared<FJsonObject>();
				JParent->Values.Add("x", MakeShared<FJsonValueNumber>(Parent[0]));
				JParent->Values.Add("y", MakeShared<FJsonValueNumber>(Parent[1]));
				JParents.Add(MakeShared<FJsonValueObject>(JParent));
			}

			// build unhidden by coordinates
			TArray<TSharedPtr<FJsonValue>> JUnhiddenByCoordinates;
			for ( auto UnhiddenBy : UnhiddenByCoordinates)
			{
				TSharedPtr<FJsonObject> JUnhiddenBy = MakeShared<FJsonObject>();
				JUnhiddenBy->Values.Add("x", MakeShared<FJsonValueNumber>(UnhiddenBy[0]));
				JUnhiddenBy->Values.Add("y", MakeShared<FJsonValueNumber>(UnhiddenBy[1]));
				JUnhiddenByCoordinates.Add(MakeShared<FJsonValueObject>(JUnhiddenBy));
			}

			// build readable enum string
			FString State = TEXT("Unknown");
			switch (UFGSchematic::GetSchematicState(NodeSchematic, WorldContext))
			{
				case ESchematicState::ESS_Available: State = "Available"; break;
				case ESchematicState::ESS_Hidden: State = "Hidden"; break;
				case ESchematicState::ESS_Locked: State = "Locked"; break;
				case ESchematicState::ESS_Purchased: State = "Purchased"; break;
			}

			// test 1
			auto SchematicCategory = UFGSchematic::GetSchematicCategory(NodeSchematic);

			// end result
			JResearchNode->Values.Add("Name", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDisplayName(NodeSchematic).ToString()));
			JResearchNode->Values.Add("ClassName", MakeShared<FJsonValueString>(ResearchNode->GetName()));
			JResearchNode->Values.Add("Description", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDescription(NodeSchematic).ToString()));
			JResearchNode->Values.Add("Category", MakeShared<FJsonValueString>(UFGSchematicCategory::GetCategoryName(SchematicCategory).ToString()));
			JResearchNode->Values.Add("State", MakeShared<FJsonValueString>(State));
			JResearchNode->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(NodeSchematic)));
			JResearchNode->Values.Add("TimeToComplete", MakeShared<FJsonValueNumber>(UFGSchematic::GetTimeToComplete(NodeSchematic)));
			JResearchNode->Values.Add("Cost", MakeShared<FJsonValueArray>(UFRM_Library::GetInventoryJSON(UFGSchematic::GetCost(NodeSchematic))));
			JResearchNode->Values.Add("UnhiddenBy", MakeShared<FJsonValueArray>(JUnhiddenByCoordinates));
			JResearchNode->Values.Add("Parents", MakeShared<FJsonValueArray>(JParents));
			JResearchNode->Values.Add("Coordinates", MakeShared<FJsonValueObject>(JCoordinates));
			
			JResearchNodes.Add(MakeShared<FJsonValueObject>(JResearchNode));
		}

		JResearchTree->Values.Add("Name", MakeShared<FJsonValueString>(UFGResearchTree::GetDisplayName(ResearchTree).ToString()));
		JResearchTree->Values.Add("Nodes", MakeShared<FJsonValueArray>(JResearchNodes));

		JResearchTrees.Add(MakeShared<FJsonValueObject>(JResearchTree));
	}

	return JResearchTrees;
}

TArray<TSharedPtr<FJsonValue>> UFRM_World::GetChatMessages(UObject* WorldContext)
{
	TArray<TSharedPtr<FJsonValue>> JResponse;
	TArray<FChatMessageStruct> Messages;

	AFGChatManager::Get(WorldContext->GetWorld())->GetReceivedChatMessages(Messages);

	for (const FChatMessageStruct& ChatMessage : Messages)
	{
		TSharedPtr<FJsonObject> JChatMessage = MakeShared<FJsonObject>();
		JChatMessage->Values.Add("ServerTimeStamp", MakeShared<FJsonValueNumber>(ChatMessage.ServerTimeStamp));
		JChatMessage->Values.Add("Sender", MakeShared<FJsonValueString>(ChatMessage.MessageSender.ToString()));

		switch (ChatMessage.MessageType)
		{
		case EFGChatMessageType::CMT_SystemMessage:
			JChatMessage->Values.Add("Type", MakeShared<FJsonValueString>(TEXT("System")));
			break;
		case EFGChatMessageType::CMT_AdaMessage:
			JChatMessage->Values.Add("Type", MakeShared<FJsonValueString>(TEXT("Ada")));
			break;
		case EFGChatMessageType::CMT_PlayerMessage:
			JChatMessage->Values.Add("Type", MakeShared<FJsonValueString>(TEXT("Player")));
			break;
		}

		JChatMessage->Values.Add("Message", MakeShared<FJsonValueString>(ChatMessage.MessageText.ToString()));

		TSharedPtr<FJsonObject> JColor = MakeShared<FJsonObject>();
		JColor->Values.Add("R", MakeShared<FJsonValueNumber>(ChatMessage.MessageSenderColor.R));
		JColor->Values.Add("G", MakeShared<FJsonValueNumber>(ChatMessage.MessageSenderColor.G));
		JColor->Values.Add("B", MakeShared<FJsonValueNumber>(ChatMessage.MessageSenderColor.B));
		JColor->Values.Add("A", MakeShared<FJsonValueNumber>(ChatMessage.MessageSenderColor.A));
		JChatMessage->Values.Add("Color", MakeShared<FJsonValueObject>(JColor));

		JResponse.Add(MakeShared<FJsonValueObject>(JChatMessage));
	}

	return JResponse;
}

TArray<TSharedPtr<FJsonValue>> UFRM_World::CreatePing(UObject* WorldContext, FRequestData RequestData)
{
	TArray<TSharedPtr<FJsonValue>> JResponses;
	if (RequestData.Body.Num() == 0) return JResponses;

	AFGPlayerController* PlayerController = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController());
	if (!PlayerController)
	{
		JResponses.Add(MakeShared<FJsonValueObject>(UFRM_RequestLibrary::GenerateError("No player connected.")));

		return JResponses;
	}

	for (const TSharedPtr<FJsonValue>& BodyObject : RequestData.Body)
	{
		TSharedPtr<FJsonObject> JsonObject = BodyObject->AsObject();

		if (!JsonObject->HasField("x") || !JsonObject->HasField("y") || !JsonObject->HasField("z"))
		{
			JResponses.Add(MakeShared<FJsonValueObject>(UFRM_RequestLibrary::GenerateError("Missing field x, y or z.")));
			continue;
		}

		FVector Location = FVector(0, 0, 0);

		JsonObject->TryGetNumberField("x", Location.X);
		JsonObject->TryGetNumberField("y", Location.Y);
		JsonObject->TryGetNumberField("z", Location.Z);

		FTransform Transform;
		Transform.SetLocation(Location);

		// Using PlayerController to spawn a ping actor that plays sounds.
		// I'm not sure if the game has a limit on the number of pings a player can create.
		// Perhaps we need to modify this and spawn the actor manually
		PlayerController->Server_SpawnAttentionPingActor(Location, Location);
		
		// AFGAttentionPingActor* Actor = WorldContext->GetWorld()->SpawnActorDeferred<AFGAttentionPingActor>(AFGAttentionPingActor::StaticClass(), Transform);
		// Actor->SetOwningPlayerState(PlayerState);
		// UGameplayStatics::FinishSpawningActor(Actor, Transform);
	}

	return JResponses;
}

TArray<TSharedPtr<FJsonValue>> UFRM_World::SendChatMessage(UObject* WorldContext, FRequestData RequestData)
{
	TArray<TSharedPtr<FJsonValue>> JResponses;
	if (RequestData.Body.Num() == 0) return JResponses;

	UWorld* World = WorldContext->GetWorld();
	AFGChatManager* ChatManager = AFGChatManager::Get(World);

	for (const TSharedPtr<FJsonValue>& BodyObject : RequestData.Body)
	{
		TSharedPtr<FJsonObject> JsonObject = BodyObject->AsObject();

		// build error message if no message is available
		if (!JsonObject->HasField("message"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field message.");
			JResponse->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(false));
			JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		FLinearColor Color = FLinearColor::White;
		FString Message;
		FString SenderName = "";

		JsonObject->TryGetStringField("sender", SenderName);
		JsonObject->TryGetStringField("message", Message);

		// get custom color
		if (JsonObject->HasField("color"))
		{
			const TSharedPtr<FJsonObject>* JsonColor;
			JsonObject->TryGetObjectField("color", JsonColor);

			// check if color is valid
			if (!JsonColor || !JsonColor->IsValid())
			{
				const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Field \"color\" is invalid.");
				JResponse->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(false));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
				continue;
			}

			// get all color values
			float r = 1.f;
			float g = 1.f;
			float b = 1.f;
			float a = 1.f;
			JsonColor->Get()->TryGetNumberField("r", r);
			JsonColor->Get()->TryGetNumberField("g", g);
			JsonColor->Get()->TryGetNumberField("b", b);
			JsonColor->Get()->TryGetNumberField("a", a);

			Color = FLinearColor(r, g, b, a);
		}

		FChatMessageStruct MessageStruct;

		// set default message type to system
		MessageStruct.MessageType = EFGChatMessageType::CMT_SystemMessage;

		// set message type to ADA if sender name is ada
		if (SenderName == "ada")
		{
			MessageStruct.MessageType = EFGChatMessageType::CMT_AdaMessage;
		}
		// set player name and set type to "player" message
		else if (!SenderName.IsEmpty())
		{
			MessageStruct.MessageType = EFGChatMessageType::CMT_PlayerMessage;
			MessageStruct.MessageSender = FText::FromString(SenderName.Left(32));
		}

		MessageStruct.MessageText = FText::FromString(Message);
		MessageStruct.ServerTimeStamp = World->TimeSeconds;
		MessageStruct.MessageSenderColor = Color;
		ChatManager->BroadcastChatMessage(MessageStruct, nullptr);

		TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
		JResponse->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(true));
		JResponse->Values.Add("Message", MakeShared<FJsonValueString>(MessageStruct.MessageText.ToString()));
		JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
	}

	return JResponses;
}
