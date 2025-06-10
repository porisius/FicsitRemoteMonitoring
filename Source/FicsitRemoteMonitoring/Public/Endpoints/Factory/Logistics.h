#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Logistics.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API ULogistics : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getBelts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getBelts_Helper(WorldContext, true);
	}

	static void getLifts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getBelts_Helper(WorldContext, false);
	}

	static void getPipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getPipeJunctions(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getPump(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getThroughputCounter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSplitterMerger(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	//Helper Functions
	
	static TArray<TSharedPtr<FJsonValue>> getBelts_Helper(UObject* WorldContext, bool IsBelt );
};
