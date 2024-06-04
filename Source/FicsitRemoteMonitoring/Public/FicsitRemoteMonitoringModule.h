#pragma once

//#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHttpServer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogWSServer, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogArduino, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFRMAPI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFRMNotification, Log, All);

DECLARE_LOG_CATEGORY_EXTERN(LogWebSocketServer, Log, All);

class FFicsitRemoteMonitoringModule : public FDefaultGameModuleImpl {
public:
    virtual void StartupModule() override;

    virtual bool IsGameModule() const override { return true; }

    virtual void ShutdownModule() override;
};