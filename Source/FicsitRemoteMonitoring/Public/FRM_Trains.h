// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGRailroadSubsystem.h"
#include "Buildables\FGBuildableRailroadStation.h"
#include "Buildables\FGBuildableTrainPlatform.h"
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
	UFUNCTION(BlueprintPure)
	static FString getTrains(UObject* WorldContext);
};
