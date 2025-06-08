#pragma once
#include "RemoteMonitoringLibrary.h"
#include "FRM_RequestData.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"

#include "Support.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API USupport : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getHubTerminal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getTradingPost(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSpaceElevator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getElevators(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getPortal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getRadarTower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSigns(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	friend class AFGBuildableWidgetSign;
	
};
