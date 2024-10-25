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


    //WebSocket Timer
    UWorld* world = GetWorld();
    auto config = FConfig_HTTPStruct::GetActiveConfig(world);

    world->GetTimerManager().SetTimer(
        TimerHandle,  // The timer handle
        this,         // The instance of the class
        &AFicsitRemoteMonitoring::PushUpdatedData,  // Pointer to member function
        config.WebSocketPushCycle,        // Time interval in seconds
        true          // Whether to loop the timer (true = repeating)
    );

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

                app.get("/*", [UIPath, this, World](auto* res, auto* req) {
                    std::string url(req->getUrl().begin(), req->getUrl().end());

                    bool bFileExists = false;
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
                        bFileExists = true;
                    }
                    else if (FPaths::FileExists(FilePath + ".html")) {
                        FilePath = FilePath + ".html";
                        bFileExists = true;
                    }

                    if (bFileExists) {
                        HandleGetRequest(res, req, FilePath);
                    }
                    else {
                        HandleApiRequest(World, res, req, RelativePath);
                    }

                    });

                app.ws<FWebSocketUserData>("/*", std::move(wsBehavior));

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

void AFicsitRemoteMonitoring::OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
    // Remove the client from all endpoint subscriptions
    for (auto& Elem : EndpointSubscribers) {
        Elem.Value.Remove(ws);
    }
}

void AFicsitRemoteMonitoring::OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
	FScopeLock ScopeLock(&WebSocketCriticalSection);

	FString MessageContent = FString(message.data());

	// Parse JSON message from the client
	TSharedPtr<FJsonObject> JsonRequest;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageContent);

	if (FJsonSerializer::Deserialize(Reader, JsonRequest) && JsonRequest.IsValid())
	{
		this->ProcessClientRequest(ws, JsonRequest);
	}
	else
	{
		UE_LOG(LogHttpServer, Error, TEXT("Failed to parse client message: %s"), *MessageContent);
	}
}

void AFicsitRemoteMonitoring::ProcessClientRequest(uWS::WebSocket<false, true, FWebSocketUserData>* ws, const TSharedPtr<FJsonObject>& JsonRequest)
{
    FString Action = JsonRequest->GetStringField("action");
    const TArray<TSharedPtr<FJsonValue>>* EndpointsArray;
    FString Endpoint;

    if (JsonRequest->TryGetArrayField("endpoints", EndpointsArray))
    {
        for (const TSharedPtr<FJsonValue>& EndpointValue : *EndpointsArray)
        {
            Endpoint = EndpointValue->AsString();

            if (Action == "subscribe")
            {
                if (!EndpointSubscribers.Contains(Endpoint)) {
                    EndpointSubscribers.Add(Endpoint, TSet<uWS::WebSocket<false, true, FWebSocketUserData>*>());
                }

                EndpointSubscribers[Endpoint].Add(ws);

                UE_LOG(LogHttpServer, Warning, TEXT("Client subscribed to endpoint: %s"), *Endpoint);
            }
            else if (Action == "unsubscribe")
            {
                EndpointSubscribers[Endpoint].Remove(ws);
                UE_LOG(LogHttpServer, Warning, TEXT("Client unsubscribed from endpoint: %s"), *Endpoint);
            }
        }
    }
    else if (JsonRequest->TryGetStringField("endpoints", Endpoint)) {

        if (Action == "subscribe")
        {
            if (!EndpointSubscribers.Contains(Endpoint)) {
                EndpointSubscribers.Add(Endpoint, TSet<uWS::WebSocket<false, true, FWebSocketUserData>*>());
            }

            EndpointSubscribers[Endpoint].Add(ws);

            UE_LOG(LogHttpServer, Warning, TEXT("Client subscribed to endpoint: %s"), *Endpoint);
        }
        else if (Action == "unsubscribe")
        {
            EndpointSubscribers[Endpoint].Remove(ws);
            UE_LOG(LogHttpServer, Warning, TEXT("Client unsubscribed from endpoint: %s"), *Endpoint);
        }
    }
}

void AFicsitRemoteMonitoring::PushUpdatedData() {

    for (auto& Elem : EndpointSubscribers) {
        FString Endpoint = Elem.Key;
        
        if (Elem.Value.Num() == 0) {
            continue;
        }

        bool bAllocationComplete = false;
        FString Json = HandleEndpoint(this, Endpoint, bAllocationComplete);

        //block while not complete
        while (!bAllocationComplete)
        {
            //100micros sleep, this should be very quick
            FPlatformProcess::Sleep(0.0001f);
        };        

        // Broadcast updated data to all clients subscribed to this endpoint
        for (uWS::WebSocket<false, true, FWebSocketUserData>* Client : Elem.Value) {
            Client->send(TCHAR_TO_UTF8(*Json), uWS::OpCode::TEXT);
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
        UE_LOGFMT(LogHttpServer, Log, "API Not Found: {Endpoint}", Endpoint);
        res->writeStatus("404 Not Found");
        res->writeHeader("Content-Type", "application/json")
            ->writeHeader("Access-Control-Allow-Origin", "*")
            ->end(TCHAR_TO_UTF8(*OutJson));
    }

}


void AFicsitRemoteMonitoring::InitAPIRegistry()
{

	//Registering Endpoints: API Name, bGetAll, bRequireGameThread, UObject, Function Name
	RegisterEndpoint("getAssembler", false, false, this->Get(GetWorld()), FName("getAssembler"));
	RegisterEndpoint("getBelts", true, false, this->Get(GetWorld()), FName("getBelts"));
	RegisterEndpoint("getBiomassGenerator", false, false, this->Get(GetWorld()), FName("getBiomassGenerator"));
	RegisterEndpoint("getBlender", false, false, this->Get(GetWorld()), FName("getBlender"));
	RegisterEndpoint("getCloudInv", true, false, this->Get(GetWorld()), FName("getCloudInv"));
	RegisterEndpoint("getCoalGenerator", false, false, this->Get(GetWorld()), FName("getCoalGenerator"));
    RegisterEndpoint("getConstructor", false, false, this->Get(GetWorld()), FName("getConstructor"));
	RegisterEndpoint("getConverter", false, false, this->Get(GetWorld()), FName("getConverter"));
	RegisterEndpoint("getDoggo", true, true, this->Get(GetWorld()), FName("getDoggo"));
	RegisterEndpoint("getDrone", true, true, this->Get(GetWorld()), FName("getDrone"));
	RegisterEndpoint("getDroneStation", true, false, this->Get(GetWorld()), FName("getDroneStation"));
	RegisterEndpoint("getDropPod", true, true, this->Get(GetWorld()), FName("getDropPod"));
	RegisterEndpoint("getEncoder", true, false, this->Get(GetWorld()), FName("getEncoder"));
	RegisterEndpoint("getExplorationSink", true, false, this->Get(GetWorld()), FName("getExplorationSink"));
	RegisterEndpoint("getExplorer", false, true, this->Get(GetWorld()), FName("getExplorer"));
	RegisterEndpoint("getExtractor", true, true, this->Get(GetWorld()), FName("getExtractor"));
	RegisterEndpoint("getFactoryCart", false, false, this->Get(GetWorld()), FName("getFactoryCart"));
	RegisterEndpoint("getFoundry", false, false, this->Get(GetWorld()), FName("getFoundry"));
	RegisterEndpoint("getFuelGenerator", false, false, this->Get(GetWorld()), FName("getFuelGenerator"));
	RegisterEndpoint("getGeothermalGenerator", false, false, this->Get(GetWorld()), FName("getGeothermalGenerator"));
	RegisterEndpoint("getHUBTerminal", true, true, this->Get(GetWorld()), FName("getHUBTerminal"));
	RegisterEndpoint("getManufacturer", false, false, this->Get(GetWorld()), FName("getManufacturer"));
	RegisterEndpoint("getModList", true, false, this->Get(GetWorld()), FName("getModList"));
	RegisterEndpoint("getNuclearGenerator", false, false, this->Get(GetWorld()), FName("getNuclearGenerator"));
    RegisterEndpoint("getPackager", false, false, this->Get(GetWorld()), FName("getPackager"));
	RegisterEndpoint("getParticle", false, false, this->Get(GetWorld()), FName("getParticle"));
	RegisterEndpoint("getPaths", true, false, this->Get(GetWorld()), FName("getPaths"));
	RegisterEndpoint("getPipes", true, false, this->Get(GetWorld()), FName("getPipes"));
	RegisterEndpoint("getPlayer", true, true, this->Get(GetWorld()), FName("getPlayer"));
	RegisterEndpoint("getPower", true, false, this->Get(GetWorld()), FName("getPower"));
	RegisterEndpoint("getPowerSlug", true, true, this->Get(GetWorld()), FName("getPowerSlug"));
	RegisterEndpoint("getProdStats", true, false, this->Get(GetWorld()), FName("getProdStats"));
	RegisterEndpoint("getRadarTower", true, false, this->Get(GetWorld()), FName("getRadarTower"));
	RegisterEndpoint("getRecipes", true, true, this->Get(GetWorld()), FName("getRecipes"));
	RegisterEndpoint("getRefinery", false, false, this->Get(GetWorld()), FName("getRefinery"));
	RegisterEndpoint("getResourceGeyser", true, true, this->Get(GetWorld()), FName("getResourceGeyser"));
	RegisterEndpoint("getResourceNode", true, true, this->Get(GetWorld()), FName("getResourceNode"));
	RegisterEndpoint("getResourceSink", true, false, this->Get(GetWorld()), FName("getResourceSink"));
    RegisterEndpoint("getResourceSinkBuilding", true, false, this->Get(GetWorld()), FName("getResourceSinkBuilding"));
	RegisterEndpoint("getResourceWell", true, true, this->Get(GetWorld()), FName("getResourceWell"));
	RegisterEndpoint("getSchematics", true, true, this->Get(GetWorld()), FName("getSchematics"));
	RegisterEndpoint("getSinkList", true, true, this->Get(GetWorld()), FName("getSinkList"));
	RegisterEndpoint("getSmelter", false, false, this->Get(GetWorld()), FName("getSmelter"));
	RegisterEndpoint("getSpaceElevator", true, false, this->Get(GetWorld()), FName("getSpaceElevator"));
	RegisterEndpoint("getStorageInv", true, false, this->Get(GetWorld()), FName("getStorageInv"));
	RegisterEndpoint("getSwitches", true, false, this->Get(GetWorld()), FName("getSwitches"));
	RegisterEndpoint("getTractor", false, false, this->Get(GetWorld()), FName("getTractor"));
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

    UE_LOGFMT(LogHttpServer, Log, "Registered API Endpoint: {APIName} - Current number of endpoints registered: {1}", APIName, APIEndpoints.Num());

/*
    // Store the APIName in a member variable for use in HandleCSSEndpoint
    StoredAPIName = APIName;

    UFGServerSubsystem* ServerSubsystem = UFGServerSubsystem::Get(GetWorld());
    if (IsValid(ServerSubsystem)) { return; }

    UFGServerAPIManager* APIManager = ServerSubsystem->GetServerAPIManager();
    if (IsValid(APIManager)) { return; }

    if (!IsRunningDedicatedServer()) {

        // Store the APIName in a member variable for use in HandleCSSEndpoint
        StoredAPIName = APIName;

        UFGServerSubsystem* ServerSubsystem = UFGServerSubsystem::Get(GetWorld());
        if (IsValid(ServerSubsystem)) { return; }

        UFGServerAPIManager* APIManager = ServerSubsystem->GetServerAPIManager();
        if (IsValid(APIManager)) { return; }

        FFGRequestHandlerRegistration HandleRegistration = FFGRequestHandlerRegistration();
        HandleRegistration.HandlerObject = this;
        HandleRegistration.HandlerFunction = this->FindFunction(FName("HandleCSSEndpoint"));
        HandleRegistration.FunctionName = FName(*APIName);
        HandleRegistration.PrivilegeLevel = EPrivilegeLevel::None;
        APIManager->mRegisteredHandlers.Add(FString(APIName), HandleRegistration);

    };
*/
}

TArray<UBlueprintJsonValue*> AFicsitRemoteMonitoring::CallEndpoint(UObject* WorldContext, FString InEndpoint, bool& bSuccess)
{
	bSuccess = false;

    TArray<UBlueprintJsonValue*> JsonArray = TArray<UBlueprintJsonValue*>{};

    for (FAPIEndpoint& EndpointInfo : APIEndpoints)
    {
        if (EndpointInfo.APIName == InEndpoint)
        {
            try {
                bSuccess = true;
                FAPICallback Callback = EndpointInfo.Callback;

                if (EndpointInfo.bRequireGameThread)
                {
                    FThreadSafeBool bAllocationComplete = false;
                    AsyncTask(ENamedThreads::GameThread, [Callback, WorldContext, &JsonArray, &bAllocationComplete]() {
                        // Execute callback via GameThread
                        JsonArray = Callback.Execute(WorldContext);
                        bAllocationComplete = true;
                    });

                    //block while not complete
                    while (!bAllocationComplete)
                    {
                        //100micros sleep, this should be very quick
                        FPlatformProcess::Sleep(0.0001f);
                    };
                }
                else
                {
                    // Directly execute the callback
                    JsonArray = Callback.Execute(WorldContext);
                }
            } catch (const std::exception& e) 
            {

                FString err = *FString(e.what());

                UBlueprintJsonObject* JsonObject = UBlueprintJsonObject::Create();

                JsonObject->SetString("error", "CallEndpoint Exception: " + err);
                JsonArray.Add(UBlueprintJsonValue::FromObject(JsonObject));

                JsonObject->SetString("recommendation", "Please relay this error, and logs the the Sysadmin Modding Discord for anaylsis.");
                JsonArray.Add(UBlueprintJsonValue::FromObject(JsonObject));

                UE_LOGFMT(LogHttpServer, Error, "CallEndpoint Exception: %s", err);

            } catch (...) {

                UE_LOG(LogHttpServer, Error, TEXT("Unknown Exception in CallEndpoint"));
                
            }

        }
    }

    // Return an empty JSON object if no matching endpoint is found
	return JsonArray;
}

FString AFicsitRemoteMonitoring::HandleEndpoint(UObject* WorldContext, FString InEndpoin, bool& bSuccess)
{
	bSuccess = false;

	TArray<UBlueprintJsonValue*> Json = this->CallEndpoint(WorldContext, InEndpoin, bSuccess);

	if (!bSuccess) {
		return "{\"error\": \"Endpoint not found. Please consult Endpoint's documentation for more information.\"}";
	}

	FConfig_FactoryStruct config = FConfig_FactoryStruct::GetActiveConfig(WorldContext);
	return UBlueprintJsonValue::StringifyArray(Json, config.JSONDebugMode);

}

/*FFGServerErrorResponse AFicsitRemoteMonitoring::HandleCSSEndpoint(FString& out_json, FString InEndpoin)
{
    bool bSuccess = false;
    auto World = GetWorld();
    TArray<UBlueprintJsonValue*> Json = this->CallEndpoint(World, InEndpoin, bSuccess);

    if (!bSuccess) {
        out_json = "{'error': 'Endpoint not found. Please consult Endpoint's documentation for more information.'}";
        return FFGServerErrorResponse::Ok();
    }

    FConfig_FactoryStruct config = FConfig_FactoryStruct::GetActiveConfig(World);
    out_json = UBlueprintJsonValue::StringifyArray(Json, config.JSONDebugMode);
    return FFGServerErrorResponse::Ok();

}
*/
TArray<UBlueprintJsonValue*> AFicsitRemoteMonitoring::getAll(UObject* WorldContext) {

	TArray<UBlueprintJsonValue*> JsonArray;

    for ( FAPIEndpoint APIEndpoint : APIEndpoints) {
        
        bool bSuccess = false;

        if (APIEndpoint.bGetAll) {
            UBlueprintJsonObject* Json = UBlueprintJsonObject::Create();
            Json->SetArray(APIEndpoint.APIName, CallEndpoint(WorldContext, APIEndpoint.APIName, bSuccess));

            //block while not complete
            while (!bSuccess)
            {
                //100micros sleep, this should be very quick
                FPlatformProcess::Sleep(0.0001f);
            };

            JsonArray.Add(UBlueprintJsonValue::FromObject(Json));
        }
    }

	return JsonArray;

}