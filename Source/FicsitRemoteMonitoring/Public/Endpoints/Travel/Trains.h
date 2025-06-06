#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Trains.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UTrains : public URemoteMonitoringLibrary
{
	GENERATED_BODY()
	
public:
	static void getTrains(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getTrainStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getTrainRails(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

private:
	friend class AFGBuildableRailroadStation;
	friend class AFGBuildableTrainPlatform;
	friend class AFGBuildableTrainPlatformCargo;
};
