#pragma once

#include "CoreMinimal.h"
#include "ThirdParty/uWebSockets/App.h"
#include "HAL/CriticalSection.h"
#include "Async/Async.h"
#include "TimerManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "FWebSocketServer.generated.h"

// Define user data struct for WebSocket connections
struct FWebSocketUserData
{
    FString ClientID;
};

UCLASS()
class FICSITREMOTEMONITORING_API UWebSocketServer : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(const FString& InModPath, int InPort, const FString& InUIPath, const FString& InIconsPath);

    virtual void BeginDestroy() override;

    void Start();
    void Stop();

private:
    uWS::App* App;  // Change to pointer type to avoid local variable hiding
    FString ModPath;
    FString UIPath;
    FString IconsPath;
    int Port;
    UObject* WorldContext;
    FCriticalSection WebSocketCriticalSection;
    TQueue<FString> MessageQueue;
    bool bIsRunning;
    FThreadSafeBool bRunning;

    struct FClientInfo
    {
        uWS::WebSocket<false, true, FWebSocketUserData>* Client;
        TSet<FString> SubscribedEndpoints;
    };

    TArray<FClientInfo> ConnectedClients;
    FTimerHandle IntervalTimerHandle;
    FTimerManager* TimerManager;

    void ServeStaticFiles();
    void SetupEndpoints();
    void AsyncMessageProcessing();
    void OnClientConnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws);
    void OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message);
    void OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode);
    void SetIntervalFunction();

    void ProcessClientRequest(FClientInfo& ClientInfo, const TSharedPtr<FJsonObject>& JsonRequest);
};
