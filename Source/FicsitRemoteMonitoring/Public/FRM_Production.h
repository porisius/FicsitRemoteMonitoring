// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableFactory.h"
#include "Buildables\FGBuildableManufacturer.h"
#include "FGInventoryLibrary.h"
#include "FGInventoryComponent.h"
#include "FGPowerCircuit.h"
#include "Resources/FGItemDescriptor.h"
#include "FGRecipe.h"
#include "FGBlueprintFunctionLibrary.h"
#include "Buildables\FGBuildableResourceExtractor.h"
#include "Buildables\FGBuildableGenerator.h"
#include <Buildables/FGBuildableGeneratorFuel.h>
#include <Buildables/FGBuildableGeneratorNuclear.h>
#include "FGItemPickup.h"
#include "FRM_Library.h"
#include "FRM_Production.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Production : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure)
	static FString getProdStats(UObject* WorldContext);
};