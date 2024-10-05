#include "FWebSocketServer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "FicsitRemoteMonitoring.h"

void UWebSocketServer::Initialize(const FString& InModPath, int InPort, const FString& InUIPath, const FString& InIconsPath)
{
    ModPath = InModPath;
    Port = InPort;
    UIPath = InUIPath;
    IconsPath = InIconsPath;

    // Add to root to avoid garbage collection
    AddToRoot();
}

void UWebSocketServer::BeginDestroy()
{
    Stop();
    Super::BeginDestroy();  // Call the parent class's BeginDestroy to ensure proper cleanup
}

void UWebSocketServer::Start()
{
    bRunning = true;

    // Use class member 'App' directly
    App = new uWS::App();

    ServeStaticFiles();
    SetupEndpoints();

    // WebSocket behavior
    uWS::App::WebSocketBehavior<FWebSocketUserData> wsBehavior;
    wsBehavior.compression = uWS::SHARED_COMPRESSOR;

    wsBehavior.open = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws) {
        OnClientConnected(ws);
    };

    wsBehavior.close = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
        OnClientDisconnected(ws, code, message);
    };

    wsBehavior.message = [this](uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
        OnMessageReceived(ws, message, opCode);
    };

    App->ws<FWebSocketUserData>("/*", std::move(wsBehavior));

    App->listen(Port, [this](auto* token) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        if (token) {
            UE_LOG(LogTemp, Warning, TEXT("WebSocket server is listening on port %d"), Port);
        } else {
            UE_LOG(LogTemp, Error, TEXT("Failed to listen on port %d"), Port);
        }
    }).run();

    // Use Unreal's TimerManager for interval functions
    if (WorldContext)
    {
        WorldContext->GetWorld()->GetTimerManager().SetTimer(IntervalTimerHandle, this, &UWebSocketServer::SetIntervalFunction, 5.0f, true);
    }

    // Start asynchronous message processing using Unreal's threading system
    Async(EAsyncExecution::Thread, [this]() {
        AsyncMessageProcessing();
    });
}

void UWebSocketServer::ServeStaticFiles()
{
    App->get("/icons/*", [this](auto* res, auto* req) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        std::string url(req->getUrl().begin(), req->getUrl().end());
        FString RelativePath = FString(url.c_str()).Replace(TEXT("/icons/"), TEXT(""));
        FString FilePath = FPaths::Combine(IconsPath, RelativePath);

        FString FileContent;
        if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(FileContent, *FilePath)) {
            res->writeHeader("Access-Control-Allow-Origin", "*")->end(TCHAR_TO_UTF8(*FileContent));
        } else {
            res->writeStatus("404 Not Found")->end("File not found");
        }
    });

    App->get("/*", [this](auto* res, auto* req) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        std::string url(req->getUrl().begin(), req->getUrl().end());
        FString RelativePath = FString(url.c_str()).Replace(TEXT("/"), TEXT(""));
        FString FilePath = FPaths::Combine(UIPath, RelativePath);

        FString FileContent;
        if (FPaths::FileExists(FilePath) && FFileHelper::LoadFileToString(FileContent, *FilePath)) {
            res->writeHeader("Access-Control-Allow-Origin", "*")->end(TCHAR_TO_UTF8(*FileContent));
        } else {
            res->writeStatus("404 Not Found")->end("File not found");
        }
    });
}

void UWebSocketServer::SetupEndpoints()
{
    App->get("/getCoffee", [this](auto* res, auto* req) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        FString noCoffee = TEXT("Error: I'm a teapot. #PraiseAlpaca #BlameSimon");

        res->writeHeader("Access-Control-Allow-Origin", "*")
            ->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
            ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
            ->writeStatus("418 I'm a teapot")
            ->end(TCHAR_TO_UTF8(*noCoffee));
    });

    App->get("/get*", [this](auto* res, auto* req) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        FString noCoffee = TEXT("Error: I'm a teapot. #PraiseAlpaca #BlameSimon");

        res->writeHeader("Access-Control-Allow-Origin", "*")
            ->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
            ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
            ->writeStatus("418 I'm a teapot")
            ->end(TCHAR_TO_UTF8(*noCoffee));
    });

    App->get("/api/*", [this](auto* res, auto* req) {
        FScopeLock ScopeLock(&WebSocketCriticalSection);
        FString noCoffee = TEXT("Error: I'm a teapot. #PraiseAlpaca #BlameSimon");

        res->writeHeader("Access-Control-Allow-Origin", "*")
            ->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
            ->writeHeader("Access-Control-Allow-Headers", "Content-Type")
            ->writeStatus("418 I'm a teapot")
            ->end(TCHAR_TO_UTF8(*noCoffee));
    });
}

void UWebSocketServer::OnClientConnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws) {
    FScopeLock ScopeLock(&WebSocketCriticalSection);
    ConnectedClients.Add({ ws, {} });
    UE_LOG(LogTemp, Warning, TEXT("Client connected. Total clients: %d"), ConnectedClients.Num());
}

void UWebSocketServer::OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message) {
    FScopeLock ScopeLock(&WebSocketCriticalSection);
    ConnectedClients.RemoveAll([ws](const FClientInfo& ClientInfo) { return ClientInfo.Client == ws; });
    UE_LOG(LogTemp, Warning, TEXT("Client disconnected. Total clients: %d"), ConnectedClients.Num());
}

void UWebSocketServer::OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode) {
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

void UWebSocketServer::ProcessClientRequest(FClientInfo& ClientInfo, const TSharedPtr<FJsonObject>& JsonRequest)
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

void UWebSocketServer::AsyncMessageProcessing()
{
    while (this->bRunning)
    {
        FString Message;
        while (MessageQueue.Dequeue(Message))
        {
            // Process message
            UE_LOG(LogTemp, Warning, TEXT("Processing message: %s"), *Message);
        }
        FPlatformProcess::Sleep(0.01f);
    }
}

void UWebSocketServer::Stop()
{
    bRunning = false;
    App->close();
    if (WorldContext)
    {
        WorldContext->GetWorld()->GetTimerManager().ClearTimer(IntervalTimerHandle);
    }
}

void UWebSocketServer::SetIntervalFunction()
{
    FScopeLock ScopeLock(&WebSocketCriticalSection);
    AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());

    for (FClientInfo& ClientInfo : ConnectedClients)
    {
        for (const FString& Endpoint : ClientInfo.SubscribedEndpoints)
        {
            FString APIData = ModSubsystem->HandleEndpoint(WorldContext, Endpoint);
            ClientInfo.Client->send(TCHAR_TO_UTF8(*APIData), uWS::OpCode::TEXT);
        }
    }
}
