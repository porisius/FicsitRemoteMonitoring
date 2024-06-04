#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

//class FWebSocketsModule;

class FuWebSocketsModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void StartServer();
    void StopServer();

    TUniquePtr<FuWebSocketsModule> WebSocketsServer;
};
