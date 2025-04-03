#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_Trains.generated.h"

struct FRequestData;

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
	static TArray<TSharedPtr<FJsonValue>> getTrainRails(UObject* WorldContext, FRequestData RequestData);

private:
	friend class AFGBuildableRailroadStation;
	friend class AFGBuildableTrainPlatform;
	friend class AFGBuildableTrainPlatformCargo;
};
