#include "FicsitRemoteMonitoring.h"

#include "Communication.h"
#include "Config_DiscITStruct.h"
#include "Config_FactoryStruct.h"
#include "Config_HTTPStruct.h"
#include "Config_SerialStruct.h"
#include "Drones.h"
#include "EngineUtils.h"
#include "EventsLibrary.h"
#include "FactoryLibrary.h"
#include "FicsitRemoteMonitoringModule.h"
#include "Async/Async.h"
#include "FRM_Request.h"
#include "Hypertubes.h"
#include "Inventory.h"
#include "Logistics.h"
#include "NativeHookManager.h"
#include "NotificationLoader.h"
#include "PlayerLibrary.h"
#include "Research.h"
#include "Resources.h"
#include "Session.h"
#include "StructuredLog.h"
#include "SubsystemActorManager.h"
#include "Support.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "TimerManager.h"
#include "Trains.h"
#include "Vehicles.h"
#include "Engine/World.h"

us_listen_socket_t* SocketListener;
bool SocketRunning = false;

AFicsitRemoteMonitoring* AFicsitRemoteMonitoring::Get(UWorld* WorldContext)
{
	for (TActorIterator<AFicsitRemoteMonitoring> It(WorldContext, StaticClass(), EActorIteratorFlags::AllActors); It; ++It) {
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

    // get config structs
    auto HttpConfig = FConfig_HTTPStruct::GetActiveConfig(GetWorld());
    auto SerialConfig = FConfig_SerialStruct::GetActiveConfig(GetWorld());

    if (HttpConfig.Web_Autostart) { StartWebSocketServer(); }
    if (SerialConfig.COM_Autostart) { InitSerialDevice(); }

    UWorld* World = GetWorld();

	// generate new authentication token if no token is available
	if (HttpConfig.Authentication_Token.IsEmpty())
	{
		HttpConfig.Authentication_Token = GenerateAuthToken(32);
		UE_LOG(LogHttpServer, Warning, TEXT("Authentication Token not set, generated a new token: %s"), *HttpConfig.Authentication_Token);
		HttpConfig.Save(World);
	}
	
    // store JSONDebugMode into a local property to prevent crash while access to GetActiveConfig while the EndPlay process
    const auto FactoryConfig = FConfig_FactoryStruct::GetActiveConfig(World);
    JSONDebugMode = FactoryConfig.JSONDebugMode;
	
    World->GetTimerManager().SetTimer(
        TimerHandle,  // The timer handle
        this,         // The instance of the class
        &AFicsitRemoteMonitoring::PushUpdatedData,  // Pointer to member function
        HttpConfig.WebSocketPushCycle,        // Time interval in seconds
        true          // Whether to loop the timer (true = repeating)
    );
	
	// Register the callback to ensure WebSocket is stopped on crash/exit
	FCoreDelegates::OnExit.AddUObject(this, &AFicsitRemoteMonitoring::StopWebSocketServer);
}

FString AFicsitRemoteMonitoring::GenerateAuthToken(const int32 Length)
{
	const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	const int32 CharactersCount = Characters.Len();

	FString RandomString;
	for (int32 i = 0; i < Length; ++i)
	{
		RandomString.AppendChar(Characters[FMath::RandRange(0, CharactersCount - 1)]);
	}

	return RandomString;
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

                    //UE_LOG(LogHttpServer, Log, TEXT("Request RelativePath: %s"), *RelativePath);
                    //UE_LOG(LogHttpServer, Log, TEXT("Request FilePath: %s"), *FilePath);

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

                app.get("/api/:APIEndpoint", [this, World, config](auto* res, auto* req) {
                    std::string url(req->getParameter("APIEndpoint"));
                    FString Endpoint = FString(url.c_str());

                    // Log the request URL
                    //UE_LOGFMT(LogHttpServer, Log, "Request URL: {0}", Endpoint);

                	FRequestData RequestData;
                	RequestData.bIsAuthorized = IsAuthorizedRequest(req, config.Authentication_Token);
                    HandleApiRequest(World, res, req, Endpoint, RequestData);
                });

            	app.options("/*", [this, World](auto* res, uWS::HttpRequest* req)
            	{
            		UFRM_RequestLibrary::AddResponseHeaders(res, false);
            		res->writeHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
            			->writeHeader("Access-Control-Allow-Headers", "Content-Type, X-FRM-Authorization");
            		res->end();
            	});
            	
            	app.post("/*", [this, World, config](auto* res, uWS::HttpRequest* req)
            	{
		            const std::string URL(req->getUrl().begin(), req->getUrl().end());
					FString RelativePath = FString(URL.c_str()).Mid(1);

            		res->onData([this, res, req, World, RelativePath, config](const std::string_view data, bool)
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
			            	RequestData.bIsAuthorized = IsAuthorizedRequest(req, config.Authentication_Token);
			            	
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

                app.get("/*", [this, UIPath, World, config](auto* res, uWS::HttpRequest* req) {
                    if (!res) return;

                    std::string url(req->getUrl().begin(), req->getUrl().end());
                    
                    bool bFileExists = false;
                    // Remove initial '/'
                    FString RelativePath = FString(url.c_str()).Mid(1);
                    FString FilePath = FPaths::Combine(UIPath, RelativePath);
                    FString FileContent;

                	/* Retaining until log levels are created or debugging required
                    UE_LOG(LogHttpServer, Log, TEXT("Request RelativePath/FilePath: %s %s"), *RelativePath, *FilePath);
					*/
                	
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
                    	RequestData.bIsAuthorized = IsAuthorizedRequest(req, config.Authentication_Token);
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

	FString MessageContent = FString(message.data()).Left(message.size());

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

        bool bSuccess = false;
    	int32 ErrorCode = 404;

    	FRequestData RequestData = FRequestData();
    	RequestData.bIsAuthorized = true;

        FString Json;

    	HandleEndpoint(Endpoint, RequestData, bSuccess, ErrorCode, Json, EInterfaceType::Socket);

    	FTCHARToUTF8 Converted(*Json);
    	const char* UWSOutput = Converted.Get();
    	
        // Broadcast updated data to all clients subscribed to this endpoint
        for (uWS::WebSocket<false, true, FWebSocketUserData>* Client : Elem.Value) {
            Client->send(UWSOutput, uWS::OpCode::TEXT);
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

            //UE_LOG(LogHttpServer, Log, TEXT("Binary File Found Returning: %s"), *FilePath);

            res->writeHeader("Content-Type", TCHAR_TO_UTF8(*ContentType));
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
            //UE_LOG(LogHttpServer, Log, TEXT("File Found Returning: %s"), *FilePath);

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

bool AFicsitRemoteMonitoring::IsAuthorizedRequest(uWS::HttpRequest* req, FString RequiredToken)
{
	const std::string_view Header = req->getHeader("x-frm-authorization");
	const FString AuthorizationHeader = FString(Header.data()).Left(Header.size());
	if (AuthorizationHeader.IsEmpty()) return false;

	return AuthorizationHeader == RequiredToken;
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
	int32 ErrorCode = 404;
	FString OutJson;

	this->HandleEndpoint(Endpoint, RequestData, bSuccess, ErrorCode, OutJson, EInterfaceType::Web);

    if (bSuccess) {
        //UE_LOGFMT(LogHttpServer, Log, "API Found Returning: {Endpoint}", Endpoint);
        UFRM_RequestLibrary::AddResponseHeaders(res, true);
        res->end(TCHAR_TO_UTF8(*OutJson));
    	return;
    }

    switch (ErrorCode)
    {
    case 401:
	    UFRM_RequestLibrary::SendErrorJson(res, "401 Unauthorized", OutJson);
	    break;
    case 404:
	    UE_LOGFMT(LogHttpServer, Log, "API Not Found: {Endpoint}", Endpoint);
	    UFRM_RequestLibrary::SendErrorJson(res, "404 Not Found", OutJson);
	    break;
    case 405:
	    UFRM_RequestLibrary::SendErrorJson(res, "405 Method Not Allowed", OutJson);
	    break;
    default:
	    UE_LOGFMT(LogHttpServer, Log, "Unknown Error {Endpoint} {ErrorCode}", Endpoint, ErrorCode);
	    UFRM_RequestLibrary::SendErrorJson(res, "500 Internal Server Error", OutJson);
    }
}

void AFicsitRemoteMonitoring::InitAPIRegistry()
{
	//Registering Endpoints: API Name, bRequireGameThread, FunctionPtr
	RegisterEndpoint(FAPIEndpoint("GET", "getAssembler", &UFactoryLibrary::getAssembler));
	RegisterEndpoint(FAPIEndpoint("GET", "getBelts", &ULogistics::getBelts));
	RegisterEndpoint(FAPIEndpoint("GET", "getLifts", &ULogistics::getLifts));
	RegisterEndpoint(FAPIEndpoint("GET", "getElevators", &USupport::getElevators));
	RegisterEndpoint(FAPIEndpoint("GET", "getBiomassGenerator", &UPower::getBiomassGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getBlender", &UFactoryLibrary::getBlender));
	RegisterEndpoint(FAPIEndpoint("GET", "getCables", &UPower::getCables));
	RegisterEndpoint(FAPIEndpoint("GET", "getCloudInv", &UInventory::getCloudInv));
	RegisterEndpoint(FAPIEndpoint("GET", "getCoalGenerator", &UPower::getCoalGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getConstructor", &UFactoryLibrary::getConstructor));
	RegisterEndpoint(FAPIEndpoint("GET", "getConverter", &UFactoryLibrary::getConverter));
	RegisterEndpoint(FAPIEndpoint("GET", "getDoggo", &UPlayerLibrary::getDoggo).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getDrone", &UDrones::getDrone).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getDroneStation", &UDrones::getDroneStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getDropPod", &UResources::getDropPod).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getEncoder", &UFactoryLibrary::getEncoder));
	RegisterEndpoint(FAPIEndpoint("GET", "getExplorationSink", &USession::getExplorationSink));
	RegisterEndpoint(FAPIEndpoint("GET", "getExplorer", &UVehicles::getExplorer).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getExtractor", &UResources::getExtractor));
	RegisterEndpoint(FAPIEndpoint("GET", "getFactoryCart", &UVehicles::getFactoryCart));
	RegisterEndpoint(FAPIEndpoint("GET", "getFoundry", &UFactoryLibrary::getFoundry));
	RegisterEndpoint(FAPIEndpoint("GET", "getFrackingActivator", &UResources::getFrackingActivator));
	RegisterEndpoint(FAPIEndpoint("GET", "getFuelGenerator", &UPower::getFuelGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getGeothermalGenerator", &UPower::getGeothermalGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getHUBTerminal", &USupport::getHubTerminal).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getHyperEntrance", &UHypertubes::getHyperEntrance));
	RegisterEndpoint(FAPIEndpoint("GET", "getHypertube", &UHypertubes::getHypertube));
	RegisterEndpoint(FAPIEndpoint("GET", "getHyperJunctions", &UHypertubes::getHyperJunctions));
	RegisterEndpoint(FAPIEndpoint("GET", "getManufacturer", &UFactoryLibrary::getManufacturer));
	RegisterEndpoint(FAPIEndpoint("GET", "getModList", &USession::getModList));
	RegisterEndpoint(FAPIEndpoint("GET", "getNuclearGenerator", &UPower::getNuclearGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getPackager", &UFactoryLibrary::getPackager));
	RegisterEndpoint(FAPIEndpoint("GET", "getParticle", &UFactoryLibrary::getParticle));
	RegisterEndpoint(FAPIEndpoint("GET", "getPipes", &ULogistics::getPipes));
	RegisterEndpoint(FAPIEndpoint("GET", "getPipeJunctions", &ULogistics::getPipeJunctions));
	RegisterEndpoint(FAPIEndpoint("GET", "getPlayer", &UPlayerLibrary::getPlayer).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getPortal", &USupport::getPortal));
	RegisterEndpoint(FAPIEndpoint("GET", "getPower", &UPower::getPower));
	RegisterEndpoint(FAPIEndpoint("GET", "getPowerSlug", &UResources::getPowerSlug).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getArtifacts", &UResearch::getArtifacts).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getPowerUsage", &UPower::getPowerUsage));
	RegisterEndpoint(FAPIEndpoint("GET", "getProdStats", &USession::getProdStats));
	RegisterEndpoint(FAPIEndpoint("GET", "getPump", &ULogistics::getPump));
	RegisterEndpoint(FAPIEndpoint("GET", "getRadarTower", &USupport::getRadarTower));
	RegisterEndpoint(FAPIEndpoint("GET", "getRecipes", &UResearch::getRecipes).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getRefinery", &UFactoryLibrary::getRefinery));
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceGeyser", &UResources::getResourceGeyser).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceDeposit", &UResources::getResourceDeposit).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceNode", &UResources::getResourceNode).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceSink", &USession::getResourceSink));
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceSinkBuilding", &USupport::getResourceSinkBuilding));
	RegisterEndpoint(FAPIEndpoint("GET", "getResourceWell", &UResources::getResourceWell).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getSplitterMerger", &ULogistics::getSplitterMerger));
	RegisterEndpoint(FAPIEndpoint("GET", "getSessionInfo", &USession::getSessionInfo).RequiresGameThread().UseFirstObject());
	RegisterEndpoint(FAPIEndpoint("GET", "getSchematics", &UResearch::getSchematics).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getSinkList", &USession::getSinkList).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getSmelter", &UFactoryLibrary::getSmelter));
	RegisterEndpoint(FAPIEndpoint("GET", "getSpaceElevator", &USupport::getSpaceElevator));
	RegisterEndpoint(FAPIEndpoint("GET", "getStorageInv", &UInventory::getStorageInv));
	RegisterEndpoint(FAPIEndpoint("GET", "getSwitches", &UPower::getSwitches));
	RegisterEndpoint(FAPIEndpoint("GET", "getThroughputCounter", &ULogistics::getThroughputCounter));
	RegisterEndpoint(FAPIEndpoint("GET", "getTractor", &UVehicles::getTractor));
	RegisterEndpoint(FAPIEndpoint("GET", "getTradingPost", &USupport::getTradingPost));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrains", &UTrains::getTrains));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrainRails", &UTrains::getTrainRails));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrainStation", &UTrains::getTrainStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getTruck", &UVehicles::getTruck));
	RegisterEndpoint(FAPIEndpoint("GET", "getTruckStation", &UVehicles::getTruckStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getWorldInv", &UInventory::getWorldInv));
	RegisterEndpoint(FAPIEndpoint("GET", "getResearchTrees", &UResearch::getResearchTrees).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getChatMessages", &UCommunication::getChatMessages).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getItemPickups", &UResources::getItemPickups).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getUObjectCount", &USession::getUObjectCount).UseFirstObject());
	RegisterEndpoint(FAPIEndpoint("GET", "getVehiclePaths", &UVehicles::getVehiclePaths));

	// event endpoints
	RegisterEndpoint(FAPIEndpoint("GET", "getFallingGiftBundles", &UEventsLibrary::getFallingGiftBundles).RequiresGameThread());

	//FRM API Endpoint Groups
	RegisterEndpoint(FAPIEndpoint("GET", "getAll", &AFicsitRemoteMonitoring::getAll));
	RegisterEndpoint(FAPIEndpoint("GET", "getFactory", &UFactoryLibrary::getFactory));
	RegisterEndpoint(FAPIEndpoint("GET", "getGenerators", &UPower::getGenerators));
	RegisterEndpoint(FAPIEndpoint("GET", "getVehicles", &UVehicles::getVehicles));

	// post/write endpoints
	RegisterEndpoint(FAPIEndpoint("POST", "setSwitches", &UPower::setSwitches).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "setEnabled", &UCommunication::setEnabled).RequiresAuthentication());
	RegisterEndpoint(FAPIEndpoint("POST", "sendChatMessage", &UCommunication::sendChatMessage).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "createPing", &UCommunication::createPing).RequiresAuthentication().RequiresGameThread());
}

FString AFicsitRemoteMonitoring::FlavorTextRandomizer(EFlavorType FlavorType) {

	auto World = GetWorld();	

	FString DefaultPath = FPaths::ProjectDir() + "Mods/FicsitRemoteMonitoring/JSON/Outage.json";
	FString JsonPath;
	FString WebhookJson;
	TSharedPtr<FJsonObject> FlavorJson;
	TArray<FString> FlavorArray;
	TArray<FString> BufferArray;
	TArray<FString> DetrimentalArray;
	TArray<FString> PositiveArray;
				
	auto config = FConfig_DiscITStruct::GetActiveConfig(World);


	JsonPath = config.OutageJSON;

	if (config.OutageJSON.IsEmpty()) { 
		JsonPath = DefaultPath; 
	}

	UNotificationLoader::FileLoadString(JsonPath, WebhookJson);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(WebhookJson);
	FJsonSerializer::Deserialize(Reader, FlavorJson);

	FlavorJson->TryGetStringArrayField("Any", FlavorArray);
	FlavorJson->TryGetStringArrayField("Detrimental", DetrimentalArray);
	FlavorJson->TryGetStringArrayField("Positive", PositiveArray);

	switch (FlavorType)
	{
		case EFlavorType::Battery:
			if (FlavorJson->TryGetStringArrayField("Battery", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::Power:
			if (FlavorJson->TryGetStringArrayField("Power", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::Train:
			if (FlavorJson->TryGetStringArrayField("Train", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(DetrimentalArray);
			};
			break;
		
		case EFlavorType::Doggo:
			if (FlavorJson->TryGetStringArrayField("Doggo", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
		
		case EFlavorType::Player:
			if (FlavorJson->TryGetStringArrayField("Player", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
		
		case EFlavorType::Research:
			if (FlavorJson->TryGetStringArrayField("Research", BufferArray))
			{
				FlavorArray.Append(BufferArray);
				FlavorArray.Append(PositiveArray);
			};
			break;
	}
	
	
	
	int32 len = FlavorArray.Num();
	int32 rng = UKismetMathLibrary::RandomIntegerInRange(0, len);

	return FlavorArray[rng];

}

void AFicsitRemoteMonitoring::InitOutageNotification() {
	#if !WITH_EDITOR

	auto World = GetWorld();
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

void AFicsitRemoteMonitoring::RegisterEndpoint(const FAPIEndpoint& Endpoint)
{
	APIEndpoints.Add(Endpoint);

	UE_LOGFMT(LogHttpServer, Log, "Registered API Endpoint: {APIName} - Current number of endpoints registered: {1}", Endpoint.APIName, APIEndpoints.Num());
	
}

FCallEndpointResponse AFicsitRemoteMonitoring::CallEndpoint(UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode)
{
    FCallEndpointResponse Response;
    Response.bUseFirstObject = false;
    bSuccess = false;
    TArray<TSharedPtr<FJsonValue>> JsonArray;
	
	TArray<FString> AvailableMethods;
    bool bEndpointFound = false;

    for (FAPIEndpoint& EndpointInfo : APIEndpoints)
    {
	    if (EndpointInfo.APIName != InEndpoint) continue;

        if (RequestData.Method != EndpointInfo.Method)
        {
	        AvailableMethods.Add(EndpointInfo.Method);
        	continue;
        }

	    if (EndpointInfo.bRequiresAuthentication && !RequestData.bIsAuthorized)
    	{
    		ErrorCode = 401;
    		AddErrorJson(JsonArray, TEXT("Unauthorized"));
    		Response.JsonValues = JsonArray;
    		return Response;
    	}

        bEndpointFound = true;
        Response.bUseFirstObject = EndpointInfo.bUseFirstObject;

        try {
            if (EndpointInfo.bRequireGameThread && RequestData.Interface != EInterfaceType::Server) {
                FThreadSafeBool bAllocationComplete = false;
                AsyncTask(ENamedThreads::GameThread, [&EndpointInfo, WorldContext, RequestData, &JsonArray, &bAllocationComplete, &ErrorCode, &bSuccess]() {
					//if (SocketListener && EndpointInfo.FunctionPtr)
					if (EndpointInfo.FunctionPtr)
					{
						(EndpointInfo.FunctionPtr)(WorldContext, RequestData, JsonArray);  // Use direct function call
						ErrorCode = 200;
						bSuccess = true;
					}
					bAllocationComplete = true;
				});

                while (!bAllocationComplete) {
                    FPlatformProcess::Sleep(0.0001f);
                }
            }
			//else if (SocketListener && EndpointInfo.FunctionPtr)
			else if (EndpointInfo.FunctionPtr)
			{
				(EndpointInfo.FunctionPtr)(WorldContext, RequestData, JsonArray);  // Use direct function call
				ErrorCode = 200;
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

	if (AvailableMethods.Num()) {
		ErrorCode = 405;
		AddErrorJson(JsonArray, FString::Printf(
			TEXT("The %s method is not supported for this route. Supported methods: %s."),
			*RequestData.Method,
			*FString::Join(AvailableMethods, TEXT(", "))
		));
	}
    else if (!bEndpointFound) {
        UE_LOG(LogHttpServer, Warning, TEXT("No matching endpoint found for '%s'."), *InEndpoint);
    	ErrorCode = 404;
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

void AFicsitRemoteMonitoring::HandleEndpoint(FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode, FString& Out_Data, EInterfaceType Interface)
{
	bSuccess = false;

	UObject* WorldContext = this->GetWorld();

	RequestData.Interface = Interface;

	auto [JsonValues, bUseFirstObject] = this->CallEndpoint(WorldContext, InEndpoint, RequestData, bSuccess, ErrorCode);

	if (bSuccess && !bUseFirstObject)
	{
		if (Interface == EInterfaceType::Socket)
		{
			const TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
			JsonObject->Values.Add("endpoint", MakeShared<FJsonValueString>(InEndpoint));
			JsonObject->Values.Add("data", MakeShared<FJsonValueArray>(JsonValues));
			Out_Data = UFRM_RequestLibrary::JsonObjectToString(JsonObject, JSONDebugMode);
		}
		else
		{
			Out_Data = UFRM_RequestLibrary::JsonArrayToString(JsonValues, JSONDebugMode);
		}
	} else if (JsonValues.Num() == 0) {
		Out_Data = "{}";
	}
	else
	{
		// return empty object, if JsonValues is empty
		TSharedPtr<FJsonObject> FirstJsonObject = JsonValues[0]->AsObject();
		Out_Data = UFRM_RequestLibrary::JsonObjectToString(FirstJsonObject, JSONDebugMode);
	}
}



