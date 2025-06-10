#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "PlayerLibrary.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UPlayerLibrary : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:
	static void getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getDoggo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
};
