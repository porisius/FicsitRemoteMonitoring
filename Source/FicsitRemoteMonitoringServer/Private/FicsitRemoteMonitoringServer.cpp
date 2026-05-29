#include "FicsitRemoteMonitoringServer.h"

#include "FGServerSubsystem.h"
#include "../../FicsitRemoteMonitoring/Public/Libraries/FRMConfigManager.h"
#include "../../FicsitRemoteMonitoring/Public/FicsitRemoteMonitoring.h"
#include "Engine/GameInstance.h"

class UFGServerSubsystem;
class UFRM_Controller;

void AFicsitRemoteMonitoringServer::BeginPlay()
{
	Super::BeginPlay();

	const auto World = this->GetWorld();
	this->Controller = NewObject<UFRM_Controller>();
	this->Controller->World = World;

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(World);
	this->Controller->ModSubsystem = ModSubsystem;

	const FString AuthToken = UFRMConfigManager::GetConfigOrDefault<FString>(TEXT("uWS.AuthenticationToken"), "");

	this->Controller->AuthToken = AuthToken;
	
	if (World == nullptr)
		return;
	
	const auto GameInstance = World->GetGameInstance();
	if (GameInstance == nullptr)
		return;
	
	const auto Subsystem = GameInstance->GetSubsystem<UFGServerSubsystem>();
	if (Subsystem == nullptr)
		return;
	
	const auto ServerAPIManager = Subsystem->GetServerAPIManager();
	if (ServerAPIManager == nullptr)
		return;

	ServerAPIManager->RegisterRequestHandler(this->Controller);
}
