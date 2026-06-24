#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Environment.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UEnvironment : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:
	static void getHazards(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSpawners(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSporeFlowers(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
};
