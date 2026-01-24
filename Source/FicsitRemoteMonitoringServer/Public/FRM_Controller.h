#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FGServerAPIDataTypes.h"
#include "FRM_Controller.generated.h"

// Forward declarations
struct FFGServerErrorResponse;
struct FFGFileResponseWrapper;
class UFicsitRemoteMonitoring;

UCLASS()
class FICSITREMOTEMONITORINGSERVER_API UFRM_Controller : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    FFGServerErrorResponse Handler_Frm(
        FFGFileResponseWrapper& OutFileResponse,
        FFGRequestHandlerContextWrapper RequestContext,
        FString Endpoint,
        FString Data
    );

    void SetModSubsystem(UFicsitRemoteMonitoring* InModSubsystem);
    void SetAuthToken(const FString& InAuthToken);

private:
    UPROPERTY()
    UFicsitRemoteMonitoring* ModSubsystem;

    FString AuthToken;
};