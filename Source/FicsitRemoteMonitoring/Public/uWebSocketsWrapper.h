#pragma once

#include "CoreMinimal.h"

THIRD_PARTY_INCLUDES_START
#include "ThirdParty/uWebSockets/App.h"
THIRD_PARTY_INCLUDES_END

#include <memory>

class UWebSocketsWrapper : public UObject
{
public:
    UWebSocketsWrapper();
    ~UWebSocketsWrapper();

    void StartServer(int32 Port);
    void StopServer();

    static UWebSocketsWrapper* CreateHttpServer();

    // Handlers
    void OnHttpRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

private:

    using FHttpServerPtr = TSharedPtr<uWS::App, ESPMode::ThreadSafe>;

    UPROPERTY()
    UWebSocketsWrapper* WebsocketServer;

    UPROPERTY()
    std::unique_ptr<uWS::App> WebSocketApp;
};
