#include "FicsitRemoteMonitoring.h"

#include <sstream>

#include "Runtime/Core/Public/Logging/LogCategory.h"
#include "EngineUtils.h"
#include "FGPowerCircuit.h"
#include "FGRailroadSubsystem.h"
#include "FicsitRemoteMonitoringModule.h"
#include "Async/Async.h"
#include "FRM_Request.h"
#include "Inventory.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "TimerManager.h"
#include "Endpoints/Factory/FactoryLibrary.h"
#include "Endpoints/Factory/Logistics.h"
#include "Endpoints/Factory/Support.h"
#include "Endpoints/Travel/Drones.h"
#include "Endpoints/Travel/Hypertubes.h"
#include "Endpoints/Travel/Trains.h"
#include "Endpoints/Travel/Vehicles.h"
#include "Endpoints/World/Communication.h"
#include "Endpoints/World/EventsLibrary.h"
#include "Endpoints/World/Inventory.h"
#include "Endpoints/World/PlayerLibrary.h"
#include "Endpoints/World/Research.h"
#include "Endpoints/World/Resources.h"
#include "Endpoints/World/Session.h"
#include "Engine/World.h"
#include "FGPlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "Libraries/Validation.h"
#include "Misc/FileHelper.h"
#include "Patching/NativeHookManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

	// Populate the player display-name cache on connect (PLYR-03, D-04).
	InitPlayerConnectCache();

	const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.AuthenticationToken"), "");
	
	// Debug log to verify token retrieval -Porisius
	// UE_LOGFMT(LogHttpServer, Log, "DEBUG: AuthToken - {AuthToken}", *AuthToken);
	
	if (AuthToken.IsEmpty())
	{
		if (!UFRMConfigManager::SetConfigFromInput(TEXT("uWS.AuthenticationToken"), GenerateAuthToken(32), false))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to apply setting"));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("Generated and saved new token: %s"), *AuthToken);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Token already exists."));
	}
	
	if (UFRMConfigManager::GetConfigOrDefault<bool>(TEXT("uWS.Autostart"), false))
	{
		StartWebSocketServer(true);
	}
	
	if (UFRMConfigManager::GetConfigOrDefault<bool>(TEXT("Serial.Autostart"), false))
	{
		InitSerialDevice();
	}

	// Register the callback to ensure WebSocket is stopped on crash/exit
	FCoreDelegates::OnExit.AddUObject(this, &AFicsitRemoteMonitoring::StopWebSocketServer);
}

void AFicsitRemoteMonitoring::StartWebSocketPushDataLoop()
{
	if (bHasRunningPushDataLoop) return;

	// This function is only ever invoked from ProcessClientRequest's marshaled game-thread subscribe
	// branch, so this read/write of bHasRunningPushDataLoop is already game-thread owned. Set it here,
	// synchronously, before spawning the push-pacing thread (D-04: all writes/reads of this flag must be
	// game-thread owned end-to-end — do not move this into the Async(Thread,...) lambda below).
	bHasRunningPushDataLoop = true;

	Async(EAsyncExecution::Thread, [this]()
	{
		UE_LOGFMT(LogHttpServer, Log, "Starting PushUpdatedData loop");
		while (SocketRunning && !bShouldStop)
		{
			const float PushCycle = UFRMConfigManager::GetConfigOrDefault<float>(TEXT("uWS.PushCycle"), 5.0f);

			PushUpdatedData();
			FPlatformProcess::Sleep(PushCycle);
		}
		UE_LOGFMT(LogHttpServer, Log, "Stopped PushUpdatedData loop");

		// Marshal the terminal write back to the game thread (D-04) instead of writing it directly from
		// this push-pacing thread.
		TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
		AsyncTask(ENamedThreads::GameThread, [WeakThis]()
		{
			if (AFicsitRemoteMonitoring* Self = WeakThis.Get())
			{
				Self->bHasRunningPushDataLoop = false;
			}
		});
	});
}

void AFicsitRemoteMonitoring::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Ensure the server is stopped during normal gameplay exit
	StopWebSocketServer();
	Super::EndPlay(EndPlayReason);
}

void AFicsitRemoteMonitoring::StopWebSocketServer()
{
	bShouldStop = true;

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

        // D-04 shutdown safety / resolves RESEARCH Open Question #1: ws->close() is a socket-owning call,
        // the same thread-ownership violation class as PushUpdatedData's send() — it must run on the uWS
        // loop thread, not here on the game thread. Snapshot (ws, ClientID) pairs now (game-thread-owned
        // ConnectedClients/ClientGenerations are still safe to read here), then defer the actual close()
        // onto the loop thread; the deferred callback re-validates against LoopLiveSockets before
        // touching ws. Safe no-op if CapturedLoop is already null (loop thread already torn down).
        if (CapturedLoop)
        {
            TArray<TPair<uWS::WebSocket<false, true, FWebSocketUserData>*, int32>> ClientsToClose;
            ClientsToClose.Reserve(ConnectedClients.Num());

            for (const auto ConnectedClient : ConnectedClients)
            {
                if (const int32* FoundClientID = ClientGenerations.Find(ConnectedClient))
                {
                    ClientsToClose.Emplace(ConnectedClient, *FoundClientID);
                }
            }

            TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
            CapturedLoop->defer([WeakThis, ClientsToClose]()
            {
                AFicsitRemoteMonitoring* Self = WeakThis.Get();
                if (!Self)
                {
                    return;
                }

                for (const auto& ClientPair : ClientsToClose)
                {
                    const int32* FoundClientID = Self->LoopLiveSockets.Find(ClientPair.Key);
                    if (FoundClientID && *FoundClientID == ClientPair.Value)
                    {
                        ClientPair.Key->close();
                    }
                }
            });
        }

        ConnectedClients.Empty();
    }

    // clear endpoint subscribers
    EndpointSubscribers.Empty();
}

FArduinoConfig AFicsitRemoteMonitoring::GetSerialConfig()
{
	FArduinoConfig Config;
	
	Config.AutoStart = UFRMConfigManager::GetConfigOrDefault<bool>("Serial.AutoStart", false);
	Config.BaudRate = UFRMConfigManager::GetConfigOrDefault<int32>("Serial.BaudRate", 9600);
	Config.RS232_Port = UFRMConfigManager::GetConfigOrDefault<FString>("Serial.Port", "COM3");
	Config.SerialDelay = UFRMConfigManager::GetConfigOrDefault<float>("Serial.SerialDelay", 0.5);
	Config.SerialStack = UFRMConfigManager::GetConfigOrDefault<int32>("Serial.AutoStart", 4098);
		
	return Config;
}

void AFicsitRemoteMonitoring::StartWebSocketServer(bool bSkipIfRunning) 
{
    UE_LOGFMT(LogHttpServer, Log, "Initializing WebSocket Service");

    if (SocketRunning)
    {
	    if (bSkipIfRunning)
    	{
	    	UE_LOG(LogHttpServer, Log, TEXT("Websocket Thread is already running. Stop start process."));

	    	return;
    	}

        UE_LOG(LogHttpServer, Log, TEXT("Old Websocket Thread is still running, try again in 3 seconds..."));

        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
        {
            // Sleep for the specified delay
            FPlatformProcess::Sleep(3.0f);
        
            // Then switch back to the game thread to run the task
            AsyncTask(ENamedThreads::GameThread, [this]()
            {
                StartWebSocketServer(true);
            });
        });
        return;
    }

        // WebSocket server logic runs in a separate thread
        WebServer = Async(EAsyncExecution::Thread, [this]() {
            try {
                auto app = uWS::App();

                // Captured once, on the uWS loop thread (uWS::Loop::get() is thread-local — calling it
                // from any other thread returns a different, unrelated loop). Consumed by PushUpdatedData
                // and StopWebSocketServer to defer() outbound send()/close() onto this thread (THRD-02).
                CapturedLoop = uWS::Loop::get();

                auto World = GetWorld();

            	const int32 port = UFRMConfigManager::GetConfigOrDefault<int32>(TEXT("uWS.Port"), 8080);
            	const FString Root = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.Root"), "");            	

                FString ModPath = FPaths::ProjectModsDir() + "GameFeatures/FicsitRemoteMonitoring/";
                FString IconsPath = ModPath + "Icons";
                FString UIPath;

                if (Root.IsEmpty()) {
                    UIPath = ModPath + "www";
                }
                else
                {
                    UIPath = Root;
                };

                // Define WebSocket behavior
                uWS::App::WebSocketBehavior<FWebSocketUserData> wsBehavior;

                wsBehavior.compression = uWS::SHARED_COMPRESSOR;

                // Close handler (for when a client disconnects). Runs on the uWS loop thread; per uWS's
                // open->close validity guarantee, ws and its userdata are still safely dereferenceable here
                // (closeHandler fires before ~WebSocketData()). This is the last safe touch of ws on this
                // thread. All shared-state mutation (ConnectedClients/EndpointSubscribers/ClientGenerations)
                // is marshaled to the game thread and re-validated by generation tag (THRD-01, D-02/D-03).
                wsBehavior.close = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
                    const int32 ClosedClientID = ws->getUserData()->ClientID;

                    // Loop-thread-only bookkeeping, consumed by the outbound deferred-send path (Plan 03).
                    LoopLiveSockets.Remove(ws);

                    TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
                    AsyncTask(ENamedThreads::GameThread, [WeakThis, ws, ClosedClientID]()
                    {
                        AFicsitRemoteMonitoring* Self = WeakThis.Get();
                        if (!Self || Self->bShouldStop) return;   // shutdown/teardown race guard (D-04)

                        // Re-validate against the generation map (D-03) rather than trusting submission
                        // order (Pitfall 2) — a stale/reused ws must not mutate the wrong client's state.
                        if (Self->IsClientCurrent(ws, ClosedClientID))
                        {
                            Self->ConnectedClients.Remove(ws);
                            Self->ClientGenerations.Remove(ws);

                            // Folded from the retired OnClientDisconnected: remove ws from every
                            // subscription so no EndpointSubscribers mutation remains on the loop thread.
                            for (auto& Elem : Self->EndpointSubscribers)
                            {
                                Elem.Value.Remove(ws);
                            }
                        }

                        UE_LOG(LogHttpServer, Log, TEXT("Client Disconnected. Remaining connections: %d"), Self->ConnectedClients.Num());
                    });
                };

                // Message handler (for when a client sends a message)
                wsBehavior.message = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
                    OnMessageReceived(ws, message, opCode);  // Make sure this signature matches
                };

                // Open handler (for when a client connects). Runs on the uWS loop thread; ws is provably
                // valid here (freshly opened). Mint the ClientID generation tag synchronously (safe: this is
                // the loop-thread-only counter/bookkeeping), then marshal the registry mutation to the game
                // thread — fire-and-forget, no spin-wait (D-02).
                wsBehavior.open = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws)
                {
                    const int32 NewClientID = NextClientIDCounter++;
                    ws->getUserData()->ClientID = NewClientID;
                    LoopLiveSockets.Add(ws, NewClientID);

                    TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
                    AsyncTask(ENamedThreads::GameThread, [WeakThis, ws, NewClientID]()
                    {
                        AFicsitRemoteMonitoring* Self = WeakThis.Get();
                        if (!Self || Self->bShouldStop) return;   // shutdown/teardown race guard (D-04)

                        Self->ConnectedClients.Add(ws);
                        Self->ClientGenerations.Add(ws, NewClientID);  // ws used only as an opaque map key
                        UE_LOG(LogHttpServer, Log, TEXT("Client Connected. Connections: %d"), Self->ConnectedClients.Num());
                    });
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

                app.get("/api/:APIEndpoint", [this, World](auto* res, auto* req) {
                    std::string url(req->getParameter("APIEndpoint"));
                    FString Endpoint = FString(url.c_str());

                    // Log the request URL
                    //UE_LOGFMT(LogHttpServer, Log, "Request URL: {0}", Endpoint);

                	const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.AuthenticationToken"), "");

                	FRequestData RequestData;
                	RequestData.bIsAuthorized = IsAuthorizedRequest(req, AuthToken);
                    HandleApiRequest(World, res, req, Endpoint, RequestData);
                });

            	app.options("/*", [this, World](auto* res, uWS::HttpRequest* req)
            	{
            		UFRM_RequestLibrary::AddResponseHeaders(res, false);
            		res->writeHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
            			->writeHeader("Access-Control-Allow-Headers", "Content-Type, X-FRM-Authorization");
            		res->end();
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
			            		UE_LOG(LogHttpServer, Error, TEXT("Invalid JSON or failed to deserialize"));
			            		return UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            	}

			            	const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.AuthenticationToken"), "");
			            	
			            	FRequestData RequestData;
			            	RequestData.Method = "POST";
			            	RequestData.bIsAuthorized = IsAuthorizedRequest(req, AuthToken);
			            	
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
			            		UE_LOG(LogHttpServer, Error, TEXT("Invalid JSON Array or Object"));
			            		return UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            	}

			            	HandleApiRequest(World, res, req, RelativePath, RequestData);
			            }
			            catch (const std::exception& e)
			            {
			            	UE_LOG(LogHttpServer, Error, TEXT("Request Exception: %s"), *FString(e.what()));
			            	UFRM_RequestLibrary::SendErrorMessage(res, "400 Bad Request", FString("Invalid Request Body"));
			            }
            		});

            		res->onAborted([]() {});
            	});

                app.get("/*", [this, UIPath, World](auto* res, uWS::HttpRequest* req) {
                    if (!res) return;

                    std::string url(req->getUrl().begin(), req->getUrl().end());

                	const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.AuthenticationToken"), "");
                    
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
                    	RequestData.bIsAuthorized = IsAuthorizedRequest(req, AuthToken);
                        HandleApiRequest(World, res, req, RelativePath, RequestData);
                    }
                });

                app.ws<FWebSocketUserData>("/*", std::move(wsBehavior));

                app.listen(port, [this, port](us_listen_socket_t* token) {

                    UE_LOG(LogHttpServer, Warning, TEXT("Attempting to listen on port %d"), port);

                	FString Reason;
                	if (!UFRMValidation::IsTcpPortAvailable(port, Reason))
                	{
                		UE_LOG(LogHttpServer, Error, TEXT("Port %d unavailable: %s"), port, *Reason);
                	}
                	else if (token) {
                        SocketListener = token;
                        UE_LOGFMT(LogHttpServer, Warning, "Listening on port {port}", port);

                    	SocketRunning = true;
                    	bShouldStop = false;
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

            // Teardown safety (RESEARCH Open Question #2): clear the captured loop pointer once this
            // thread's app.run() returns (normally or via exception) so no push-pacing/game-thread caller
            // can defer() onto a stale/torn-down loop after this thread exits.
            CapturedLoop = nullptr;
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

bool AFicsitRemoteMonitoring::IsClientCurrent(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int32 ClientID) const
{
    // Game-thread-only. ws is used strictly as an opaque map key here — never dereferenced — so this is
    // safe to call even for a ws that has since been closed/freed/reused (D-03 generation-tag defense).
    const int32* FoundClientID = ClientGenerations.Find(ws);
    return FoundClientID != nullptr && *FoundClientID == ClientID;
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
    // Loop-thread-only: extract plain values while ws is still safely dereferenceable — this runs
    // synchronously inside wsBehavior.message's own call stack (per uWS's open->close validity
    // guarantee). No shared-state mutation happens here; the actual EndpointSubscribers/
    // bHasRunningPushDataLoop work is marshaled to the game thread below (THRD-01).
    const FString Action = JsonRequest->GetStringField(TEXT("action"));
    const int32 RequestClientID = ws->getUserData()->ClientID;

    TArray<FString> EndpointNames;
    const TArray<TSharedPtr<FJsonValue>>* EndpointsArray;
    FString SingleEndpoint;

    if (JsonRequest->TryGetArrayField(TEXT("endpoints"), EndpointsArray))
    {
        for (const TSharedPtr<FJsonValue>& EndpointValue : *EndpointsArray)
        {
            EndpointNames.Add(EndpointValue->AsString());
        }
    }
    else if (JsonRequest->TryGetStringField(TEXT("endpoints"), SingleEndpoint))
    {
        EndpointNames.Add(SingleEndpoint);
    }

    if (EndpointNames.Num() == 0) return;

    TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
    AsyncTask(ENamedThreads::GameThread, [WeakThis, ws, RequestClientID, Action, EndpointNames]()
    {
        AFicsitRemoteMonitoring* Self = WeakThis.Get();
        if (!Self || Self->bShouldStop) return;   // shutdown/teardown race guard (D-04)

        // Drop stale/not-yet-registered messages rather than treat as an error (Assumption A2): a
        // message for a ws whose ClientID isn't (or is no longer) in ClientGenerations is a valid
        // ignorable state, not a crash — also covers Pitfall 2 (AsyncTask ordering isn't guaranteed;
        // self-validate instead of trusting submission order).
        if (!Self->IsClientCurrent(ws, RequestClientID)) return;

        for (const FString& Endpoint : EndpointNames)
        {
            if (Action == "subscribe")
            {
                if (!Self->EndpointSubscribers.Contains(Endpoint))
                {
                    Self->EndpointSubscribers.Add(Endpoint, TSet<uWS::WebSocket<false, true, FWebSocketUserData>*>());
                }

                if (!Self->bHasRunningPushDataLoop)
                {
                    Self->StartWebSocketPushDataLoop();
                }

                Self->EndpointSubscribers[Endpoint].Add(ws);

                UE_LOG(LogHttpServer, Warning, TEXT("Client subscribed to endpoint: %s"), *Endpoint);
            }
            else if (Action == "unsubscribe" && Self->EndpointSubscribers.Contains(Endpoint))
            {
                Self->EndpointSubscribers[Endpoint].Remove(ws);
                UE_LOG(LogHttpServer, Warning, TEXT("Client unsubscribed from endpoint: %s"), *Endpoint);
            }
        }
    });
}

void AFicsitRemoteMonitoring::PushUpdatedData() {

    // THRD-02 / Pitfall 3: PushUpdatedData runs on the push-pacing thread (Async(EAsyncExecution::Thread,
    // ...) started in StartWebSocketPushDataLoop) — a third thread distinct from both the game thread and
    // the uWS loop thread. Reading EndpointSubscribers here directly would be an unmarshaled read racing
    // against the game thread's writes, and calling Client->send() here would violate uWS's single-
    // thread socket affinity. So: (1) hop to the game thread to build each endpoint's JSON and snapshot
    // its recipients as plain {ws, ClientID} pairs (never hand a live TSet/ws reference across threads),
    // then (2) back on the pacing thread, defer() each send onto the loop thread, re-validating liveness
    // against the loop-thread-only LoopLiveSockets set (D-03) before touching ws.

    struct FPushRecipient
    {
        uWS::WebSocket<false, true, FWebSocketUserData>* Ws;
        int32 ClientID;
    };

    struct FPushSnapshotEntry
    {
        FString Payload;
        TArray<FPushRecipient> Recipients;
    };

    TArray<FPushSnapshotEntry> Snapshot;

    TWeakObjectPtr<AFicsitRemoteMonitoring> WeakThis(this);
    FThreadSafeBool bSnapshotComplete = false;

    AsyncTask(ENamedThreads::GameThread, [WeakThis, &Snapshot, &bSnapshotComplete]()
    {
        AFicsitRemoteMonitoring* Self = WeakThis.Get();
        if (Self && !Self->bShouldStop)
        {
            for (auto& Elem : Self->EndpointSubscribers)
            {
                if (Elem.Value.Num() == 0)
                {
                    continue;
                }

                bool bSuccess = false;
                int32 ErrorCode = 404;

                FRequestData RequestData = FRequestData();
                RequestData.bIsAuthorized = true;

                FString Json;
                Self->HandleEndpoint(Elem.Key, RequestData, bSuccess, ErrorCode, Json, EInterfaceType::Socket);

                FPushSnapshotEntry Entry;
                Entry.Payload = MoveTemp(Json);

                for (uWS::WebSocket<false, true, FWebSocketUserData>* Client : Elem.Value)
                {
                    // Re-validate against the authoritative generation map (D-03) rather than trusting
                    // that everything in EndpointSubscribers is still current.
                    if (const int32* FoundClientID = Self->ClientGenerations.Find(Client))
                    {
                        Entry.Recipients.Add({ Client, *FoundClientID });
                    }
                }

                if (Entry.Recipients.Num() > 0)
                {
                    Snapshot.Add(MoveTemp(Entry));
                }
            }
        }
        bSnapshotComplete = true;
    });

    // The pacing thread's whole job is this periodic snapshot-then-send cycle, so it is expected to wait
    // synchronously for the result (same fire-and-wait shape as CallEndpoint's existing game-thread hop,
    // FicsitRemoteMonitoring.cpp CallEndpoint) — unlike the inbound WS callbacks (D-02), nothing here is
    // blocking the uWS loop thread.
    while (!bSnapshotComplete)
    {
        FPlatformProcess::Sleep(0.0001f);
    }

    if (!CapturedLoop)
    {
        // Loop already torn down (shutdown race) — nothing safe to defer onto.
        return;
    }

    for (const FPushSnapshotEntry& Entry : Snapshot)
    {
        // Owning UTF-8 copy: the deferred callback may run well after this stack frame (and this
        // function's FTCHARToUTF8 buffer) is gone, so a plain char* into a stack-scoped converter must
        // never be captured — copy into an owning std::string instead.
        FTCHARToUTF8 Converted(*Entry.Payload);
        const std::string PayloadUtf8(Converted.Get(), Converted.Length());

        for (const FPushRecipient& Recipient : Entry.Recipients)
        {
            uWS::WebSocket<false, true, FWebSocketUserData>* Ws = Recipient.Ws;
            const int32 ClientID = Recipient.ClientID;

            CapturedLoop->defer([WeakThis, Ws, ClientID, PayloadUtf8]()
            {
                // Now executing on the loop thread. Re-validate ws against the loop-thread-only liveness
                // set before dereferencing it — the client may have disconnected (and its ws* memory
                // possibly reused) between the game-thread snapshot above and this callback running.
                AFicsitRemoteMonitoring* Self = WeakThis.Get();
                if (!Self)
                {
                    return;
                }

                const int32* FoundClientID = Self->LoopLiveSockets.Find(Ws);
                if (FoundClientID && *FoundClientID == ClientID)
                {
                    Ws->send(PayloadUtf8, uWS::OpCode::TEXT);
                }
            });
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
    else if (Extension == "avif") {
    	ContentType = "image/avif";
    	IsBinary = true;
    }
    else if (Extension == "woff2") {
    	ContentType = "font/woff2";
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

FString AFicsitRemoteMonitoring::GenerateAuthToken(const int32 Length)
{
	const FString Characters = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	const int32 CharactersCount = Characters.Len();

	FString RandomString{};
	for (int32 i = 0; i < Length; ++i)
	{
		RandomString.AppendChar(Characters[FMath::RandRange(0, CharactersCount - 1)]);
	}

	return RandomString;
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
	RegisterEndpoint(FAPIEndpoint("GET", "getArtifacts", &UResearch::getArtifacts).RequiresGameThread());
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
	RegisterEndpoint(FAPIEndpoint("GET", "getCrateInv", &UInventory::getCrateInv).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getDoggo", &UPlayerLibrary::getDoggo).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getDrone", &UDrones::getDrone).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getDroneStation", &UDrones::getDroneStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getDropPod", &UResources::getDropPod).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getEncoder", &UFactoryLibrary::getEncoder));
	RegisterEndpoint(FAPIEndpoint("GET", "getExplorationSink", &USession::getExplorationSink));
	RegisterEndpoint(FAPIEndpoint("GET", "getExplorer", &UVehicles::getExplorer).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getExtractor", &UResources::getExtractor));
	RegisterEndpoint(FAPIEndpoint("GET", "getFactoryCart", &UVehicles::getFactoryCart).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getFoundry", &UFactoryLibrary::getFoundry));
	RegisterEndpoint(FAPIEndpoint("GET", "getFrackingActivator", &UResources::getFrackingActivator));
	RegisterEndpoint(FAPIEndpoint("GET", "getFuelGenerator", &UPower::getFuelGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getGeothermalGenerator", &UPower::getGeothermalGenerator));
	RegisterEndpoint(FAPIEndpoint("GET", "getHazards", &UPlayerLibrary::getHazards).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getHUBTerminal", &USupport::getHubTerminal).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getHyperEntrance", &UHypertubes::getHyperEntrance));
	RegisterEndpoint(FAPIEndpoint("GET", "getHypertube", &UHypertubes::getHypertube));
	RegisterEndpoint(FAPIEndpoint("GET", "getHyperJunctions", &UHypertubes::getHyperJunctions));
	RegisterEndpoint(FAPIEndpoint("GET", "getManufacturer", &UFactoryLibrary::getManufacturer));
	RegisterEndpoint(FAPIEndpoint("GET", "getMapMarkers", &USession::getMapMarkers));
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
	RegisterEndpoint(FAPIEndpoint("GET", "getSPWN", &USupport::getSPWN));
	RegisterEndpoint(FAPIEndpoint("GET", "getTractor", &UVehicles::getTractor).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getTradingPost", &USupport::getTradingPost));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrains", &UTrains::getTrains));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrainRails", &UTrains::getTrainRails));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrainStation", &UTrains::getTrainStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getTruck", &UVehicles::getTruck).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getTruckStation", &UVehicles::getTruckStation));
	RegisterEndpoint(FAPIEndpoint("GET", "getWorldInv", &UInventory::getWorldInv));
	RegisterEndpoint(FAPIEndpoint("GET", "getResearchTrees", &UResearch::getResearchTrees).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getChatMessages", &UCommunication::getChatMessages).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getItemPickups", &UResources::getItemPickups).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getUnlockItems", &UResources::getUnlockItems).RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("GET", "getTapes", &UResources::getTapes));
	RegisterEndpoint(FAPIEndpoint("GET", "getUObjectCount", &USession::getUObjectCount).UseFirstObject());
	RegisterEndpoint(FAPIEndpoint("GET", "getBlueprints", &USession::getBlueprints));
	RegisterEndpoint(FAPIEndpoint("GET", "getTrainSignals", &UTrains::getTrainSignals));
	RegisterEndpoint(FAPIEndpoint("GET", "getVehiclePaths", &UVehicles::getVehiclePaths));
	RegisterEndpoint(FAPIEndpoint("GET", "getCreatures", &UPlayerLibrary::getCreatures));

	// event endpoints
	RegisterEndpoint(FAPIEndpoint("GET", "getFallingGiftBundles", &UEventsLibrary::getFallingGiftBundles).RequiresGameThread());

	//FRM API Endpoint Groups
	RegisterEndpoint(FAPIEndpoint("GET", "getFactory", &UFactoryLibrary::getFactory));
	RegisterEndpoint(FAPIEndpoint("GET", "getGenerators", &UPower::getGenerators));
	RegisterEndpoint(FAPIEndpoint("GET", "getVehicles", &UVehicles::getVehicles).RequiresGameThread());

	// post/write endpoints
	RegisterEndpoint(FAPIEndpoint("POST", "setSwitches", &UPower::setSwitches).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "setEnabled", &UCommunication::setEnabled).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "sendChatMessage", &UCommunication::sendChatMessage).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "createPing", &UCommunication::createPing).RequiresAuthentication().RequiresGameThread());
	RegisterEndpoint(FAPIEndpoint("POST", "setModSetting", &UCommunication::setModSetting).RequiresAuthentication().RequiresGameThread());
	

	// API Endpoints Retired - Remove in next update after 1.4.0
	RegisterEndpoint(FAPIEndpoint("GET", "getAll", &AFicsitRemoteMonitoring::getAll));
	
}

void AFicsitRemoteMonitoring::InitOutageNotification() {
	#if (!WITH_EDITOR)
	{
		auto World = GetWorld();
		SUBSCRIBE_UOBJECT_METHOD_AFTER(UFGPowerCircuitGroup, OnFuseSet, [World](UFGPowerCircuitGroup* PowerGroup)
			{
				AFicsitRemoteMonitoring* FicsitRemoteMonitoring = AFicsitRemoteMonitoring::Get(World);
				TArray<FString> FlavorArray = FicsitRemoteMonitoring->Flavor_Power;


				if (UFGPowerCircuit* PowerCircuit = GetValid(PowerGroup->mCircuits[0])) {
					const int32 CircuitID = PowerCircuit->GetCircuitGroupID();
				}
			});
	}
	#endif
}

void AFicsitRemoteMonitoring::InitTrainDerailNotification() {
if (!WITH_EDITOR) {/*

	auto World = GetWorld();

	//	void OnCollided( AFGRailroadVehicle* ourVehicle, float ourVelocity, AFGRailroadVehicle* otherVehicle, float otherVelocity, bool shouldDerail );
	SUBSCRIBE_METHOD_AFTER(AFGRailroadSubsystem::OnTrainsCollided, [this](AFGTrain* PriTrain, AFGTrain* SecTrain)
		{
			auto TrainOne = FString::(PriTrain->GetTrainName());
		});*/
	}
}

// Populates PlayerNameCache on connect (PLYR-03, D-04). First-time integration of a
// SUBSCRIBE_UOBJECT_METHOD_AFTER hook that is actually wired into BeginPlay (unlike the dead
// InitOutageNotification/InitTrainDerailNotification siblings above) — treat as new, unverified
// runtime surface until live-connect-verified (03-02 Task 3).
void AFicsitRemoteMonitoring::InitPlayerConnectCache() {
	#if (!WITH_EDITOR)
	{
		SUBSCRIBE_UOBJECT_METHOD_AFTER(AGameModeBase, PostLogin, [this](AGameModeBase* GameMode, APlayerController* NewPlayer)
			{
				if (!IsValid(NewPlayer)) { return; }

				const AFGPlayerState* PlayerState = Cast<AFGPlayerState>(NewPlayer->PlayerState);
				if (!IsValid(PlayerState)) { return; }

				const FString UserID = PlayerState->GetUserID();
				if (UserID.IsEmpty()) { return; } // defensive: never cache an empty key

				// PostLogin already runs on the game thread — no AsyncTask hop needed here.
				PlayerNameCache.Add(UserID, PlayerState->GetPlayerName());
			});
	}
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

	if (!IsValid(WorldContext) || !IsValid(WorldContext->GetWorld()))
	{
		UE_LOG(LogHttpServer, Warning, TEXT("Blocked API call: World not ready."));
		ErrorCode = 503;
		AddErrorJson(JsonArray, TEXT("Blocked API call: World not ready."));
		Response.JsonValues = JsonArray;
		return Response;
	}

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
            if ((EndpointInfo.bRequireGameThread || IsGarbageCollecting()) && !IsInGameThread()) {
                FThreadSafeBool bAllocationComplete = false;
                AsyncTask(ENamedThreads::GameThread, [&EndpointInfo, WorldContext, RequestData, &JsonArray, &bAllocationComplete, &ErrorCode, &bSuccess]() {
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

	const bool JSONDebugMode = UFRMConfigManager::GetConfigOrDefault<bool>(TEXT("Debug.JSONDebug"), false);
	
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