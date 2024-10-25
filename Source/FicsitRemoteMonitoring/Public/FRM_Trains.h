#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGBuildableTrainPlatformEmpty.h"
#include "FGTrainPlatformConnection.h"
#include "FGRailroadSubsystem.h"
#include "Buildables\FGBuildableRailroadStation.h"
#include "Buildables\FGBuildableTrainPlatform.h"
#include "Buildables\FGBuildableTrainPlatformCargo.h"
#include "FGRailroadTimeTable.h"
#include "FGPowerCircuit.h"
#include "FGCircuitSubsystem.h"
#include "FRM_Library.h"
#include "FGLocomotive.h"
#include "FGTrain.h"
#include "FGTrainStationIdentifier.h"
#include "FGLocomotiveMovementComponent.h"
#include "FRM_Trains.generated.h"
 
/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Trains : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TArray<TSharedPtr<FJsonValue>> getTrains(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getTrainStation(UObject* WorldContext);

private:
	friend class AFGBuildableRailroadStation;
	friend class AFGBuildableTrainPlatform;
	friend class AFGBuildableTrainPlatformCargo;
};
