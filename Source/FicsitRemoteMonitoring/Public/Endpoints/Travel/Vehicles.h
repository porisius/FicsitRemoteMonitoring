#pragma once

#include "CoreMinimal.h"
#include "FGWheeledVehicle.h"
#include "RemoteMonitoringLibrary.h"
#include "Vehicles.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UVehicles : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getExplorer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getVehicles_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer_C")));
	}
		
	static void getFactoryCart(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getVehicles_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart_C")));
	}
	
	static void getTractor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getVehicles_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Tractor/BP_Tractor.BP_Tractor_C")));
	}

	static void getTruck(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getVehicles_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck_C")));
	}
	
	static void getVehicles(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = getVehicles_Helper(WorldContext, AFGWheeledVehicle::StaticClass());
	}
	
	static void getTruckStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getVehiclePaths(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	
//Helper Functions
	
	static TArray<TSharedPtr<FJsonValue>> getVehicles_Helper(UObject* WorldContext, UClass* VehicleClass);
	static FString GetPathNameForTargetList(AFGDrivingTargetList* TargetList);

protected: 
	friend class AFGVehicleSubsystem;
	friend class AFGBuildableDockingStation;
	friend class AFGSavedWheeledVehiclePath;
};
