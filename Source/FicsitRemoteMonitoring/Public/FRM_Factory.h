#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FGCentralStorageSubsystem.h"
#include "FGBuildableSubsystem.h"
#include "FGBlueprintFunctionLibrary.h"
#include "Buildables/FGBuildableConveyorBase.h"
#include "Buildables/FGBuildablePipeline.h"
#include "FGPipeConnectionComponent.h"
#include "FGFactoryConnectionComponent.h"
#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableHubTerminal.h"
#include "Buildables/FGBuildableTradingPost.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableManufacturerVariablePower.h"
#include "Buildables/FGBuildableResourceExtractor.h"
#include "Buildables/FGBuildableResourceSink.h"
#include "Buildables/FGBuildableSpaceElevator.h"
#include "Buildables/FGBuildableRadarTower.h"
#include "FGScannableSubsystem.h"
#include "Resources/FGExtractableResourceInterface.h"
#include "Resources/FGResourceNode.h"
#include "Resources/FGResourceNodeBase.h"
#include "Resources/FGResourceNodeFrackingCore.h"
#include "Resources/FGResourceNodeFrackingSatellite.h"
#include "FGGamePhaseManager.h"
#include "Buildables/FGBuildableStorage.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "FGSchematicManager.h"
#include "FGInventoryLibrary.h"
#include "FGInventoryComponent.h"
#include "FGPowerCircuit.h"
#include "Resources/FGItemDescriptor.h"
#include "FGRecipe.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGDropPod.h"
#include "FGItemPickup.h"
#include "Logging\StructuredLog.h"
#include "FRM_Library.h"
#include "FRM_Factory.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Factory : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getBelts(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getFactory(UObject* WorldContext, UClass* TypedBuildable);
	static TArray<TSharedPtr<FJsonValue>> getHubTerminal(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getPowerSlug(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getStorageInv(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getWorldInv(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getDropPod(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getResourceExtractor(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getResourceSinkBuilding(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getPipes(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getModList(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getResourceNode(UObject* WorldContext, UClass* ResourceActor);
	static TArray<TSharedPtr<FJsonValue>> getRadarTower(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getSpaceElevator(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getCloudInv(UObject* WorldContext);

	friend class AFGBuildableConveyorBase;
	friend class AFGBuildableTradingPost;
	friend class AFGSchematicManager;
	friend class AFGBuildableFactory;
	friend class AFGBuildableRadarTower;
};
