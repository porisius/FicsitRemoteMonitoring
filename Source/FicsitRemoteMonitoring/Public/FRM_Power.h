#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FGBlueprintFunctionLibrary.h"
#include <FRM_Library.h>
#include <FGInventoryLibrary.h>
#include "FGCircuitConnectionComponent.h"
#include <Buildables/FGBuildableCircuitSwitch.h>
#include "Buildables\FGBuildableGeneratorGeoThermal.h"
#include "Buildables\FGBuildableGeneratorNuclear.h"
#include "FGPowerCircuit.h"
#include "FGCircuitSubsystem.h"
#include "BlueprintJsonObject.h"
#include "FRM_Power.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Power : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TArray<TSharedPtr<FJsonValue>> getPower(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getSwitches(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getGenerators(UObject* WorldContext, UClass* TypedBuildable);

private:
	friend class UFGPowerCircuit;
	friend class UFGPowerCircuitGroup;
	friend class AFGBuildableGenerator;
	friend class AFGBuildableGeneratorFuel;
	friend class AFGBuildableGeneratorNuclear;
	friend class AFGBuildableGeneratorGeoThermal;
	friend class AFGCircuitSubsystem;

};