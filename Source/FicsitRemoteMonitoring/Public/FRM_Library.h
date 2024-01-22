#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FGBuildableSubsystem.h"
#include "Buildables\FGBuildableManufacturer.h"
#include "FGPowerInfoComponent.h"
#include "FRM_Library.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Library : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TSharedPtr<FJsonObject> getActorJSON(AActor* Actor);
	static TSharedPtr<FJsonObject> getActorFeaturesJSON(AActor* Actor, FString DisplayName);
};
