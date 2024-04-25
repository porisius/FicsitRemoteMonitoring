#include "FicsitRemoteMonitoring.h"
#include <ArduinoKitBP.h>

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	for (TActorIterator<AFicsitRemoteMonitoring> It(WorldContext, AFicsitRemoteMonitoring::StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
		AFicsitRemoteMonitoring* CurrentActor = *It;
		return CurrentActor;
	};

	return NULL;
}

/*void AFicsitRemoteMonitoring::Tick(DeltaTime) {
	Super::Tick(DeltaTime);
}*/

void AFicsitRemoteMonitoring::BeginPlay()
{
	Super::BeginPlay();

	InitHttpService();
	InitSerialDevice();

}

void AFicsitRemoteMonitoring::InitHttpService() {

	UHttpServer* HttpServer = UHttpServer::CreateHttpServer();

	FString ModPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/";
	FString IconsPath = ModPath + "Icons";
	FString UIPath;

	auto config = FConfig_HTTPStruct::GetActiveConfig(GetWorld());
	
	if (config.Web_Root.IsEmpty()) {
		UIPath = ModPath + "www";
	}
	else
	{
		UIPath = config.Web_Root;
	}	

	HttpServer->Mount(TEXT("/"), UIPath);
	HttpServer->Mount(TEXT("/Icons/"), IconsPath);

	HttpServer->Get(TEXT("/"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			Response.Redirect(TEXT("/html/index.html"));
		})
	);

	HttpServer->Get(TEXT("/getCoffee"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			Response.getCoffee();
		})
	);

	HttpServer->Get(TEXT("/getDrones"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			UWorld* World = GEngine->GameViewport->GetWorld();
			FString Json = UFRM_Library::APItoJSON(UFRM_Drones::getDrone(World), World);
			Response.ReplyJSON(Json, TEXT("application/json"));
		})
	);

	HttpServer->Get(TEXT("/getDroneStation"), FHttpServerAPICallback::CreateLambda([](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			UWorld* World = GEngine->GameViewport->GetWorld();
			FString Json = UFRM_Library::APItoJSON(UFRM_Drones::getDroneStation(World), World);
			Response.ReplyJSON(Json, TEXT("application/json"));
		})
	);

	FString Listen_IP = config.Listen_IP;
	int32 Port = config.HTTP_Port;
	bool AutoStart = config.Web_Autostart;

	if (AutoStart) {
		HttpServer->Listen(Listen_IP, Port, FHttpServerListenCallback::CreateLambda([](const bool Success) -> void
			{
				if (!Success)
				{
					UE_LOG(LogHttpServer, Error, TEXT("HTTP Service failed to start at game load."));
				}
				else
				{
					UE_LOG(LogHttpServer, Log, TEXT("HTTP Service started."));
				};
			})
		);
	};

	
}

void AFicsitRemoteMonitoring::InitSerialDevice() {

	FString SerialMod = "ArduinoKit";

	auto config = FConfig_SerialStruct::GetActiveConfig(GetWorld());

	UModLoadingLibrary* ModLoadingLibrary = NewObject<UModLoadingLibrary>();
	if ((ModLoadingLibrary->IsModLoaded(SerialMod)) && config.COM_Autostart) {
		

		FString Port = config.COM_Port;
		int32 Baud = config.Baud_Rate;
		int32 Size = config.SerialStackSize;

		bool IsOpened = false;

		//this->OpenSerial(Port, Baud, Size, &IsOpened);

		if (!IsOpened)
		{
			UE_LOGFMT(LogArduino, Error, "Unable to open Serial Device on COM Port: {0}, Baud: {1}.", Port, Baud);
		}
		else
		{
			UE_LOGFMT(LogArduino, Log, "Serial Device Initialized on COM Port: {0}, Baud: {1}.", Port, Baud);
		};

	};
}

void AFicsitRemoteMonitoring::SerialListen() {

	/*const bool RS232_Init = UArduinoKitBP::IsOpen();

	if (!RS232_Init) {
		return;
	};*/

	//TArray<uint8> SerialBytes = ReadSerial();

	FString SerialString = "";
	
	this->ReadSerial(SerialString);
	
	/*if (SerialBytes.IsEmpty()) {
		return;
	};*/

	//FString SerialString = BytesToString(SerialBytes);

	UKismetStringLibrary::ReplaceInline(SerialString, "\r\n", "", ESearchCase::IgnoreCase);

	TArray<TSharedPtr<FJsonValue>> JSONArray;

}