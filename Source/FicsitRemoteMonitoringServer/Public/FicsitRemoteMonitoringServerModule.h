#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFRMServerAPI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogFRMServerDebug, Log, All);

class FFicsitRemoteMonitoringServerModule : public FDefaultGameModuleImpl {
public:
    virtual void StartupModule() override;

    virtual bool IsGameModule() const override { return true; }

    virtual void ShutdownModule() override;
};