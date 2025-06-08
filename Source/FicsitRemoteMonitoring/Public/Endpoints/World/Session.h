#pragma once

#include "CoreMinimal.h"
#include "FGResourceSinkSubsystem.h"
#include "RemoteMonitoringLibrary.h"
#include "Session.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API USession : public URemoteMonitoringLibrary
{
	GENERATED_BODY()
	
public:

	static void getSessionInfo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getModList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getUObjectCount(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSinkList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getProdStats(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getBlueprints(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	
	static void getResourceSink(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceSink_Helper(WorldContext, EResourceSinkTrack::RST_Default);
	}

	static void getExplorationSink(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceSink_Helper(WorldContext, EResourceSinkTrack::RST_Exploration);
	}

	//Helper Functions
	static TArray<TSharedPtr<FJsonValue>> getResourceSink_Helper(UObject* WorldContext, EResourceSinkTrack ResourceSinkTrack);
	
};
