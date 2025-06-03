#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "Hypertubes.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UHypertubes : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static TArray<TSharedPtr<FJsonValue>> getHyperEntrance(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getHypertube(UObject* WorldContext);

};
