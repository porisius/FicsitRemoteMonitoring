#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringRCO.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFicsitRemoteMonitoringRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()
public:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	bool Dummy;

	UFUNCTION(BlueprintCallable, Reliable, Server)
	void Server_StartWebSocketServer(UWorld* WorldContext);
	
	UFUNCTION(BlueprintCallable, Reliable, Server)
	void Server_StopWebSocketServer(UWorld* WorldContext);
};
