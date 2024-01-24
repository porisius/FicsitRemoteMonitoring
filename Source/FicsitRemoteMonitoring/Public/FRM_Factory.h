#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableFactory.h"
#include "Buildables\FGBuildableManufacturer.h"
#include "FGInventoryLibrary.h"
#include "FGInventoryComponent.h"
#include "FGPowerCircuit.h"
#include "Resources/FGItemDescriptor.h"
#include "FGRecipe.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemPickup.h"
#include "Logging\StructuredLog.h"
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
};
