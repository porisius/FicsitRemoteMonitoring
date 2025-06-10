#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonValue.h"
#include "RemoteMonitoringLibrary.h"
#include "FRM_RequestData.h"
#include "EventsLibrary.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UEventsLibrary : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:
	static void getFallingGiftBundles(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	
};
