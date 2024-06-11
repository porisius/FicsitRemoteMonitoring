
#include "Commands/multi.h"
#include <regex>

FChatReturn AFRMCommand::RemoteMonitoringCommand(UObject* WorldContext, class UCommandSender* Sender, const TArray<FString> Arguments) {

	FChatReturn ChatReturn;
	FLinearColor color = FLinearColor::Red;
	FString chat;

	FString command = Arguments[0].ToLower();

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	if (command == "debug") {

		FString OutputType = Arguments[1].ToLower();
		FString sEndpoint = Arguments[2];
		bool bSuccess = false;
		EAPIEndpoints eEndpoint;

		ModSubsystem->TextToAPI(sEndpoint, bSuccess, eEndpoint);

		if (bSuccess) {

			FString Json = AFicsitRemoteMonitoring::API_Endpoint(WorldContext, eEndpoint);

			if (OutputType == "file") {

				FString JsonPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/Debug/" + sEndpoint + ".json";

				UNotificationLoader::FileSaveString(Json, JsonPath);

				ChatReturn.Chat = TEXT("Data saved to Mod location in the Debug folder.");
				ChatReturn.Color = FLinearColor::White;
				ChatReturn.Status = EExecutionStatus::COMPLETED;

				return ChatReturn;

			}
			else if (OutputType == "info") {

				ChatReturn.Chat = Json;
				ChatReturn.Color = FLinearColor::White;
				ChatReturn.Status = EExecutionStatus::COMPLETED;

				return ChatReturn;

			}

		}
		else {

			ChatReturn.Chat = TEXT("Unable to find output type, please refer to the documentation at docs.ficsit.app.");
			ChatReturn.Color = FLinearColor::Red;
			ChatReturn.Status = EExecutionStatus::UNCOMPLETED;

			return ChatReturn;

		}
	}
	else if (command == "http") {

		FString arg1 = Arguments[1].ToLower();

		auto World = GetWorld();
		

		if (arg1 == "start") {

			auto config = FConfig_HTTPStruct::GetActiveConfig(World);
			FString Listen_IP = config.Listen_IP;
			int32 Port = config.HTTP_Port;
			
			//WebSocketServer;

			/*ModSubsystem->HttpServer->Listen(Listen_IP, Port, FHttpServerListenCallback::CreateLambda([&ChatReturn, Listen_IP, Port](const bool Success) -> void
				{
					switch (Success) {
					case true:
						ChatReturn.Chat = FString(TEXT("HTTP Service Initiated"));
						ChatReturn.Color = FLinearColor::Green;
						ChatReturn.Status = EExecutionStatus::COMPLETED;
						UE_LOG(LogHttpServer, Log, TEXT("HTTP Service started. IP: $s - Port: %d"), *FString(Listen_IP), Port);
					case false:
						ChatReturn.Chat = FString(TEXT("HTTP Service failed to start on IP: " + Listen_IP + " - Port: " + FString::FromInt(Port)));
						ChatReturn.Color = FLinearColor::Red;
						ChatReturn.Status = EExecutionStatus::UNCOMPLETED;
						UE_LOG(LogHttpServer, Error, TEXT("HTTP Service failed to start on IP: $s - Port: %d"), *FString(Listen_IP), Port);
					}
				})
			);
			*/

			return ChatReturn;

		}
		else if (arg1 == "stop") {
			//ModSubsystem->HttpServer->Stop();
			UE_LOG(LogHttpServer, Log, TEXT("Stopping HTTP Service."));

			ChatReturn.Chat = TEXT("Stopping HTTP Service.");
			ChatReturn.Color = FLinearColor::White;
			ChatReturn.Status = EExecutionStatus::COMPLETED;

			return ChatReturn;
		}
		else {

			const FRegexPattern ipv4("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
			FString IP = Arguments[1];
			int32 Port = FCString::Atoi(*Arguments[2]);
			
			FRegexMatcher listener(ipv4, IP);

			if (!listener.FindNext()) {
				ChatReturn.Chat = FString(TEXT("HTTP Listener IP Address is invalid"));
				ChatReturn.Color = FLinearColor::Red;
				ChatReturn.Status = EExecutionStatus::UNCOMPLETED;
			}

			if (!UFRM_Library::IsIntInRange(Port, 1, 65535)) {
				ChatReturn.Chat = FString(TEXT("Invalid Port Number"));
				ChatReturn.Color = FLinearColor::Red;
				ChatReturn.Status = EExecutionStatus::UNCOMPLETED;
			}

			/*ModSubsystem->HttpServer->Listen(IP, Port, FHttpServerListenCallback::CreateLambda([&ChatReturn, IP, Port](const bool Success) -> void
				{
					switch (Success) {
					case true:
						ChatReturn.Chat = FString(TEXT("HTTP Service Initiated"));
						ChatReturn.Color = FLinearColor::Green;
						ChatReturn.Status = EExecutionStatus::COMPLETED;
						UE_LOG(LogHttpServer, Log, TEXT("HTTP Service started. IP: $s - Port: %d"), *FString(IP), Port);
					case false:
						ChatReturn.Chat = FString(TEXT("HTTP Service failed to start on IP: " + IP + " - Port: " + FString::FromInt(Port)));
						ChatReturn.Color = FLinearColor::Red;
						ChatReturn.Status = EExecutionStatus::UNCOMPLETED;
						UE_LOG(LogHttpServer, Error, TEXT("HTTP Service failed to start on IP: $s - Port: %d"), *FString(IP), Port);
					}
				})
			);
			*/
			return ChatReturn;

		}

		ChatReturn.Chat = TEXT("Unable to find output type, please refer to the documentation at docs.ficsit.app.");
		ChatReturn.Color = FLinearColor::Red;
		ChatReturn.Status = EExecutionStatus::UNCOMPLETED;

		return ChatReturn;

	}

	ChatReturn.Chat = TEXT("Unable to find command type, please refer to the documentation at docs.ficsit.app.");
	ChatReturn.Color =  FLinearColor::Red;
	ChatReturn.Status = EExecutionStatus::UNCOMPLETED;

	return ChatReturn;
};