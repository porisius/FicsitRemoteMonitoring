#pragma once

#include "CoreMinimal.h"
#include "App.h" // Include the uWebSockets header
#include <memory>

class UWebSocketsWrapper
{
public:
    UWebSocketsWrapper();
    ~UWebSocketsWrapper();

    void StartServer(int32 Port);
    void StopServer();

    // Handlers
    void OnHttpRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

private:
    std::unique_ptr<uWS::App> WebSocketApp;
};
