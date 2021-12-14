#include "FicsitRemoteMonitoringModule.h"

#include "FGGameMode.h"
#include "Patching/NativeHookManager.h"

void FFicsitRemoteMonitoringModule::StartupModule()
{
}

IMPLEMENT_GAME_MODULE(FFicsitRemoteMonitoringModule, FicsitRemoteMonitoring);