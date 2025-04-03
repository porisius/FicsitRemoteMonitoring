#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGDroneStationInfo.h"
#include "FRM_Drones.generated.h"

class AFGBuildableDroneStation;

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Drones : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TArray<TSharedPtr<FJsonValue>> getDroneStation(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getDrone(UObject* WorldContext);

private:
	static FString getDronePortName(AFGBuildableDroneStation* DroneStation);
};
