#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_Vehicles.generated.h"

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Vehicles : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getVehicles(UObject* WorldContext, UClass* VehicleClass);
	static TArray<TSharedPtr<FJsonValue>> getTruckStation(UObject* WorldContext);

protected: 
	friend class AFGVehicleSubsystem;
	friend class AFGBuildableDockingStation;
	friend class AFGSavedWheeledVehiclePath;
};
