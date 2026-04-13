#include "UnrealNetwork.h"
#include "RemoteCallObjects/FicsitRemoteMonitoringRCO.h"

#include "../../Public/FicsitRemoteMonitoring.h"

void UFicsitRemoteMonitoringRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFicsitRemoteMonitoringRCO, Dummy);
}

void UFicsitRemoteMonitoringRCO::Server_StartWebSocketServer_Implementation(UWorld* WorldContext)
{
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext);
	ModSubsystem->StartWebSocketServer(true);
}

void UFicsitRemoteMonitoringRCO::Server_StopWebSocketServer_Implementation(UWorld* WorldContext)
{
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext);
	ModSubsystem->StopWebSocketServer();
}