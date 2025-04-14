#include "Commands/multi.h"

#include "Config_HTTPStruct.h"
#include "Config_SerialStruct.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "NotificationLoader.h"
#include "StructuredLog.h"
#include "Kismet/KismetSystemLibrary.h"

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

		ModSubsystem->HandleEndpoint(sEndpoint, RequestData, bSuccess, ErrorCode, Json);

		if (!bSuccess) {
			ChatReturn.Chat = TEXT("Unable to find endpoint, please refer to the documentation at docs.ficsit.app or the mod creator.");

			return ChatReturn;
		}

		if (OutputType == "file") {
			FString JsonPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/Debug/" + sEndpoint + ".json";

			UNotificationLoader::FileSaveString(Json, JsonPath);

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
	
	if (command == "http") {
		ChatReturn.Chat = TEXT("Usage: /frm http <start/stop>");

		if (argumentsNum < 2) {
			return ChatReturn;
		}

		FString arg1 = Arguments[1].ToLower();

		if (arg1 == "start") {
			auto config = FConfig_HTTPStruct::GetActiveConfig(WorldContext);
			int32 Port = config.HTTP_Port;
			
			ModSubsystem->StartWebSocketServer();

			ChatReturn.Chat = FString(TEXT("HTTP Service Initiated on Port: " + FString::FromInt(Port)));
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;

			UE_LOG(LogHttpServer, Log, TEXT("HTTP Service started. Port: %d"), Port);
		}
		else if (arg1 == "stop") {
			ModSubsystem->StopWebSocketServer();
			UE_LOG(LogHttpServer, Log, TEXT("Stopping HTTP Service."));

			ChatReturn.Chat = TEXT("Stopping HTTP Service.");
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;
		}

		return ChatReturn;
	}

	if (command == "serial") {
		ChatReturn.Chat = TEXT("Usage: /frm serial <start/stop>");

		if (argumentsNum < 2) {
			return ChatReturn;
		}

		FString arg1 = Arguments[1].ToLower();

		if (arg1 == "start") {
			auto config = FConfig_SerialStruct::GetActiveConfig(WorldContext);
			FString Port = config.COM_Port;

			ModSubsystem->StartWebSocketServer();

			ChatReturn.Chat = FString(TEXT("Serial/RS232 Service Initiated on Port: " + Port));
			ChatReturn.Color = FLinearColor::Green;
			ChatReturn.Status = EExecutionStatus::COMPLETED;

			UE_LOGFMT(LogHttpServer, Log, "Serial/RS232 Service started. Port: {Port}");
		}
		else if (arg1 == "stop") {
			ModSubsystem->StopWebSocketServer();
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

	ChatReturn.Chat = TEXT("Unable to find command " + command + ", please refer to the documentation at docs.ficsit.app.");

	return ChatReturn;
};