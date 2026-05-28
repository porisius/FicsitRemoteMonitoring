#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Validation.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API UFRMValidation : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static bool IsTcpPortAvailable(int32 Port, FString& OutReason);
};
