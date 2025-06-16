#pragma once

#include "CoreMinimal.h"
#include "FGResourceDeposit.h"
#include "FGResourceNode.h"
#include "FGResourceNodeFrackingCore.h"
#include "FGResourceNodeFrackingSatellite.h"
#include "RemoteMonitoringLibrary.h"
#include "Resources.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UResources : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:
	static void getItemPickups(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getExtractor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getFrackingActivator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getPowerSlug(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getDropPod(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getTapes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getUnlockItems(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	static void getResourceGeyser(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceNode_Helper(WorldContext, AFGResourceNodeFrackingCore::StaticClass());
	}
	
	static void getResourceNode(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceNode_Helper(WorldContext, AFGResourceNode::StaticClass());
	}

	static void getResourceDeposit(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceNode_Helper(WorldContext, AFGResourceDeposit::StaticClass());
	}

	static void getResourceWell(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getResourceNode_Helper(WorldContext, AFGResourceNodeFrackingSatellite::StaticClass());
	}

	//Helper Functions
	static TArray<TSharedPtr<FJsonValue>> getResourceNode_Helper(UObject* WorldContext, UClass* ResourceActor);

protected:
	friend class AFGUnlockPickup;
	
};
