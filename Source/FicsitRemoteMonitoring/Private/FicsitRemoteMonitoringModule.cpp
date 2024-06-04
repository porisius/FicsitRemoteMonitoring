#include "FicsitRemoteMonitoringModule.h"

#include "FGGameMode.h"
#include "Patching/NativeHookManager.h"

IMPLEMENT_GAME_MODULE(FFicsitRemoteMonitoringModule, FicsitRemoteMonitoring);

DEFINE_LOG_CATEGORY(LogHttpServer);
DEFINE_LOG_CATEGORY(LogWSServer);
DEFINE_LOG_CATEGORY(LogArduino);
DEFINE_LOG_CATEGORY(LogFRMAPI);
DEFINE_LOG_CATEGORY(LogWebSocketServer);

#define LOCTEXT_NAMESPACE "FFicsitRemoteMonitoringModule"

void FFicsitRemoteMonitoringModule::StartupModule()
{

}

void FFicsitRemoteMonitoringModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
