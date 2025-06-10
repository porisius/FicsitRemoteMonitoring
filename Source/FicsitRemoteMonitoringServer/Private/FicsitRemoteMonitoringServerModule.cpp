#include "FicsitRemoteMonitoringServerModule.h"

#include "FGGameMode.h"
#include "Patching/NativeHookManager.h"

IMPLEMENT_GAME_MODULE(FFicsitRemoteMonitoringServerModule, FicsitRemoteMonitoring);

DEFINE_LOG_CATEGORY(LogFRMServerAPI);
DEFINE_LOG_CATEGORY(LogFRMServerDebug);

#define LOCTEXT_NAMESPACE "FFicsitRemoteMonitoringModule"

void FFicsitRemoteMonitoringServerModule::StartupModule()
{

}

void FFicsitRemoteMonitoringServerModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
