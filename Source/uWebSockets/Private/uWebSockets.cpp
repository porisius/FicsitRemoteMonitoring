#include "uWebSockets.h"
#include "Modules/ModuleManager.h"
#include "uWebSocketsWrapper.h"

IMPLEMENT_MODULE(FuWebSocketsModule, uWebSockets)

TUniquePtr<UWebSocketsWrapper> WebSocketsWrapper;

void FuWebSocketsModule::StartupModule()
{
    WebSocketsWrapper = MakeUnique<UWebSocketsWrapper>();
    WebSocketsWrapper->StartServer(9001);
}

void FuWebSocketsModule::ShutdownModule()
{
    if (WebSocketsWrapper.IsValid())
    {
        WebSocketsWrapper->StopServer();
    }
}
