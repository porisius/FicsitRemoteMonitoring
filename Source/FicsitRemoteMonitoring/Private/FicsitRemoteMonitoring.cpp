#include "FicsitRemoteMonitoring.h"
#include "Async/Async.h"
#include "FRM_Request.h"

us_listen_socket_t* SocketListener;
bool SocketRunning = false;

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

    // store JSONDebugMode into a local property to prevent crash while access to GetActiveConfig while the EndPlay process
    const auto FactoryConfig = FConfig_FactoryStruct::GetActiveConfig(world);
    JSONDebugMode = FactoryConfig.JSONDebugMode;

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
    // clear the timer
    UWorld* world = GetWorld();
    world->GetTimerManager().ClearTimer(TimerHandle);

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

    // Close WebSocket listener
    if (SocketListener)
    {
        UE_LOGFMT(LogHttpServer, Log, "Stopping uWS listener");
        us_listen_socket_close(0, SocketListener);
        SocketListener = nullptr;

        UE_LOG(LogHttpServer, Log, TEXT("Closing all %d connections"), ConnectedClients.Num());
        for (const auto ConnectedClient : ConnectedClients)
        {
            ConnectedClient->close();
        }
        ConnectedClients.Empty();
    }

    // clear endpoint subscribers
    EndpointSubscribers.Empty();
}

void AFicsitRemoteMonitoring::StartWebSocketServer() 
{
    UE_LOGFMT(LogHttpServer, Warning, "Initializing WebSocket Service");

    if (SocketRunning)
    {
        UE_LOG(LogHttpServer, Log, TEXT("Old Websocket Thread is still running, try again in 3 seconds..."));

        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
        {
            // Sleep for the specified delay
            FPlatformProcess::Sleep(3.0f);
        
            // Then switch back to the game thread to run the task
            AsyncTask(ENamedThreads::GameThread, [this]()
            {
                StartWebSocketServer();
            });
        });
        return;
    }

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
                    ConnectedClients.Remove(ws);
                    UE_LOG(LogHttpServer, Log, TEXT("Client Disconnected. Remaining connections: %d"), ConnectedClients.Num());
                    OnClientDisconnected(ws, code, message);  // Ensure this signature matches
                };

                // Message handler (for when a client sends a message)
                wsBehavior.message = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
                    OnMessageReceived(ws, message, opCode);  // Make sure this signature matches
                };

                wsBehavior.open = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws)
                {
                    ConnectedClients.Add(ws);
                    UE_LOG(LogHttpServer, Log, TEXT("Client Disconnected. Connections: %d"), ConnectedClients.Num());
                };

                app.get("/getCoffee", [this](auto* res, auto* req) {
                    if (!res || !req) {
                        UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
                        return;
                    }

                    FString noCoffee = TEXT("Error getting coffee, coffee cup, or red solo cup: (418) I'm a teapot."
                        "#PraiseAlpaca"
                        "#BlameSimon");

                    // Set CORS headers
                    res->writeStatus("418 I'm a teapot");
                    res->writeHeader("Access-Control-Allow-Methods", "GET, POST");
                    res->writeHeader("Access-Control-Allow-Headers", "Content-Type");
                    UFRM_RequestLibrary::AddResponseHeaders(res, false);

                    res->end(TCHAR_TO_UTF8(*noCoffee));
                });

                app.get("/", [](auto* res, auto* req) {
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

                    if (!res || !req) {
                        UE_LOG(LogHttpServer, Error, TEXT("Invalid request or response pointer!"));
                        return;
                    }

                    if (FPaths::FileExists(FilePath)) {
                        HandleGetRequest(res, req, FilePath);
                    	return;
                    }

                	UFRM_RequestLibrary::SendErrorJson(res, "404 Not Found", "");
                });

                app.get("/api/:APIEndpoint", [this, World](auto* res, auto* req) {
                    std::string url(req->getParameter("APIEndpoint"));
                    FString Endpoint = FString(url.c_str());

                    // Log the request URL
                    UE_LOGFMT(LogHttpServer, Log, "Request URL: {0}", Endpoint);

                	FRequestData RequestData;
                    HandleApiRequest(World, res, req, Endpoint, RequestData);
                });

            	app.post("/*", [this, World](auto* res, uWS::HttpRequest* req)
            	{
		            const std::string URL(req->getUrl().begin(), req->getUrl().end());
					FString RelativePath = FString(URL.c_str()).Mid(1);

            		res->onData([this, res, req, World, RelativePath](const std::string_view data, bool)
            		{
			            try
			            {
				            const std::string PostData(data);
				            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FString(PostData.c_str()));
			            	TSharedPtr<FJsonValue> JsonValue;

			            	if (!FJsonSerializer::Deserialize(Reader, JsonValue) || !JsonValue.IsValid())
			            	{
			            		return UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            	}

			            	FRequestData RequestData;
			            	RequestData.Method = "POST";
			            	
			            	if (JsonValue->Type == EJson::Array)
			            	{
			            		RequestData.Body = JsonValue->AsArray();
							}
			            	else if (JsonValue->Type == EJson::Object)
			            	{
			            		RequestData.Body.Add(JsonValue);
							}
			            	else
			            	{
			            		return UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            	}

			            	HandleApiRequest(World, res, req, RelativePath, RequestData);
			            }
			            catch (const std::exception &e)
			            {
			            	UE_LOG(LogHttpServer, Error, TEXT("Request Exception: %s"), *e.what());
			            	UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            }
            		});

            		res->onAborted([]() {});
            	});

                app.get("/*", [UIPath, this, World](auto* res, uWS::HttpRequest* req) {
                    if (!res) return;

                    std::string url(req->getUrl().begin(), req->getUrl().end());
                    
                    bool bFileExists = false;
                    // Remove initial '/'
                    FString RelativePath = FString(url.c_str()).Mid(1);
                    FString FilePath = FPaths::Combine(UIPath, RelativePath);
                    FString FileContent;
                    bool IsBinary = false; // to flag non-text files (e.g., images)

                    UE_LOG(LogHttpServer, Log, TEXT("Request RelativePath/FilePath: %s %s"), *RelativePath, *FilePath);

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
                    	FRequestData RequestData;
                        HandleApiRequest(World, res, req, RelativePath, RequestData);
                    }
                });

                app.ws<FWebSocketUserData>("/*", std::move(wsBehavior));

                app.listen(port, [port](us_listen_socket_t* token) {

                    UE_LOG(LogHttpServer, Warning, TEXT("Attempting to listen on port %d"), port);

                    if (token) {
                        SocketListener = token;
                        UE_LOGFMT(LogHttpServer, Warning, "Listening on port {port}", port);
                    }
                    else {
                        UE_LOGFMT(LogHttpServer, Error, "Failed to listen on port {port}", port);
                    }
                });

                SocketRunning = true;
                
                app.run();

                SocketRunning = false;

                UE_LOG(LogHttpServer, Log, TEXT("WebSocket Server Thread finished."));
            } catch (const std::exception& e) {
                UE_LOG(LogHttpServer, Error, TEXT("WebSocket Server Exception: %s"), *FString(e.what()));
            } catch (...) {
                UE_LOG(LogHttpServer, Error, TEXT("Unknown Exception in WebSocket Server"));
            }
        });

}

std::string UrlDecode(const std::string &Value) {
	std::ostringstream Decoded;
	for (size_t i = 0; i < Value.length(); ++i) {
		if (Value[i] == '%') {
			std::istringstream HexStream(Value.substr(i + 1, 2));
			if (int HexValue; HexStream >> std::hex >> HexValue) {
				Decoded << static_cast<char>(HexValue);
				i += 2;
			} else {
				Decoded << '%'; // Invalid hex sequence
			}
		} else if (Value[i] == '+') {
			Decoded << ' ';
		} else {
			Decoded << Value[i];
		}
	}
	return Decoded.str();
}

std::unordered_map<std::string, std::string> ParseQueryString(const std::string& Query) {
	std::unordered_map<std::string, std::string> QueryPairs;
	std::istringstream QueryStream(Query);
	std::string Pair;
    
	while (std::getline(QueryStream, Pair, '&')) {
		const auto DelimiterPos = Pair.find('=');
		if (DelimiterPos == std::string::npos) continue; // Skip if there's no '=' character

		std::string Key = Pair.substr(0, DelimiterPos);
		std::string Value = Pair.substr(DelimiterPos + 1);
		QueryPairs[UrlDecode(Key)] = UrlDecode(Value);
	}
    
	return QueryPairs;
}

void AFicsitRemoteMonitoring::OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
    // Remove the client from all endpoint subscriptions
    for (auto& Elem : EndpointSubscribers) {
        Elem.Value.Remove(ws);
    }
}

void AFicsitRemoteMonitoring::OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {

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
        FString Json = HandleEndpoint(this, Endpoint, FRequestData(), bAllocationComplete);

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

    bool FileLoaded;
    if (IsBinary) {
        // For binary files like images, we need to use LoadFileToArray
        TArray<uint8> BinaryContent;
        FileLoaded = FFileHelper::LoadFileToArray(BinaryContent, *FilePath);

        if (FileLoaded) {
            std::string contentLength = std::to_string(BinaryContent.Num());

            UE_LOG(LogHttpServer, Log, TEXT("Binary File Found Returning: %s"), *FilePath);

            res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType));
            res->writeHeader("Content-Length", contentLength.c_str());
            UFRM_RequestLibrary::AddResponseHeaders(res, false);
            res->write(std::string_view((char*)BinaryContent.GetData(), BinaryContent.Num()));
            res->end();
        }
    }
    else {
        FString FileContent;
        // Text-based files like HTML, CSS, JS
        FileLoaded = FFileHelper::LoadFileToString(FileContent, *FilePath);
        if (FileLoaded) {
            UE_LOG(LogHttpServer, Log, TEXT("File Found Returning: %s"), *FilePath);

            res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType));
            UFRM_RequestLibrary::AddResponseHeaders(res, false);
            res->end(TCHAR_TO_UTF8(*FileContent));
        }
    }

    if (!FileLoaded) {
        UE_LOG(LogHttpServer, Error, TEXT("Failed to load file: %s"), *FilePath);
    	UFRM_RequestLibrary::SendErrorMessage(res, "500 Internal Server Error", "Failed to load file.");
    }
}

void AFicsitRemoteMonitoring::HandleApiRequest(UObject* World, uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString Endpoint, FRequestData RequestData)
{
	// Parse all query parameters
	const std::string QueryString(req->getQuery().begin(), req->getQuery().end());
	const auto QueryParams = ParseQueryString(QueryString);

	TMap<FString, FString> RequestQueryParams = TMap<FString, FString>();
	// Iterate through all query parameters and log them
	for (const auto& Param : QueryParams) {
		FString Key(Param.first.c_str());
		FString Value(Param.second.c_str());

		RequestQueryParams.Add(*Key, *Value);
	}

	RequestData.QueryParams = RequestQueryParams;
	
    bool bSuccess = false;
    FString OutJson = this->HandleEndpoint(World, Endpoint, RequestData, bSuccess);

    if (bSuccess) {
        UE_LOGFMT(LogHttpServer, Log, "API Found Returning: {Endpoint}", Endpoint);
        UFRM_RequestLibrary::AddResponseHeaders(res, true);
        res->end(TCHAR_TO_UTF8(*OutJson));
    }
    else
    {
        UE_LOGFMT(LogHttpServer, Log, "API Not Found: {Endpoint}", Endpoint);
    	UFRM_RequestLibrary::SendErrorJson(res, "404 Not Found", OutJson);
    }

}

void AFicsitRemoteMonitoring::InitAPIRegistry()
{

	//Registering Endpoints: API Name, bGetAll, bRequireGameThread, FunctionPtr
	RegisterEndpoint("getAssembler", false, false, &AFicsitRemoteMonitoring::getAssembler);
	RegisterEndpoint("getBelts", true, false, &AFicsitRemoteMonitoring::getBelts);
	RegisterEndpoint("getBiomassGenerator", false, false, &AFicsitRemoteMonitoring::getBiomassGenerator);
	RegisterEndpoint("getBlender", false, false, &AFicsitRemoteMonitoring::getBlender);
	RegisterEndpoint("getCloudInv", true, false, &AFicsitRemoteMonitoring::getCloudInv);
	RegisterEndpoint("getCoalGenerator", false, false, &AFicsitRemoteMonitoring::getCoalGenerator);
    RegisterEndpoint("getConstructor", false, false, &AFicsitRemoteMonitoring::getConstructor);
	RegisterEndpoint("getConverter", false, false, &AFicsitRemoteMonitoring::getConverter);
	RegisterEndpoint("getDoggo", true, true, &AFicsitRemoteMonitoring::getDoggo);
	RegisterEndpoint("getDrone", true, true, &AFicsitRemoteMonitoring::getDrone);
	RegisterEndpoint("getDroneStation", true, false, &AFicsitRemoteMonitoring::getDroneStation);
	RegisterEndpoint("getDropPod", true, true, &AFicsitRemoteMonitoring::getDropPod);
	RegisterEndpoint("getEncoder", true, false, &AFicsitRemoteMonitoring::getEncoder);
	RegisterEndpoint("getExplorationSink", true, false, &AFicsitRemoteMonitoring::getExplorationSink);
	RegisterEndpoint("getExplorer", false, true, &AFicsitRemoteMonitoring::getExplorer);
	RegisterEndpoint("getExtractor", true, true, &AFicsitRemoteMonitoring::getExtractor);
	RegisterEndpoint("getFactoryCart", false, false, &AFicsitRemoteMonitoring::getFactoryCart);
	RegisterEndpoint("getFoundry", false, false, &AFicsitRemoteMonitoring::getFoundry);
    RegisterEndpoint("getFrackingActivator", false, false, &AFicsitRemoteMonitoring::getFrackingActivator);
	RegisterEndpoint("getFuelGenerator", false, false, &AFicsitRemoteMonitoring::getFuelGenerator);
	RegisterEndpoint("getGeothermalGenerator", false, false, &AFicsitRemoteMonitoring::getGeothermalGenerator);
	RegisterEndpoint("getHUBTerminal", true, true, &AFicsitRemoteMonitoring::getHUBTerminal);
  RegisterEndpoint("getHypertube", true, false, &AFicsitRemoteMonitoring::getHypertube);
	RegisterEndpoint("getManufacturer", false, false, &AFicsitRemoteMonitoring::getManufacturer);
	RegisterEndpoint("getModList", true, false, &AFicsitRemoteMonitoring::getModList);
	RegisterEndpoint("getNuclearGenerator", false, false, &AFicsitRemoteMonitoring::getNuclearGenerator);
    RegisterEndpoint("getPackager", false, false, &AFicsitRemoteMonitoring::getPackager);
	RegisterEndpoint("getParticle", false, false, &AFicsitRemoteMonitoring::getParticle);
	RegisterEndpoint("getPaths", true, false, &AFicsitRemoteMonitoring::getPaths);
	RegisterEndpoint("getPipes", true, false, &AFicsitRemoteMonitoring::getPipes);
	RegisterEndpoint("getPlayer", true, true, &AFicsitRemoteMonitoring::getPlayer);
  RegisterEndpoint("getPortal", true, false, &AFicsitRemoteMonitoring::getPortal);
	RegisterEndpoint("getPower", true, false, &AFicsitRemoteMonitoring::getPower);
	RegisterEndpoint("getPowerSlug", true, true, &AFicsitRemoteMonitoring::getPowerSlug);
	RegisterEndpoint("getPowerUsage", true, false, &AFicsitRemoteMonitoring::getPowerUsage);
	RegisterEndpoint("getProdStats", true, false, &AFicsitRemoteMonitoring::getProdStats);
  RegisterEndpoint("getPump", true, false, &AFicsitRemoteMonitoring::getPump);
	RegisterEndpoint("getRadarTower", true, false, &AFicsitRemoteMonitoring::getRadarTower);
	RegisterEndpoint("getRecipes", true, true, &AFicsitRemoteMonitoring::getRecipes);
	RegisterEndpoint("getRefinery", false, false, &AFicsitRemoteMonitoring::getRefinery);
	RegisterEndpoint("getResourceGeyser", true, true, &AFicsitRemoteMonitoring::getResourceGeyser);
	RegisterEndpoint("getResourceNode", true, true, &AFicsitRemoteMonitoring::getResourceNode);
	RegisterEndpoint("getResourceSink", true, false, &AFicsitRemoteMonitoring::getResourceSink);
    RegisterEndpoint("getResourceSinkBuilding", true, false, &AFicsitRemoteMonitoring::getResourceSinkBuilding);
	RegisterEndpoint("getResourceWell", true, true, &AFicsitRemoteMonitoring::getResourceWell);
    RegisterEndpoint("getSessionInfo", true, true, true, &AFicsitRemoteMonitoring::getSessionInfo);
	RegisterEndpoint("getSchematics", true, true, &AFicsitRemoteMonitoring::getSchematics);
	RegisterEndpoint("getSinkList", true, true, &AFicsitRemoteMonitoring::getSinkList);
	RegisterEndpoint("getSmelter", false, false, &AFicsitRemoteMonitoring::getSmelter);
	RegisterEndpoint("getSpaceElevator", true, false, &AFicsitRemoteMonitoring::getSpaceElevator);
	RegisterEndpoint("getStorageInv", true, false, &AFicsitRemoteMonitoring::getStorageInv);
	RegisterEndpoint("getSwitches", true, false, &AFicsitRemoteMonitoring::getSwitches);
	RegisterEndpoint("getTractor", false, false, &AFicsitRemoteMonitoring::getTractor);
	RegisterEndpoint("getTrains", true, false, &AFicsitRemoteMonitoring::getTrains);
	RegisterEndpoint("getTrainStation", true, false, &AFicsitRemoteMonitoring::getTrainStation);
	RegisterEndpoint("getTruck", false, false, &AFicsitRemoteMonitoring::getTruck);
	RegisterEndpoint("getTruckStation", true, false, &AFicsitRemoteMonitoring::getTruckStation);
	RegisterEndpoint("getWorldInv", true, false, &AFicsitRemoteMonitoring::getWorldInv);
	RegisterEndpoint("getResearchTrees", true, true, &AFicsitRemoteMonitoring::getResearchTrees);

	//FRM API Endpoint Groups
	RegisterEndpoint("getAll", false, false, &AFicsitRemoteMonitoring::getAll);
	RegisterEndpoint("getFactory", true, false, &AFicsitRemoteMonitoring::getFactory);
	RegisterEndpoint("getGenerators", true, false, &AFicsitRemoteMonitoring::getGenerators);
	RegisterEndpoint("getVehicles", true, false, &AFicsitRemoteMonitoring::getVehicles);

	// post/write endpoints
	RegisterPostEndpoint("setSwitches", true, true, &AFicsitRemoteMonitoring::setSwitches);
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

void AFicsitRemoteMonitoring::RegisterPostEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, FEndpointFunction FunctionPtr)
{
	RegisterEndpoint("POST", APIName, bGetAll, bRequireGameThread, false, FunctionPtr);
}

void AFicsitRemoteMonitoring::RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, FEndpointFunction FunctionPtr)
{
	RegisterEndpoint("GET", APIName, bGetAll, bRequireGameThread, false, FunctionPtr);
}

void AFicsitRemoteMonitoring::RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, bool bUseFirstObject, FEndpointFunction FunctionPtr)
{
	RegisterEndpoint("GET", APIName, bGetAll, bRequireGameThread, bUseFirstObject, FunctionPtr);
}

void AFicsitRemoteMonitoring::RegisterEndpoint(const FString& Method, const FString& APIName, bool bGetAll, bool bRequireGameThread, bool bUseFirstObject, FEndpointFunction FunctionPtr)
{
	FAPIEndpoint NewEndpoint;
	NewEndpoint.APIName = APIName;
	NewEndpoint.Method = Method;
	NewEndpoint.bGetAll = bGetAll;
	NewEndpoint.bRequireGameThread = bRequireGameThread;
	NewEndpoint.bUseFirstObject = bUseFirstObject;
	NewEndpoint.FunctionPtr = FunctionPtr;
	
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

FCallEndpointResponse AFicsitRemoteMonitoring::CallEndpoint(UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess)
{
    FCallEndpointResponse Response;
    Response.bUseFirstObject = false;
    bSuccess = false;
    TArray<TSharedPtr<FJsonValue>> JsonArray;

    if (!SocketListener) {
        UE_LOG(LogHttpServer, Warning, TEXT("SocketListener is closed. Skipping request for endpoint '%s'."), *InEndpoint);
        return Response;
    }

	TArray<FString> AvailableMethods;
    bool bEndpointFound = false;

    for (FAPIEndpoint& EndpointInfo : APIEndpoints)
    {
        if (EndpointInfo.APIName == InEndpoint)
        {
	        if (RequestData.Method != EndpointInfo.Method)
        	{
	        	AvailableMethods.Add(EndpointInfo.Method);
        		continue;
        	}

            bEndpointFound = true;
            Response.bUseFirstObject = EndpointInfo.bUseFirstObject;

            try {
                if (EndpointInfo.bRequireGameThread) {
                    FThreadSafeBool bAllocationComplete = false;
                    AsyncTask(ENamedThreads::GameThread, [this, &EndpointInfo, WorldContext, RequestData, &JsonArray, &bAllocationComplete, &bSuccess]() {
						if (SocketListener && EndpointInfo.FunctionPtr)
						{
							(this->*EndpointInfo.FunctionPtr)(WorldContext, RequestData, JsonArray);  // Use direct function call
							bSuccess = true;
						}
						bAllocationComplete = true;
					});

                    while (!bAllocationComplete) {
                        FPlatformProcess::Sleep(0.0001f);
                    }
                }
				else if (SocketListener && EndpointInfo.FunctionPtr)
				{
					(this->*EndpointInfo.FunctionPtr)(WorldContext, RequestData, JsonArray);  // Use direct function call
					bSuccess = true;
				}
            } catch (const std::exception& e) {
                FString err = FString(e.what());
                UE_LOG(LogHttpServer, Error, TEXT("Exception in CallEndpoint for endpoint '%s': %s"), *InEndpoint, *err);
                AddErrorJson(JsonArray, TEXT("Exception: ") + err);
            } catch (...) {
                UE_LOG(LogHttpServer, Error, TEXT("Unknown exception in CallEndpoint for endpoint '%s'."), *InEndpoint);
                AddErrorJson(JsonArray, TEXT("Unknown exception occurred."));
            }

            break;
        }
    }

	if (AvailableMethods.Num()) {
		AddErrorJson(JsonArray, FString::Printf(
			TEXT("The %s method is not supported for this route. Supported methods: %s."),
			*RequestData.Method,
			*FString::Join(AvailableMethods, TEXT(", "))
		));
	}
    else if (!bEndpointFound) {
        UE_LOG(LogHttpServer, Warning, TEXT("No matching endpoint found for '%s'."), *InEndpoint);
        AddErrorJson(JsonArray, TEXT("No matching endpoint found."));
    }

    Response.JsonValues = JsonArray;
    return Response;
}

// Helper function to add error messages to JsonArray
void AFicsitRemoteMonitoring::AddErrorJson(TArray<TSharedPtr<FJsonValue>>& JsonArray, const FString& ErrorMessage)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
    JsonObject->SetStringField("error", ErrorMessage);
    JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));
}

FString AFicsitRemoteMonitoring::HandleEndpoint(UObject* WorldContext, FString InEndpoint, const FRequestData RequestData, bool& bSuccess)
{
	bSuccess = false;

	auto [JsonValues, bUseFirstObject] = this->CallEndpoint(WorldContext, InEndpoint, RequestData, bSuccess);

	if (bSuccess && !bUseFirstObject) return UFRM_RequestLibrary::JsonArrayToString(JsonValues, JSONDebugMode);

	// return empty object, if JsonValues is empty
	if (JsonValues.Num() == 0) return "{}";

	TSharedPtr<FJsonObject> FirstJsonObject = JsonValues[0]->AsObject();

	return UFRM_RequestLibrary::JsonObjectToString(FirstJsonObject, JSONDebugMode);
}

/*FFGServerErrorResponse AFicsitRemoteMonitoring::HandleCSSEndpoint(FString& out_json, FString InEndpoin)
{
    bool bSuccess = false;
    auto World = GetWorld();
    TArray<TSharedPtr<FJsonValue>> Json = this->CallEndpoint(World, InEndpoin, bSuccess);

    if (!bSuccess) {
        out_json = "{'error': 'Endpoint not found. Please consult Endpoint's documentation for more information.'}";
        return FFGServerErrorResponse::Ok();
    }

    FConfig_FactoryStruct config = FConfig_FactoryStruct::GetActiveConfig(World);
    out_json = UBlueprintJsonValue::StringifyArray(Json, config.JSONDebugMode);
    return FFGServerErrorResponse::Ok();

}
*/

void AFicsitRemoteMonitoring::getAll(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
    TArray<TSharedPtr<FJsonValue>> JsonArray;  // The composite JSON array to hold data from each endpoint

    // Loop through all registered endpoints
    for (const FAPIEndpoint& APIEndpoint : APIEndpoints)
    {
        // Skip endpoints that aren't marked for inclusion in `getAll`
        if (!APIEndpoint.bGetAll) continue;

        // Prepare to collect the JSON results from this endpoint
        bool bSuccess = false;
        TArray<TSharedPtr<FJsonValue>> EndpointJsonValues;

        if (APIEndpoint.bRequireGameThread)
        {
            // If the endpoint requires the GameThread, execute the call asynchronously
            FThreadSafeBool bAllocationComplete = false;
            AsyncTask(ENamedThreads::GameThread, [this, &APIEndpoint, WorldContext, RequestData, &EndpointJsonValues, &bAllocationComplete, &bSuccess]() {
                (this->*APIEndpoint.FunctionPtr)(WorldContext, RequestData, EndpointJsonValues);
                bSuccess = !EndpointJsonValues.IsEmpty();
                bAllocationComplete = true;
            });

            // Block until completion
            while (!bAllocationComplete) {
                FPlatformProcess::Sleep(0.0001f);
            }
        }
        else
        {
            // Directly invoke the endpoint function if it doesn't require the GameThread
            (this->*APIEndpoint.FunctionPtr)(WorldContext, RequestData, EndpointJsonValues);
            bSuccess = !EndpointJsonValues.IsEmpty();
        }

        // Create a JSON object to store the endpoint's result
        TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();

        if (APIEndpoint.bUseFirstObject && EndpointJsonValues.Num() > 0)
        {
            // If only the first object is required, add it to the JSON object
            TSharedPtr<FJsonObject> FirstJsonObject = EndpointJsonValues[0]->AsObject();
            JsonObject->SetObjectField(APIEndpoint.APIName, FirstJsonObject.IsValid() ? FirstJsonObject : MakeShared<FJsonObject>());
        }
        else
        {
            // Otherwise, include the entire array of JSON values
            JsonObject->SetArrayField(APIEndpoint.APIName, EndpointJsonValues);
        }

        // Add the constructed JSON object for this endpoint to the composite JSON array
        JsonArray.Add(MakeShared<FJsonValueObject>(JsonObject));
    }

    // Assign the assembled JSON array to the output parameter
    OutJsonArray = JsonArray;
}

