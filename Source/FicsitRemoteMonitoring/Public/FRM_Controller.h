#pragma once

#include "FGDSSharedTypes.h"
#include "FactoryDedicatedServer/Public/Controller/FGServerControllerBase.h"

#include "FRM_Controller.generated.h"

struct FFGFileResponseWrapper;
class AFicsitRemoteMonitoring;
class UFGUserSetting;
struct FFGRequestHandlerContextWrapper;
struct FFGServerJoinDataResponse;
class UFGServerSubsystem;

/*
 * Notes for creating new "functions" in the game's API
 *
 * Headers are created as a FGServerRequestHandler as shown below:
 * 
 * 	UFUNCTION( FGServerRequestHandler, FGServerRequestPrivilegeLevel = "NotAuthenticated" )
 *	FFGServerErrorResponse Handler_getPower(FString &OutBlameVoid);
 *
 * This function will be accessible from anyone with firewall access and return data as
 * { "data": { "blameVoid": { <Insert JSON Here> } } }
 *
 *  To add input information: use the following
 *  NOTE: I do not believe this is a restriction on variable types, just using FString as an example
* 	UFUNCTION( FGServerRequestHandler, FGServerRequestPrivilegeLevel = "NotAuthenticated" )
 *	FFGServerErrorResponse Handler_getPower(FString& BlameFeatheredToast, FString &OutBlameVoid);
 *
 *	In Powershell, you can call this function via
 *	iwr -uri https://localhost:7777/api/v1 -Method Post
 *		-Body '{"function": "getPower", "data":{ "blameFeatheredToast":"also derPierre and Fezalion"} }'
 *		-ContentType application/json
 *		-SkipCertificateCheck
 *
 *	In curl:
 *	curl --header "Content-Type: application/json"
 *		--request POST
 *		--data '{"function": "getPower", "data":{ "blameFeatheredToast":"also derPierre and Fezalion"} }'
 *		https://localhost:7777/api/v1 --insecure
 *
 *	FGServerRequestPrivilegeLevel is handled by the game API and not FicsitRemoteMonitoring. We just provide the functions.
 *
 *	From FGDSAuthenticationTypes.h as of the Satisfactory 1.1 Experimental Build Dated 4/14/2025	
 *	Supported privilege levels for the dedicated server issued tokens
 *	UENUM( BlueprintType )
 *	enum class EPrivilegeLevel : uint8
 * 	{
 * 		// Invalid authentication
 * 		None,
 * 		// This player is not authenticated and may not perform any operations on the server
 * 		NotAuthenticated,
 * 		// This player is authenticated as a client and may only join games on the server
 * 		Client,
 * 		// This player is an administrator and may perform any administration tasks, such as starting new games, saving games, loading games, kicking players aso
 * 		Administrator,
 * 		// This player is an initial administrator. The sole purpose of this privilege level is to allow a player to set an admin password and a server name. No other operations are allowed.
 * 		InitialAdmin,
 * 		// This is a permanent API token that is not bound to a particular player and is instead issued to an application. It cannot be used to log into the game, but for the context of privileges it is equal to initial admin.
 * 		APIToken,
 * 	};
*/
enum class EInterfaceType : uint8;

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Controller : public UFGServerControllerBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UWorld *World;

	UPROPERTY()
	AFicsitRemoteMonitoring *ModSubsystem;

	UPROPERTY()
	FString AuthToken;
	
	UFUNCTION( FGServerRequestHandler, FGServerRequestPrivilegeLevel = "NotAuthenticated" )
	FFGServerErrorResponse Handler_Frm(FFGFileResponseWrapper& OutFileResponse, const FFGRequestHandlerContextWrapper& RequestContext);
	
};