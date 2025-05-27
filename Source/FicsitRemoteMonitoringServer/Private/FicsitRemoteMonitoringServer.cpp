#include "FicsitRemoteMonitoringServer.h"

#include "FGServerAPIManager.h"
#include "FGServerSubsystem.h"
#include "../../FicsitRemoteMonitoring/Public/Configs/Config_HTTPStruct.h"
#include "../../FicsitRemoteMonitoring/Public/FicsitRemoteMonitoring.h"

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
	this->Controller->AuthToken = FConfig_HTTPStruct::GetActiveConfig(World).Authentication_Token;
	
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
