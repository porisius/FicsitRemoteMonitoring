#include "Communication.h"

#include "FGBuildableCircuitSwitch.h"
#include "FGChatManager.h"
#include "FGPlayerController.h"
#include "FRM_Request.h"

struct FChatMessageStruct;

void UCommunication::getChatMessages(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TArray<FChatMessageStruct> Messages;

	AFGChatManager::Get(WorldContext->GetWorld())->GetReceivedChatMessages(Messages);

	const int64 ServerTimestamp = FMath::RoundToInt64(UGameplayStatics::GetTimeSeconds(WorldContext));
	const int64 CurrentTimestamp = FDateTime::UtcNow().ToUnixTimestamp();
	
	for (const FChatMessageStruct& ChatMessage : Messages)
	{
		TSharedPtr<FJsonObject> JChatMessage = MakeShared<FJsonObject>();
		JChatMessage->Values.Add("TimeStamp", MakeShared<FJsonValueNumber>(CurrentTimestamp - ServerTimestamp + FMath::RoundToInt64(ChatMessage.ServerTimeStamp)));
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

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JChatMessage));
	}
}

void UCommunication::createPing(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (RequestData.Body.Num() == 0) return;

	AFGPlayerController* PlayerController = Cast<AFGPlayerController>(WorldContext->GetWorld()->GetFirstPlayerController());
	if (!PlayerController)
	{
		OutJsonArray.Add(MakeShared<FJsonValueObject>(UFRM_RequestLibrary::GenerateError("No player connected.")));

		return;
	}

	for (const TSharedPtr<FJsonValue>& BodyObject : RequestData.Body)
	{
		TSharedPtr<FJsonObject> JsonObject = BodyObject->AsObject();

		if (!JsonObject->HasField("x") || !JsonObject->HasField("y") || !JsonObject->HasField("z"))
		{
			OutJsonArray.Add(MakeShared<FJsonValueObject>(UFRM_RequestLibrary::GenerateError("Missing field x, y or z.")));
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
}

void UCommunication::sendChatMessage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (RequestData.Body.Num() == 0) return;

	UWorld* World = WorldContext->GetWorld();
	AFGChatManager* ChatManager = AFGChatManager::Get(World);

	for (const TSharedPtr<FJsonValue>& BodyObject : RequestData.Body)
	{
		TSharedPtr<FJsonObject> JsonObject = BodyObject->AsObject();

		// build error message if no message is available
		if (!JsonObject->HasField("message"))
		{
			const TSharedPtr<FJsonObject> JChatMessage = UFRM_RequestLibrary::GenerateError("Missing field message.");
			JChatMessage->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(false));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JChatMessage));
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
				const TSharedPtr<FJsonObject> JChatMessage = UFRM_RequestLibrary::GenerateError("Field \"color\" is invalid.");
				JChatMessage->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(false));
				OutJsonArray.Add(MakeShared<FJsonValueObject>(JChatMessage));
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

		TSharedPtr<FJsonObject> JChatMessage = MakeShared<FJsonObject>();
		JChatMessage->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(true));
		JChatMessage->Values.Add("Message", MakeShared<FJsonValueString>(MessageStruct.MessageText.ToString()));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JChatMessage));
	}
}

void UCommunication::setEnabled(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (RequestData.Body.Num() == 0) return;

	TArray<AFGBuildableFactory*> FactoryBuildings;
	TArray<AFGBuildableCircuitSwitch*> CircuitSwitches;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFactory>(FactoryBuildings);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(CircuitSwitches);
	
	for (const auto& BodyObject : RequestData.Body)
	{
		FString RequestedBuildable;
		auto JsonObject = BodyObject->AsObject();
		if (UFRM_RequestLibrary::TryGetStringField(JsonObject, "ID", RequestedBuildable, OutJsonArray)) continue;

		// check if ID or status is present in this json object
		if (!JsonObject->HasField("status"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field status.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		bool foundBuildable = false;

		bool PowerState = JsonObject->GetBoolField("status");
		
		for (AFGBuildableFactory* FactoryBuilding : FactoryBuildings)
		{
			if (FactoryBuilding && FactoryBuilding->GetName() == RequestedBuildable)
			{
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
				//Inverted PowerState as preferred True = On, False = Off. AFGBuildableFactory uses IsProductionPaused, True = Off, False = On
				FactoryBuilding->SetIsProductionPaused(!PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(!FactoryBuilding->IsProductionPaused()));
				OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));

				foundBuildable = true;
			}

			// If found start on exit for
			if (foundBuildable) { break; }
		}

		// If found start on next RequestedBuildable
		if (foundBuildable) { continue; }
		
		for (AFGBuildableCircuitSwitch* CircuitSwitch : CircuitSwitches)
		{
			if (CircuitSwitch && CircuitSwitch->GetName() == RequestedBuildable)
			{
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();

				CircuitSwitch->SetSwitchOn(PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(CircuitSwitch->IsSwitchOn()));
				OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
				
				foundBuildable = true;
			}

			// If found start on exit for
			if (foundBuildable) { break; }
		}
	}
};

void UCommunication::setModSetting(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (RequestData.Body.Num() == 0) return;
	
	for (const auto& BodyObject : RequestData.Body)
	{
		auto JsonObject = BodyObject->AsObject();

		// check if SplineSampleDistance is present in this json object
		if (!JsonObject->HasField("SplineSampleDistance"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field status.");
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		float SplineSampleDistance = 0.f;
		if (!JsonObject->TryGetNumberField("SplineSampleDistance", SplineSampleDistance))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("SplineSampleDistance is invalid. Requires float/integer");
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("SplineSampleDistance: %f"), SplineSampleDistance);

		const FString& cvar = "FicsitRemoteMonitoring.General.SplineSampleDistance";
		
		USessionSettingsManager* SessionSettings = WorldContext->GetWorld()->GetSubsystem<USessionSettingsManager>();
		SessionSettings->SetFloatOptionValue(cvar, SplineSampleDistance);

		TSharedPtr<FJsonObject> JChatMessage = MakeShared<FJsonObject>();
		JChatMessage->Values.Add("IsSent", MakeShared<FJsonValueBoolean>(true));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JChatMessage));
		
	}
};