#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectConverter.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGPowerCircuit.h"
#include "FGCircuitSubsystem.h"
#include "Power.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRMPowerLibrary : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
    public:

		UFUNCTION(BlueprintPure)
		static FString getCircuit(UObject* WorldContext);

private:
	friend class UFGPowerCircuit;
	friend class UFGPowerCircuitGroup;
	friend class AFGCircuitSubsystem;

};