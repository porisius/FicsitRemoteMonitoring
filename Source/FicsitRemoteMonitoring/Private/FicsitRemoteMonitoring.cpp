#include "FicsitRemoteMonitoring.h"

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
	//InitSerialDevice();

}

AFicsitRemoteMonitoring::AFicsitRemoteMonitoring() : AModSubsystem()
{

}

AFicsitRemoteMonitoring::~AFicsitRemoteMonitoring()
{

}


void AFicsitRemoteMonitoring::InitHttpService() {

	AFicsitRemoteMonitoring::HttpServer = UHttpServer::CreateHttpServer();

	FString ModPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/";
	FString IconsPath = ModPath + "Icons";
	FString UIPath;

	auto World = GetWorld();
	auto config = FConfig_HTTPStruct::GetActiveConfig(World);

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
			Response.Send();
		})
	);

	HttpServer->Get("/getAssembler", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getAssembler);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBelts", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBelts);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBiomassGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBiomassGenerator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getBlender", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getBlender);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getCoalGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getCoalGenerator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getConstructor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getConstructor);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDoggo", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDoggo);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDrone", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDrone);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDroneStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDroneStation);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getDropPod", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getDropPod);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExplorationSink", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExplorationSink);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExplorer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExplorer);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getExtractor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getExtractor);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFactoryCart", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFactoryCart);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFoundry", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFoundry);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFuelGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFuelGenerator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getGeothermalGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getGeothermalGenerator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getHUBTerminal", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getHUBTerminal);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getManufacturer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getManufacturer);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getModList", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getModList);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getNuclearGenerator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getNuclearGenerator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPackager", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPackager);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getParticle", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getParticle);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPaths", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPaths);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPipes", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPipes);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPlayer", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPlayer);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPower", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPower);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getPowerSlug", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getPowerSlug);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getProdStats", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getProdStats);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRadarTower", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRadarTower);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRecipes", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRecipes);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getRefinery", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getRefinery);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceGeyser", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceGeyser);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceNode", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceNode);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceSink", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceSink);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getResourceWell", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getResourceWell);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSchematics", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSchematics);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSinkList", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSinkList);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSmelter", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSmelter);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSpaceElevator", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSpaceElevator);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getStorageInv", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getStorageInv);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getSwitches", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getSwitches);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTractor", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTractor);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTrains", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTrains);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTrainStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTrainStation);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTruck", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTruck);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getTruckStation", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getTruckStation);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getWorldInv", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getWorldInv);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getAll", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getAll);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getFactory", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getFactory);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getGenerators", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getGenerators);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
		})
	);

	HttpServer->Get("/getVehicles", FHttpServerAPICallback::CreateLambda([World](const FHttpRequest& Request, FHttpResponse& Response) -> void
		{
			TArray<TSharedPtr<FJsonValue>> JSONArray = UAPI_Endpoints::API_Endpoint(World, EAPIEndpoints::getVehicles);
			FString Json = UFRM_Library::APItoJSON(JSONArray, World);
			Response.ReplyJSON(Json, TEXT("application/json"));
			Response.Send();
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

void AFicsitRemoteMonitoring::InitOutageNotification() {

	auto World = GetWorld();

	#if !WITH_EDITOR

	SUBSCRIBE_UOBJECT_METHOD_AFTER(UFGPowerCircuitGroup, OnFuseSet, [World](UFGPowerCircuitGroup* PowerGroup)
		{

			FString DefaultPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json";
			FString JsonPath;
			FString WebhookJson;
			FString Flavor;
						
			auto config = FConfig_DiscITStruct::GetActiveConfig(World);
			AFicsitRemoteMonitoring* FicsitRemoteMonitoring = AFicsitRemoteMonitoring::Get(World);
			TArray<FString> FlavorArray = FicsitRemoteMonitoring->Flavor_Power;

			JsonPath = config.OutageJSON;

			if (config.OutageJSON.IsEmpty()) { 
				JsonPath = DefaultPath; 
			}

			UNotificationLoader::FileLoadString(JsonPath, WebhookJson);

			if (UFGPowerCircuit* PowerCircuit = GetValid(PowerGroup->mCircuits[0])) {
				const int32 CircuitID = PowerCircuit->GetCircuitGroupID();

				int32 len = FlavorArray.Num();
				int32 rng = UKismetMathLibrary::RandomIntegerInRange(0, len);
				Flavor = FlavorArray[rng];

				FString Circuit = FString::FromInt(CircuitID);

				int32 CircuitLog = UKismetStringLibrary::ReplaceInline(WebhookJson, "CircuitID", Circuit, ESearchCase::IgnoreCase);
				int32 FlavorLog = UKismetStringLibrary::ReplaceInline(WebhookJson, "FlavorText", Flavor, ESearchCase::IgnoreCase);



			}


		});

	#endif

}