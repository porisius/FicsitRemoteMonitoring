#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FRM_Power.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Power : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TArray<TSharedPtr<FJsonValue>> getPower(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getSwitches(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> setSwitches(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getGenerators(UObject* WorldContext, UClass* TypedBuildable);
	static TArray<TSharedPtr<FJsonValue>> getPowerUsage(UObject* WorldContext);
	
private:
	friend class UFGPowerCircuit;
	friend class UFGPowerCircuitGroup;
	friend class AFGBuildableGenerator;
	friend class AFGBuildableGeneratorFuel;
	friend class AFGBuildableGeneratorNuclear;
	friend class AFGBuildableGeneratorGeoThermal;
	friend class AFGCircuitSubsystem;

};