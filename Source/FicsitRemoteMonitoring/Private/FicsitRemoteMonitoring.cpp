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

    // Load FRM's API Endpoints
    InitAPIRegistry();

    // If true, autostart web server/socket
    auto WSconfig = FConfig_HTTPStruct::GetActiveConfig(GetWorld());
    bool WSStart = WSconfig.Web_Autostart;

    // If true, autostart serial device
    auto RSconfig = FConfig_SerialStruct::GetActiveConfig(GetWorld());
    bool RSStart = RSconfig.COM_Autostart;

    if (WSStart) { StartWebSocketServer(); }
    if (RSStart) { InitSerialDevice(); }	

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
    // Signal the WebSocket server to stop
    if (WebServer.IsValid())
    {
        WebServer.Reset();
    }
}

void AFicsitRemoteMonitoring::StartWebSocketServer() 
{
    UE_LOGFMT(LogHttpServer, Warning, "Initializing WebSocket Service");

        // WebSocket server logic runs in a separate thread
        WebServer = Async(EAsyncExecution::Thread, [this]() {
            try {
                auto app = uWS::App();
                auto World = GetWorld();
                auto config = FConfig_HTTPStruct::GetActiveConfig(World);

                FString ModPath = FPaths::ProjectModsDir() + "FicsitRemoteMonitoring/";
                FString IconsPath = ModPath + "Icons";
                FString UIPath;

                if (config.Web_Root.IsEmpty()) {
                    UIPath = ModPath + "www";
                }
                else
                {
                    UIPath = config.Web_Root;
                };

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
                app.get("/Icons/*", [this, IconsPath](auto* res, auto* req) {

                    std::string url(req->getUrl().begin(), req->getUrl().end());

                    // Remove initial '/Icons/'
                    FString RelativePath = FString(url.c_str()).Mid(7);
                    FString FilePath = FPaths::Combine(IconsPath, RelativePath);

                    UE_LOG(LogHttpServer, Log, TEXT("Request RelativePath: %s"), *RelativePath);
                    UE_LOG(LogHttpServer, Log, TEXT("Request FilePath: %s"), *FilePath);

                    FScopeLock ScopeLock(&WebSocketCriticalSection);
                    if (!res || !req) {
                        UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
                        res->writeStatus("500")->end("Internal Server Error");
                        return;
                    }

                    if (FPaths::FileExists(FilePath)) {
                        HandleGetRequest(res, req, FilePath);
                    }

                    });

                app.get("/api/:APIEndpoint", [this, World](auto* res, auto* req) {
                    std::string url(req->getParameter("APIEndpoint"));
                    FString Endpoint = FString(url.c_str());

                    // Log the request URL
                    UE_LOGFMT(LogHttpServer, Log, "Request URL: {0}", Endpoint);

                    HandleApiRequest(World, res, req, Endpoint);

                    });

                app.get("/get*", [UIPath, this, World](auto* res, auto* req) {

                    std::string url(req->getUrl().begin(), req->getUrl().end());
                    FString Endpoint = FString(url.c_str()).Mid(1);

                    // Log the request URL
                    UE_LOG(LogHttpServer, Log, TEXT("Request API: %s"), *Endpoint);

                    HandleApiRequest(World, res, req, Endpoint);

                    });

                app.get("/*", [UIPath, this, World](auto* res, auto* req) {
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

                    if (FPaths::FileExists(FilePath)) {
                        HandleGetRequest(res, req, FilePath);
                    }
                    else {
                        HandleApiRequest(World, res, req, RelativePath);
                    }

                    });

                app.listen(port, [port](auto* token) {

                    UE_LOG(LogHttpServer, Warning, TEXT("Attempting to listen on port %d"), port);

                    FScopeLock ScopeLock(&WebSocketCriticalSection);
                    if (token) {
                        UE_LOGFMT(LogHttpServer, Warning, "Listening on port {port}", port);
                    }
                    else {
                        UE_LOGFMT(LogHttpServer, Error, "Failed to listen on port {port}", port);
                    }
                    });

                app.run();

            } catch (const std::exception& e) {
                UE_LOG(LogHttpServer, Error, TEXT("WebSocket Server Exception: %s"), *FString(e.what()));
            } catch (...) {
                UE_LOG(LogHttpServer, Error, TEXT("Unknown Exception in WebSocket Server"));
            }
        });

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

void AFicsitRemoteMonitoring::HandleGetRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString FilePath)
{

    FString FileContent;
    bool IsBinary = false; // to flag non-text files (e.g., images)

    // Determine the MIME type based on file extension
    FString Extension = FPaths::GetExtension(FilePath).ToLower();
    FString ContentType;

    if (Extension == "js") {
        ContentType = "application/javascript";
    }
    else if (Extension == "css") {
        ContentType = "text/css";
    }
    else if (Extension == "html" || Extension == "htm") {
        ContentType = "text/html";
    }
    else if (Extension == "png") {
        ContentType = "image/png";
        IsBinary = true;
    }
    else if (Extension == "jpg" || Extension == "jpeg") {
        ContentType = "image/jpeg";
        IsBinary = true;
    }
    else if (Extension == "gif") {
        ContentType = "image/gif";
        IsBinary = true;
    }
    else {
        ContentType = "text/plain";  // Default to plain text for unknown files
    }

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
    }
    else {
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

void AFicsitRemoteMonitoring::HandleApiRequest(UObject* World, uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString Endpoint)
{

    bool bSuccess = false;
    FString OutJson = this->HandleEndpoint(World, Endpoint, bSuccess);

    if (bSuccess) {
        UE_LOGFMT(LogHttpServer, Log, "API Found Returning: {Endpoint}", Endpoint);
        res->writeHeader("Content-Type", "application/json")
            ->writeHeader("Access-Control-Allow-Origin", "*")
            ->end(TCHAR_TO_UTF8(*OutJson));
    }
    else
    {
        res->writeStatus("404 Not Found")->end("File not found");
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
	RegisterEndpoint("getDrone", true, true, this->Get(GetWorld()), FName("getDrone"));
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
	RegisterEndpoint("getPlayer", true, true, this->Get(GetWorld()), FName("getPlayer"));
	RegisterEndpoint("getPower", true, false, this->Get(GetWorld()), FName("getPower"));
	RegisterEndpoint("getPowerSlug", true, true, this->Get(GetWorld()), FName("getPowerSlug"));
	RegisterEndpoint("getProdStats", true, false, this->Get(GetWorld()), FName("getProdStats"));
	RegisterEndpoint("getRadarTower", true, false, this->Get(GetWorld()), FName("getRadarTOwer"));
	RegisterEndpoint("getRecipes", true, false, this->Get(GetWorld()), FName("getRecipes"));
	RegisterEndpoint("getRefinery", true, false, this->Get(GetWorld()), FName("getRefinery"));
	RegisterEndpoint("getResourceGeyser", true, false, this->Get(GetWorld()), FName("getResourceGeyser"));
	RegisterEndpoint("getResourceNode", true, true, this->Get(GetWorld()), FName("getResourceNode"));
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
		return "{'error': 'Endpoint not found. Please consult Endpoint's documentation for more information.'}";
	}

	FConfig_FactoryStruct config = FConfig_FactoryStruct::GetActiveConfig(WorldContext);
	return UBlueprintJsonValue::StringifyArray(Json, config.JSONDebugMode);

}

TArray<UBlueprintJsonValue*> AFicsitRemoteMonitoring::getAll(UObject* WorldContext) {

	TArray<UBlueprintJsonValue*> JsonArray;

    UBlueprintJsonObject* Json = UBlueprintJsonObject::Create();

    for ( FAPIEndpoint APIEndpoint : APIEndpoints) {
        
        bool bSuccess = false;

        if (APIEndpoint.bGetAll) {
            Json->SetArray(APIEndpoint.APIName, CallEndpoint(WorldContext, APIEndpoint.APIName, bSuccess));
        }

        JsonArray.Add(UBlueprintJsonValue::FromObject(Json));

    }

	return JsonArray;

}