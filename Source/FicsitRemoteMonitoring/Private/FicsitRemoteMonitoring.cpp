#include "FicsitRemoteMonitoring.h"
#include "Async/Async.h"

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	for (TActorIterator<AFicsitRemoteMonitoring> It(WorldContext, AFicsitRemoteMonitoring::StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
		AFicsitRemoteMonitoring* CurrentActor = *It;
		return CurrentActor;
	};

	return NULL;
}

AFicsitRemoteMonitoring::AFicsitRemoteMonitoring() : AModSubsystem()
{

}

AFicsitRemoteMonitoring::~AFicsitRemoteMonitoring()
{
	// Destructor ensures server is stopped if the actor is destroyed unexpectedly
	StopWebSocketServer();
}

void AFicsitRemoteMonitoring::BeginPlay()
{
	Super::BeginPlay();
	StartWebSocketServer();
	InitAPIRegistry();

	// Register the callback to ensure WebSocket is stopped on crash/exit
	FCoreDelegates::OnExit.AddUObject(this, &AFicsitRemoteMonitoring::StopWebSocketServer);
}

void AFicsitRemoteMonitoring::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Ensure the server is stopped during normal gameplay exit
	StopWebSocketServer();
	Super::EndPlay(EndPlayReason);
}

void AFicsitRemoteMonitoring::StopWebSocketServer()
{
	// Signal the thread to stop
	bRunning = false;

	if (WebSocketThread.joinable())
	{
		// Timeout to avoid indefinite hang if the thread doesn't stop in time
		using namespace std::chrono_literals;
		auto startTime = std::chrono::steady_clock::now();
		constexpr auto timeoutDuration = 5s;

		// Continuously check until the timeout expires
		while (WebSocketThread.joinable() && 
		       (std::chrono::steady_clock::now() - startTime) < timeoutDuration)
		{
			// Give the WebSocket thread some time to finish
			std::this_thread::sleep_for(100ms);
		}

		// If the thread is still running after the timeout, force terminate
		if (WebSocketThread.joinable())
		{
			UE_LOG(LogTemp, Error, TEXT("WebSocket thread failed to exit in time, forcing shutdown."));
			// Depending on the platform, you may use platform-specific thread termination here.
			// This is a last resort since terminating a thread is not recommended.
		}
		else
		{
			WebSocketThread.join();  // Successfully join the thread if it exited in time
		}
	}
}

void AFicsitRemoteMonitoring::StartWebSocketServer() 
{
	// Join any previous thread before starting a new one
	if (WebSocketThread.joinable())
	{
		WebSocketThread.join();
	}

	// Start the WebSocket server in a separate thread with proper exception handling
	WebSocketThread = std::thread([this]() {
		try
		{
			this->RunWebSocketServer();
		}
		catch (const std::exception& e)
		{
			UE_LOG(LogTemp, Error, TEXT("WebSocketServer crashed: %s"), *FString(e.what()));
			this->StopWebSocketServer();  // Ensure server is stopped on error
		}
	});
}

void AFicsitRemoteMonitoring::RunWebSocketServer()
{
	bRunning = true;

	UE_LOGFMT(LogHttpServer, Warning, "Initializing WebSocket Service");

	try {

		auto app = uWS::App();

		auto World = GetWorld();
		auto config = FConfig_HTTPStruct::GetActiveConfig(World);

		FString ModPath = FPaths::ProjectModsDir() + "FicsitRemoteMonitoring/";
		FString IconsPath = ModPath + "Icons";
		FString UIPath;

		UE_LOG(LogHttpServer, Log, TEXT("Request ModPath: %s"), *ModPath);
		UE_LOG(LogHttpServer, Log, TEXT("Request IconsPath: %s"), *IconsPath);

		if (config.Web_Root.IsEmpty()) {
			UIPath = ModPath + "www";
		}
		else
		{
			UIPath = config.Web_Root;
		};

		std::string const asyncIcon = TCHAR_TO_UTF8(*IconsPath);
		std::string const asyncUI = TCHAR_TO_UTF8(*UIPath);

		int port = config.HTTP_Port;

		// Define WebSocket behavior
		uWS::App::WebSocketBehavior<FWebSocketUserData> wsBehavior;

		wsBehavior.compression = uWS::SHARED_COMPRESSOR;

		// Open handler (for when a client connects)
		wsBehavior.open = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws) {
			OnClientConnected(ws);  // Make sure this function has the correct signature
			};

		// Close handler (for when a client disconnects)
		wsBehavior.close = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
			OnClientDisconnected(ws, code, message);  // Ensure this signature matches
			};

		// Message handler (for when a client sends a message)
		wsBehavior.message = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
			OnMessageReceived(ws, message, opCode);  // Make sure this signature matches
			};


		app.get("/getCoffee", [](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (!res || !req) {
				UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
				return;
			}

			FString noCoffee = TEXT("Error getting coffee, coffee cup, or red solo cup: (418) I'm a teapot."
				"#PraiseAlpaca"
				"#BlameSimon");

			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			res->writeStatus("418 I'm a teapot")->end(TCHAR_TO_UTF8(*noCoffee));

			});

		app.get("/", [](auto* res, auto* req) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			res->writeStatus("301 Moved Permanently")->writeHeader("Location", "/index.html")->end();
			});

		/* This exists incase the root is redirected from default */
		/*app.get("/Icons/", [&asyncUI](auto* res, auto* req) {

			// Attach an onAborted handler to avoid crashes if the request is aborted
			res->onAborted([]() {
				std::cerr << "Request aborted by the client" << std::endl;
				});

			// Get the requested URL (file path)
			std::string url(req->getUrl());
			std::string fullPath = asyncUI + url;

			// Log that we're serving a file
			//UE_LOGFMT(LogHttpServer, Log, "Serving UI file: %s", url.c_str());

			// Serve the requested file
			//serveFile(res, fullPath);

			// Log after serving the file
			//UE_LOGFMT(LogHttpServer, Log, "Done serving file: %s", url.c_str());

		});*/

		app.get("/api/:APIEndpoint", [this, World](auto* res, auto* req) {
			std::string url(req->getParameter("APIEndpoint"));
			FString RequestUrl = FString(url.c_str());

			// Log the request URL
			UE_LOGFMT(LogHttpServer, Log, "Request URL: {0}", RequestUrl);

			bool bSuccess = false;
			FString OutJson = this->HandleEndpoint(World, RequestUrl, bSuccess);

			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			if (!bSuccess) {
				res->writeStatus("404 Not Found")->end("File not found");
			}
			else {
				res->end(TCHAR_TO_UTF8(*OutJson));
			}

		});

		app.get("/get*", [UIPath, this, World](auto* res, auto* req) {

			std::string url(req->getUrl().begin(), req->getUrl().end());
			FString Endpoint = FString(url.c_str()).Mid(1);

			// Log the request URL
			UE_LOG(LogHttpServer, Log, TEXT("Request API: %s"), *Endpoint);

			bool bSuccess = false;
			FString OutJson = this->HandleEndpoint(World, Endpoint, bSuccess);

			// Set CORS headers
			res->writeHeader("Access-Control-Allow-Origin", "*");
			res->writeHeader("Access-Control-Allow-Methods", "GET, POST");
			res->writeHeader("Access-Control-Allow-Headers", "Content-Type");

			if (!bSuccess) {
				res->writeStatus("404 Not Found")->end("File not found");
			}
			else {
				res->end(TCHAR_TO_UTF8(*OutJson));
			}

		});

		app.get("/*", [UIPath, this, World](uWS::HttpResponse<false>* res, auto* req) {
			std::string url(req->getUrl().begin(), req->getUrl().end());

			// Remove initial '/'
			FString RelativePath = FString(url.c_str()).Mid(1);
			FString FilePath = FPaths::Combine(UIPath, RelativePath);
			FString FileContent;
			bool IsBinary = false; // to flag non-text files (e.g., images)

			UE_LOG(LogHttpServer, Log, TEXT("Request RelativePath: %s"), *RelativePath);
			UE_LOG(LogHttpServer, Log, TEXT("Request FilePath: %s"), *FilePath);

			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (!res || !req) {
				UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
				res->writeStatus("500")->end("Internal Server Error");
				return;
			}

			// Determine the MIME type based on file extension
			FString Extension = FPaths::GetExtension(FilePath).ToLower();
			FString ContentType;

			if (Extension == "js") {
				ContentType = "application/javascript";
			} else if (Extension == "css") {
				ContentType = "text/css";
			} else if (Extension == "html" || Extension == "htm") {
				ContentType = "text/html";
			} else if (Extension == "png") {
				ContentType = "image/png";
				IsBinary = true;
			} else if (Extension == "jpg" || Extension == "jpeg") {
				ContentType = "image/jpeg";
				IsBinary = true;
			} else if (Extension == "gif") {
				ContentType = "image/gif";
				IsBinary = true;
			} else {
				ContentType = "text/plain";  // Default to plain text for unknown files
			}

			if (FPaths::FileExists(FilePath)) {
				bool FileLoaded = false;
				if (IsBinary) {
					// For binary files like images, we need to use LoadFileToArray
					TArray<uint8> BinaryContent;
					FileLoaded = FFileHelper::LoadFileToArray(BinaryContent, *FilePath);

					if (FileLoaded) {
						std::string contentLength = std::to_string(BinaryContent.Num());

						UE_LOG(LogHttpServer, Log, TEXT("Binary File Found Returning: %s"), *FilePath);
						res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType));
						res->writeHeader("Access-Control-Allow-Origin", "*");
						res->writeHeader("Content-Length", contentLength.c_str());
						res->write(std::string_view((char*)BinaryContent.GetData(), BinaryContent.Num()));
						res->end();
					}
				} else {
					// Text-based files like HTML, CSS, JS
					FileLoaded = FFileHelper::LoadFileToString(FileContent, *FilePath);
					if (FileLoaded) {
						UE_LOG(LogHttpServer, Log, TEXT("File Found Returning: %s"), *FilePath);
						res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType))
							->writeHeader("Access-Control-Allow-Origin", "*")
							->end(TCHAR_TO_UTF8(*FileContent));
					}
				}

				if (!FileLoaded) {
					UE_LOG(LogHttpServer, Error, TEXT("Failed to load file: %s"), *FilePath);
					res->writeStatus("500")->end("Internal Server Error");
				}
			}

			bool bSuccess = false;
			FString Json = this->HandleEndpoint(World, RelativePath, bSuccess);

			if (bSuccess) {
				UE_LOG(LogHttpServer, Log, TEXT("API Found Returning: %s"), *FilePath);
				res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType))
					->writeHeader("Access-Control-Allow-Origin", "*")
					->end(TCHAR_TO_UTF8(*Json));
			}
			else
			{
				res->writeStatus("404 Not Found")->end("File not found");
			}
		});

		app.listen(port, [](auto* token) {
			FScopeLock ScopeLock(&WebSocketCriticalSection);
			if (token) {
				UE_LOG(LogHttpServer, Warning, TEXT("Listening on port"));
			}
			else {
				UE_LOG(LogHttpServer, Error, TEXT("Failed to listen on port"));
			}
			}).run();

		//std::this_thread::sleep_for(10ms);

	}
	catch (const std::exception& e) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogHttpServer, Error, TEXT("Exception in InitWSService: %s"), *FString(e.what()));
	}
	catch (...) {
		FScopeLock ScopeLock(&WebSocketCriticalSection);
		UE_LOG(LogHttpServer, Error, TEXT("Unknown exception in InitWSService"));
	}

	bRunning = false;
}

void AFicsitRemoteMonitoring::OnClientConnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws) {
    FScopeLock ScopeLock(&WebSocketCriticalSection);
    ConnectedClients.Add({ ws, {} });
    UE_LOG(LogTemp, Warning, TEXT("Client connected. Total clients: %d"), ConnectedClients.Num());
}

void AFicsitRemoteMonitoring::OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
    FScopeLock ScopeLock(&WebSocketCriticalSection);
    ConnectedClients.RemoveAll([ws](const FClientInfo& ClientInfo) { return ClientInfo.Client == ws; });
    UE_LOG(LogTemp, Warning, TEXT("Client disconnected. Total clients: %d"), ConnectedClients.Num());
}

void AFicsitRemoteMonitoring::OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
	FScopeLock ScopeLock(&WebSocketCriticalSection);

	FString MessageContent = FString(message.data());

	// Parse JSON message from the client
	TSharedPtr<FJsonObject> JsonRequest;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageContent);

	if (FJsonSerializer::Deserialize(Reader, JsonRequest) && JsonRequest.IsValid())
	{
		// Find the client and process the request
		for (FClientInfo& ClientInfo : ConnectedClients)
		{
			if (ClientInfo.Client == ws)
			{
				this->ProcessClientRequest(ClientInfo, JsonRequest);
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse client message: %s"), *MessageContent);
	}
}

void AFicsitRemoteMonitoring::ProcessClientRequest(FClientInfo& ClientInfo, const TSharedPtr<FJsonObject>& JsonRequest)
{
    FString Action = JsonRequest->GetStringField("action");
    const TArray<TSharedPtr<FJsonValue>>* EndpointsArray;

    if (JsonRequest->TryGetArrayField("endpoints", EndpointsArray))
    {
        for (const TSharedPtr<FJsonValue>& EndpointValue : *EndpointsArray)
        {
            FString Endpoint = EndpointValue->AsString();

            if (Action == "subscribe")
            {
                ClientInfo.SubscribedEndpoints.Add(Endpoint);
                UE_LOG(LogTemp, Warning, TEXT("Client subscribed to endpoint: %s"), *Endpoint);
            }
            else if (Action == "unsubscribe")
            {
                ClientInfo.SubscribedEndpoints.Remove(Endpoint);
                UE_LOG(LogTemp, Warning, TEXT("Client unsubscribed from endpoint: %s"), *Endpoint);
            }
        }
    }
}

void AFicsitRemoteMonitoring::InitAPIRegistry()
{

	//Registering Endpoints: API Name, bGetAll, bRequireGameThread, UObject, Function Name
	RegisterEndpoint("getAssembler", false, false, this->Get(GetWorld()), FName("getAssembler"));
	RegisterEndpoint("getBelts", true, false, this->Get(GetWorld()), FName("getBelts"));
	RegisterEndpoint("getBiomassGenerator", true, false, this->Get(GetWorld()), FName("getBiomassGenerator"));
	RegisterEndpoint("getBlender", false, false, this->Get(GetWorld()), FName("getBlender"));
	RegisterEndpoint("getCloudInv", true, false, this->Get(GetWorld()), FName("getCloudInv"));
	RegisterEndpoint("getConstructor", true, false, this->Get(GetWorld()), FName("getConstructor"));
	RegisterEndpoint("getConverter", true, false, this->Get(GetWorld()), FName("getConverter"));
	RegisterEndpoint("getDoggo", true, true, this->Get(GetWorld()), FName("getDoggo"));
	RegisterEndpoint("getDrone", true, false, this->Get(GetWorld()), FName("getDrone"));
	RegisterEndpoint("getDroneStation", true, false, this->Get(GetWorld()), FName("getDroneStation"));
	RegisterEndpoint("getDropPod", true, true, this->Get(GetWorld()), FName("getDropPod"));
	RegisterEndpoint("getExplorationSink", true, false, this->Get(GetWorld()), FName("getExplorationSink"));
	RegisterEndpoint("getExplorer", false, true, this->Get(GetWorld()), FName("getExplorer"));
	RegisterEndpoint("getExtractor", true, false, this->Get(GetWorld()), FName("getExtractor"));
	RegisterEndpoint("getFactoryCart", true, false, this->Get(GetWorld()), FName("getFactoryCart"));
	RegisterEndpoint("getFoundry", true, false, this->Get(GetWorld()), FName("getFoundry"));
	RegisterEndpoint("getFuelGenerator", true, false, this->Get(GetWorld()), FName("getFuelGenerator"));
	RegisterEndpoint("getGeothermalGenerator", true, false, this->Get(GetWorld()), FName("getGeothermalGenerator"));
	RegisterEndpoint("getHUBTerminal", true, false, this->Get(GetWorld()), FName("getHUBTerminal"));
	RegisterEndpoint("getManufacturer", true, false, this->Get(GetWorld()), FName("getManufacturer"));
	RegisterEndpoint("getModList", true, false, this->Get(GetWorld()), FName("getModList"));
	RegisterEndpoint("getNuclearGenerator", true, false, this->Get(GetWorld()), FName("getNuclearGenerator"));
	RegisterEndpoint("getParticle", true, false, this->Get(GetWorld()), FName("getParticle"));
	RegisterEndpoint("getPaths", true, false, this->Get(GetWorld()), FName("getPaths"));
	RegisterEndpoint("getPipes", true, false, this->Get(GetWorld()), FName("getPipes"));
	RegisterEndpoint("getPlayer", true, false, this->Get(GetWorld()), FName("getPlayer"));
	RegisterEndpoint("getPower", true, false, this->Get(GetWorld()), FName("getPower"));
	RegisterEndpoint("getPowerSlug", true, false, this->Get(GetWorld()), FName("getPowerSlug"));
	RegisterEndpoint("getProdStats", true, false, this->Get(GetWorld()), FName("getProdStats"));
	RegisterEndpoint("getRadarTower", true, false, this->Get(GetWorld()), FName("getRadarTOwer"));
	RegisterEndpoint("getRecipes", true, false, this->Get(GetWorld()), FName("getRecipes"));
	RegisterEndpoint("getRefinery", true, false, this->Get(GetWorld()), FName("getRefinery"));
	RegisterEndpoint("getResourceGeyser", true, false, this->Get(GetWorld()), FName("getResourceGeyser"));
	RegisterEndpoint("getResourceNode", true, false, this->Get(GetWorld()), FName("getResourceNode"));
	RegisterEndpoint("getResourceSink", true, false, this->Get(GetWorld()), FName("getResourceSink"));
	RegisterEndpoint("getResourceWell", true, false, this->Get(GetWorld()), FName("getResourceWell"));
	RegisterEndpoint("getSchematics", true, false, this->Get(GetWorld()), FName("getSchematics"));
	RegisterEndpoint("getSinkList", true, false, this->Get(GetWorld()), FName("getSinkList"));
	RegisterEndpoint("getSmelter", true, false, this->Get(GetWorld()), FName("getSmelter"));
	RegisterEndpoint("getSpaceElevator", true, false, this->Get(GetWorld()), FName("getSpaceElevator "));
	RegisterEndpoint("getStorageInv", true, false, this->Get(GetWorld()), FName("getStorageInv"));
	RegisterEndpoint("getSwitches", true, false, this->Get(GetWorld()), FName("getSwitches"));
	RegisterEndpoint("getTractor", true, false, this->Get(GetWorld()), FName("getTractor"));
	RegisterEndpoint("getTrains", true, false, this->Get(GetWorld()), FName("getTrains"));
	RegisterEndpoint("getTrainStation", true, false, this->Get(GetWorld()), FName("getTrainStation"));
	RegisterEndpoint("getTruck", false, false, this->Get(GetWorld()), FName("getTruck"));
	RegisterEndpoint("getTruckStation", true, false, this->Get(GetWorld()), FName("getTruckStation"));
	RegisterEndpoint("getWorldInv", true, false, this->Get(GetWorld()), FName("getWorldInv"));

	//FRM API Endpoint Groups
	RegisterEndpoint("getAll", false, false, this->Get(GetWorld()), FName("getAll"));
	RegisterEndpoint("getFactory", true, false, this->Get(GetWorld()), FName("getFactory"));
	RegisterEndpoint("getGenerators", true, false, this->Get(GetWorld()), FName("getGenerators"));
	RegisterEndpoint("getVehicles", true, false, this->Get(GetWorld()), FName("getVehicles"));

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

void AFicsitRemoteMonitoring::BlueprintEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, FAPICallback InCallback)
{
	FAPIEndpoint NewEndpoint;
    NewEndpoint.APIName = APIName;
    NewEndpoint.bGetAll = bGetAll;
    NewEndpoint.bRequireGameThread = bRequireGameThread;
    NewEndpoint.Callback = InCallback;

	APIEndpoints.Add(NewEndpoint);
}

void AFicsitRemoteMonitoring::RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, UObject* TargetObject, FName FunctionName)
{
	FAPIEndpoint NewEndpoint;
	NewEndpoint.APIName = APIName;
	NewEndpoint.bGetAll = bGetAll;
	NewEndpoint.bRequireGameThread = bRequireGameThread;
	NewEndpoint.Callback.BindUFunction(TargetObject, FunctionName);

	APIEndpoints.Add(NewEndpoint);
}

TArray<UBlueprintJsonValue*> AFicsitRemoteMonitoring::CallEndpoint(UObject* WorldContext, FString InEndpoint, bool& bSuccess)
{
	bSuccess = false;

    for (FAPIEndpoint& EndpointInfo : APIEndpoints)
    {
        if (EndpointInfo.APIName == InEndpoint)
        {
			bSuccess = true;
            FAPICallback Callback = EndpointInfo.Callback;

            if (EndpointInfo.bRequireGameThread)
            {
                AsyncTask(ENamedThreads::GameThread, [Callback, WorldContext]() {
                    // Execute callback via GameThread
                    return Callback.Execute(WorldContext);
                });
            }
            else
            {
                // Directly execute the callback
                return Callback.Execute(WorldContext);
            }
        }
    }

    // Return an empty JSON object if no matching endpoint is found
	return TArray<UBlueprintJsonValue*>{};
}

FString AFicsitRemoteMonitoring::HandleEndpoint(UObject* WorldContext, FString InEndpoin, bool& bSuccess)
{
	bSuccess = false;
	TArray<UBlueprintJsonValue*> Json = this->CallEndpoint(WorldContext, InEndpoin, bSuccess);

	if (!bSuccess) {
		return "";
	}

	FConfig_FactoryStruct config = FConfig_FactoryStruct::GetActiveConfig(WorldContext);
	return UBlueprintJsonValue::StringifyArray(Json, config.JSONDebugMode);

}

/*
TArray<TSharedPtr<FJsonValue>> AFicsitRemoteMonitoring::API_Endpoint_Call(UObject* WorldContext, const EAPIEndpoints APICall)
{
	TArray<TSharedPtr<FJsonValue>> Json;

	switch (APICall)
	{
	case EAPIEndpoints::getAssembler: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1_C")));
	case EAPIEndpoints::getBelts: return UFRM_Factory::getBelts(WorldContext);
	case EAPIEndpoints::getBiomassGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass.Build_GeneratorBiomass_C")));
	case EAPIEndpoints::getBlender: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender_C")));
	case EAPIEndpoints::getCoalGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal_C")));
	case EAPIEndpoints::getConstructor: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1_C")));
	case EAPIEndpoints::getDoggo: return UFRM_Player::getDoggo(WorldContext);
	case EAPIEndpoints::getDrone: return UFRM_Drones::getDrone(WorldContext);
	case EAPIEndpoints::getDroneStation: return UFRM_Drones::getDroneStation(WorldContext);
	case EAPIEndpoints::getDropPod: return UFRM_Factory::getDropPod(WorldContext);
	case EAPIEndpoints::getExplorationSink: return UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Exploration);
	case EAPIEndpoints::getExplorer: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer_C")));
	case EAPIEndpoints::getExtractor: return UFRM_Factory::getResourceExtractor(WorldContext);
	case EAPIEndpoints::getFactoryCart: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart_C")));
	case EAPIEndpoints::getFoundry: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1_C")));
	case EAPIEndpoints::getFuelGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorFuel/Build_GeneratorFuel.Build_GeneratorFuel_C")));
	case EAPIEndpoints::getGeothermalGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal_C")));
	case EAPIEndpoints::getHUBTerminal: return UFRM_Factory::getHubTerminal(WorldContext);
	case EAPIEndpoints::getManufacturer: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1_C")));
	case EAPIEndpoints::getModList: return UFRM_Factory::getModList(WorldContext);
	case EAPIEndpoints::getNuclearGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear_C")));
	case EAPIEndpoints::getPackager: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Packager/Build_Packager.Build_Packager_C")));
	case EAPIEndpoints::getParticle: return UFRM_Factory::getFactory(WorldContext, AFGBuildableManufacturerVariablePower::StaticClass());
	case EAPIEndpoints::getPaths: return Json;
	case EAPIEndpoints::getPipes: return UFRM_Factory::getPipes(WorldContext);
	case EAPIEndpoints::getPlayer: return UFRM_Player::getPlayer(WorldContext);
	case EAPIEndpoints::getPower: return UFRM_Power::getCircuit(WorldContext);
	case EAPIEndpoints::getPowerSlug: return UFRM_Factory::getPowerSlug(WorldContext);
	case EAPIEndpoints::getProdStats: return UFRM_Production::getProdStats(WorldContext);
	case EAPIEndpoints::getRadarTower: return UFRM_Factory::getRadarTower(WorldContext);
	case EAPIEndpoints::getRecipes: return UFRM_Production::getRecipes(WorldContext);
	case EAPIEndpoints::getRefinery: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery_C")));
	case EAPIEndpoints::getResourceGeyser: return UFRM_Factory::getResourceNode(WorldContext, AFGResourceNodeFrackingCore::StaticClass());
	case EAPIEndpoints::getResourceNode: return UFRM_Factory::getResourceNode(WorldContext, AFGResourceNodeFrackingSatellite::StaticClass());
	case EAPIEndpoints::getResourceSink: return UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Default);
	case EAPIEndpoints::getResourceWell: return UFRM_Factory::getResourceNode(WorldContext, AFGResourceNode::StaticClass());
	case EAPIEndpoints::getSchematics: return UFRM_Production::getSchematics(WorldContext);
	case EAPIEndpoints::getSinkList: return UFRM_Production::getSinkList(WorldContext);
	case EAPIEndpoints::getSmelter: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1_C")));
	case EAPIEndpoints::getSpaceElevator: return UFRM_Factory::getSpaceElevator(WorldContext);
	case EAPIEndpoints::getStorageInv: return UFRM_Factory::getStorageInv(WorldContext);
	case EAPIEndpoints::getSwitches: return UFRM_Power::getSwitches(WorldContext);
	case EAPIEndpoints::getTractor: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Tractor/BP_Tractor.BP_Tractor_C")));
	case EAPIEndpoints::getTrains: return UFRM_Trains::getTrains(WorldContext);
	case EAPIEndpoints::getTrainStation: return UFRM_Trains::getTrainStation(WorldContext);
	case EAPIEndpoints::getTruck: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck_C")));
	case EAPIEndpoints::getTruckStation: return UFRM_Vehicles::getTruckStation(WorldContext);
	case EAPIEndpoints::getWorldInv: return UFRM_Factory::getWorldInv(WorldContext);

		// Read API Group Endpoints
	case EAPIEndpoints::getAll: return getAll(WorldContext);
	case EAPIEndpoints::getFactory: return UFRM_Factory::getFactory(WorldContext, AFGBuildableManufacturer::StaticClass());
	case EAPIEndpoints::getGenerators: return UFRM_Power::getGenerators(WorldContext, AFGBuildableGenerator::StaticClass());
	case EAPIEndpoints::getVehicles: return UFRM_Vehicles::getVehicles(WorldContext, AFGWheeledVehicle::StaticClass());
	}


	return Json;
}

TArray<TSharedPtr<FJsonValue>> AFicsitRemoteMonitoring::getAll(UObject* WorldContext) {

	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	Json->Values.Add("getBelts", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getBelts)));
	Json->Values.Add("getDrone", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getDrone)));
	Json->Values.Add("getDroneStation", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getDroneStation)));
	Json->Values.Add("getDropPod", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getDropPod)));
	Json->Values.Add("getExplorationSink", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getExplorationSink)));
	Json->Values.Add("getExtractor", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getExtractor)));
	Json->Values.Add("getFactory", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getFactory)));
	Json->Values.Add("getGenerators", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getGenerators)));
	Json->Values.Add("getHUBTerminal", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getHUBTerminal)));
	Json->Values.Add("getModList", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getModList)));
	Json->Values.Add("getPaths", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getPaths)));
	Json->Values.Add("getPipes", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getPipes)));
	Json->Values.Add("getPlayer", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getPlayer)));
	Json->Values.Add("getPower", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getPower)));
	Json->Values.Add("getPowerSlug", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getPowerSlug)));
	Json->Values.Add("getProdStats", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getProdStats)));
	Json->Values.Add("getRadarTower", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getRadarTower)));
	Json->Values.Add("getRecipes", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getRecipes)));
	Json->Values.Add("getResourceGeyser", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getResourceGeyser)));
	Json->Values.Add("getResourceNode", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getResourceNode)));
	Json->Values.Add("getResourceSink", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getResourceSink)));
	Json->Values.Add("getResourceWell", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getResourceWell)));
	Json->Values.Add("getSchematics", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getSchematics)));
	Json->Values.Add("getSinkList", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getSinkList)));
	Json->Values.Add("getSpaceElevator", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getSpaceElevator)));
	Json->Values.Add("getStorageInv", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getStorageInv)));
	Json->Values.Add("getSwitches", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getSwitches)));
	Json->Values.Add("getTrains", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getTrains)));
	Json->Values.Add("getTrainStation", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getTrainStation)));
	Json->Values.Add("getTruckStation", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getTruckStation)));
	Json->Values.Add("getVehicles", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getVehicles)));
	Json->Values.Add("getWorldInv", MakeShared<FJsonValueArray>(API_Endpoint_Call(WorldContext, EAPIEndpoints::getWorldInv)));

	JsonArray.Add(MakeShared<FJsonValueObject>(Json));
	return JsonArray;

}
*/