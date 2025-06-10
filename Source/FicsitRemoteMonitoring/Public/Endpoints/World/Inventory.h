#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Inventory.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UInventory : public URemoteMonitoringLibrary
{
	GENERATED_BODY()
	
public:
	
	static void getCloudInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getWorldInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getStorageInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
};
