#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FRM_Factory.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Factory : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getBelts(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getElevators(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getFactory(UObject* WorldContext, FRequestData RequestData, UClass* TypedBuildable);
	static TArray<TSharedPtr<FJsonValue>> getFrackingActivator(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getHubTerminal(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getPowerSlug(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getStorageInv(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getWorldInv(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getDropPod(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getHypertube(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getPortal(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getPump(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getResourceExtractor(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getPipes(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getModList(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getResourceNode(UObject* WorldContext, FRequestData RequestData, UClass* ResourceActor);
	static TArray<TSharedPtr<FJsonValue>> getRadarTower(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getSpaceElevator(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getCloudInv(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getSessionInfo(UObject* WorldContext, FRequestData RequestData);
	static TArray<TSharedPtr<FJsonValue>> getCables(UObject* WorldContext, FRequestData RequestData);

	friend class AFGBuildableConveyorBase;
	friend class AFGBuildableTradingPost;
	friend class AFGSchematicManager;
	friend class AFGBuildableFactory;
	friend class AFGBuildableRadarTower;
	friend class AFGBuildable;
};
