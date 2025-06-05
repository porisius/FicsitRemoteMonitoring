#pragma once

#include "FGDSSharedTypes.h"
#include "FactoryDedicatedServer/Public/Controller/FGServerControllerBase.h"

#include "FRM_Controller.generated.h"

class UWorld;
struct FFGFileResponseWrapper;
class AFicsitRemoteMonitoring;
class UFGUserSetting;
struct FFGRequestHandlerContextWrapper;
struct FFGServerJoinDataResponse;
class UFGServerSubsystem;

enum class EInterfaceType : uint8;

UCLASS()
class FICSITREMOTEMONITORINGSERVER_API UFRM_Controller : public UFGServerControllerBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UWorld* World;
	
	UPROPERTY()
	AFicsitRemoteMonitoring* ModSubsystem;

	UPROPERTY()
	FString AuthToken;
	
	UFUNCTION( FGServerRequestHandler, FGServerRequestPrivilegeLevel = "NotAuthenticated" )
	FFGServerErrorResponse Handler_Frm(FFGFileResponseWrapper& OutFileResponse, const FFGRequestHandlerContextWrapper& RequestContext);
	
};