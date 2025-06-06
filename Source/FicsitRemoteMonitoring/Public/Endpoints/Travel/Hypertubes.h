#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "FRM_RequestData.h"
#include "Hypertubes.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UHypertubes : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getHyperEntrance(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getHypertube(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getHyperJunctions(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

};
