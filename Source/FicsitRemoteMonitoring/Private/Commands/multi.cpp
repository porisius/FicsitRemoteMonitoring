#include "Commands/multi.h"

#include "FGPlayerController.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FRM_Request.h"
#include "Command/CommandSender.h"
#include "Logging/StructuredLog.h"
#include "Libraries/Validation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Libraries/Notifications.h"
#include "Networking/FGServerAPIManager.h"

FChatReturn AFRMCommand::RemoteMonitoringCommand(UObject* WorldContext, UCommandSender* Sender, TArray<FString> Arguments) {
	FChatReturn ChatReturn;

	ChatReturn.Color = FLinearColor::Red;
	ChatReturn.Status = EExecutionStatus::UNCOMPLETED;

	int32 argumentsNum = Arguments.Num();

	if (argumentsNum == 0) {
		ChatReturn.Chat = TEXT(
			"Usage:\n"
			"/frm debug <file/info> <Endpoint>\n"
			"/frm http <start/stop>\n"
			"/frm serial <start/stop>\n"
			"/frm config <setting> <value>\n"
			"/frm test <webhook/endpoints>\n"
			"/frm icon"
		);

		return ChatReturn;
	}

	FString command = Arguments[0].ToLower();

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	if (command == "debug") {
		ChatReturn.Chat = TEXT("Usage: /frm debug <file/info> <Endpoint>");

		if (argumentsNum < 3) {
			return ChatReturn;
		}

		FString OutputType = Arguments[1].ToLower();
		FString sEndpoint = Arguments[2];

		bool bSuccess = false;
		int32 ErrorCode = 404;
		FRequestData RequestData = FRequestData();
		RequestData.bIsAuthorized = true;
		FString Json;

		ModSubsystem->HandleEndpoint(sEndpoint, RequestData, bSuccess, ErrorCode, Json, EInterfaceType::Command);

		if (!bSuccess) {
			ChatReturn.Chat = TEXT("Unable to find endpoint, please refer to the documentation at docs.ficsit.app or the mod creator.");

			return ChatReturn;
		}

		if (OutputType == "file") {
			FString JsonPath = FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/Debug/" + sEndpoint + ".json";

			URemoteMonitoringLibrary::FileSaveString(Json, JsonPath);

			ChatReturn.Chat = TEXT("Data saved to Mod location in the Debug folder.");
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}
		else if (OutputType == "info") {
			ChatReturn.Chat = Json;
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}

		return ChatReturn;
	}
	
	if (command == "http" || command == "uws") {
		ChatReturn.Chat = TEXT("Usage: /frm http <start/stop>");

		if (argumentsNum < 2) {
			return ChatReturn;
		}

		FString arg1 = Arguments[1].ToLower();

		const int32 Port = UFRMConfigManager::GetConfigOrDefault<int32>(TEXT("uWS.Port"), 8080);

		FString Reason;
		if (!UFRMValidation::IsTcpPortAvailable(Port, Reason))
		{
			UE_LOG(LogHttpServer, Error, TEXT("Port %d unavailable: %s"), Port, *Reason);

			ChatReturn.Chat = FString(TEXT("Port " + FString::FromInt(Port) + "Unavailable. Reason: " + *Reason));
			ChatReturn.Color = FLinearColor::Red;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}
		
		if (arg1 == "start") {
			
			UE_LOG(LogHttpServer, Log, TEXT("Chat Command: Starting HTTP Service. Port: %d"), Port);
			ModSubsystem->StartWebSocketServer(true);

			ChatReturn.Chat = FString(TEXT("HTTP Service Initiated on Port: " + FString::FromInt(Port)));
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}
		else if (arg1 == "stop") {
			UE_LOG(LogHttpServer, Log, TEXT("Chat Command: Stopping HTTP Service."));
			ModSubsystem->StopWebSocketServer();

			ChatReturn.Chat = TEXT("Stopping HTTP Service.");
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}

		return ChatReturn;
	}
	
	if (command == "config")
	{
		ChatReturn.Chat = TEXT("Usage: /frm config setting value");
				
		if (argumentsNum < 3) {
			return ChatReturn;
		}

		FString arg1 = Arguments[1].ToLower();
		FString arg2 = Arguments[2].ToLower();
	
		AFGPlayerController* PlayerController = Cast<AFGPlayerController>(Sender->GetPlayer()->GetControlledCharacter()->GetLocalViewingPlayerController());
		EPrivilegeLevel PrivilegeLevel = PlayerController->GetDSPrivilegeLevel();
		
		if (PrivilegeLevel != EPrivilegeLevel::Administrator &&
			PrivilegeLevel != EPrivilegeLevel::InitialAdmin &&
			PlayerController->GetNetMode() != NM_ListenServer &&
			PlayerController->GetNetMode() != NM_Standalone)
		{
			ChatReturn.Chat = FString(TEXT("Insufficient Permissions to set " + arg1 + " to " +arg2));
			ChatReturn.Color = FLinearColor::Red;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}		

		if (UFRMConfigManager::SetConfigFromInput(arg1, arg2))
		{
			ChatReturn.Chat = FString(TEXT("Configured " + arg1 + " to " + arg2));
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}
		else
		{
			ChatReturn.Chat = FString(TEXT("Unable to set " + arg1 + " to " +arg2));
			ChatReturn.Color = FLinearColor::Red;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		};

		return ChatReturn;
	}

	if (command == "serial") {
		ChatReturn.Chat = TEXT("Usage: /frm serial <start/stop>");

		if (argumentsNum < 2) {
			return ChatReturn;
		}

		FString arg1 = Arguments[1].ToLower();

		if (arg1 == "start") {
			ModSubsystem->InitSerialDevice();

			const FString Port = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("Serial.Port"), "COM3").TrimStartAndEnd();
			
			ChatReturn.Chat = FString(TEXT("Serial/RS232 Service Initiated on Port: " + Port));
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;

			UE_LOGFMT(LogHttpServer, Log, "Serial/RS232 Service started. Port: {Port}");
		}
		else if (arg1 == "stop") {
			ModSubsystem->StopSerialDevice();
			UE_LOG(LogHttpServer, Log, TEXT("Stopping Serial/RS232 Service."));

			ChatReturn.Chat = TEXT("Stopping Serial/RS232 Service.");
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}

		return ChatReturn;
	}

	if (command == "icon") {
		if (!UKismetSystemLibrary::IsDedicatedServer(WorldContext)) {
			ModSubsystem->IconGenerator_BIE();

			ChatReturn.Chat = TEXT("Icon Generation Completed.");
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}
		else {
			ChatReturn.Chat = TEXT("Dedicated Servers do not have graphical assets to extract. Cancelling... Generate locally, then upload to server.");
		}

		return ChatReturn;
	}

	if (command == "test")
	{
		FString arg1 = Arguments[1].ToLower();
		
		if (arg1 == "webhooks")
		{

			ANotifications* Notifications = ANotifications::Get(this->GetWorld());
			
			TMap<FString, FString> TestValues;
		
			TestValues.Add("{CircuitID}", "Actually 42");
			TestValues.Add("{TimeEmpty}", "Not Enough Time");
			TestValues.Add("{BattPercent}", "Less than 42");
						
			TestValues.Add("{ItemName}", "Screws");
			TestValues.Add("{ItemNum}", "42");
						
			TestValues.Add("{TrainName}", "Actually 42");
			TestValues.Add("{SelfDriving}", "Who's driving?");
			TestValues.Add("{TrainStation}", "The End (Minecraft)");
						
			TestValues.Add("{Player}", "Robb testing PowerSuit");
			TestValues.Add("{Doggo}", "Nuclear Waste Finder");
						
			TestValues.Add("{Recipes}", "Screws, Screws, and moar Screws");
			TestValues.Add("{Schematic}", "Everyone's favorite Cast Screws");
			
			for (EFlavorType WebhookType : TEnumRange<EFlavorType>())
			{
				Notifications->SendWebhook(TestValues, WebhookType);
			}
			
			ChatReturn.Chat = TEXT("Webhook Tests Completed. Verify results");
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
			
			return ChatReturn;
		}
		
		if (arg1 == "endpoints")
		{
					
			for (FAPIEndpoint endpoint : ModSubsystem->APIEndpoints)
			{
				bool bSuccess = false;
				int32 ErrorCode = 404;
				FRequestData RequestData = FRequestData();
				RequestData.bIsAuthorized = true;
				FString Json;

				const FString sEndpoint = endpoint.APIName;
				
				ModSubsystem->HandleEndpoint(sEndpoint, RequestData, bSuccess, ErrorCode, Json, EInterfaceType::Command);
				
				FString JsonPath = FPaths::ProjectDir() + "Mods/GameFeatures/FicsitRemoteMonitoring/Debug/" + sEndpoint + ".json";

				URemoteMonitoringLibrary::FileSaveString(Json, JsonPath);
				
			}
			
			ChatReturn.Chat = TEXT("Endpoint Tests Completed. Verify results");
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
			
			return ChatReturn;

		}
	}
	
	ChatReturn.Chat = TEXT("Unable to find command " + command + ", please refer to the documentation at docs.ficsit.app.");

	return ChatReturn;
};