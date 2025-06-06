#pragma once

#include "CoreMinimal.h"
#include "FGDroneStationInfo.h"
#include "RemoteMonitoringLibrary.h"
#include "Drones.generated.h"

class AFGBuildableDroneStation;

UCLASS()
class FICSITREMOTEMONITORING_API UDrones : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:
	static void getDroneStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getDrone(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

// Helper Functions:	
	static FString getDronePortName(AFGBuildableDroneStation* DroneStation);
};
