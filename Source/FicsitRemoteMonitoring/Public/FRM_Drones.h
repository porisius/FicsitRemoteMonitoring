#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableDroneStation.h"
#include "FGCircuitSubsystem.h"
#include "FRM_Library.h"
#include "FGDroneStationInfo.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FRM_Drones.generated.h"

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
};
