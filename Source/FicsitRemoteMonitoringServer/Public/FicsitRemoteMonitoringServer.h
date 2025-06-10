#pragma once

#include "FRM_Controller.h"
#include "ModSubsystem.h"
#include "FicsitRemoteMonitoringServer.generated.h"

UCLASS()
class FICSITREMOTEMONITORINGSERVER_API AFicsitRemoteMonitoringServer : public AModSubsystem
{
	GENERATED_BODY()

public:
	
	UPROPERTY()
	UFRM_Controller* Controller;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};