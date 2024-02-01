#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableFactory.h"
#include "Buildables\FGBuildableManufacturer.h"
#include "Buildables\FGBuildableResourceExtractor.h"
#include "Resources/FGExtractableResourceInterface.h"
#include "Buildables/FGBuildableStorage.h"
#include "FGInventoryLibrary.h"
#include "FGInventoryComponent.h"
#include "FGPowerCircuit.h"
#include "Resources/FGItemDescriptor.h"
#include "FGRecipe.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGDropPod.h"
#include "FGItemPickup.h"
#include "Logging\StructuredLog.h"
#include "FicsitRemoteMonitoring.h"
#include "FRM_Library.h"
#include "FRM_Factory.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Factory : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static FString getFactory(UObject* WorldContext, UClass* TypedBuildable);

	UFUNCTION(BlueprintPure)
	static FString getPowerSlug(UObject* WorldContext);

	UFUNCTION(BlueprintPure)
	static FString getStorageInv(UObject* WorldContext);

	UFUNCTION(BlueprintPure)
	static FString getWorldInv(UObject* WorldContext);

	UFUNCTION(BlueprintPure)
	static FString getDropPod(UObject* WorldContext);

	UFUNCTION(BlueprintPure)
	static FString getResourceExtractor(UObject* WorldContext);

};
