// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGVehicleSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableDockingStation.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FRM_Library.h"
#include "FRM_Vehicles.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Vehicles : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static FString getTruckStation(UObject* WorldContext);
	
};
