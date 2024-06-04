#pragma once

#include "FicsitRemoteMonitoring.h"
#include "FRM_APIEndpointLinker.h"
#include "Command/ChatCommandLibrary.h"
#include "Command/ChatCommandInstance.h"
#include "Command/CommandSender.h"
#include "Containers/UnrealString.h"
#include "NotificationLoader.h"
#include "Configs/Config_HTTPStruct.h"
#include "Configs/Config_SerialStruct.h"
#include "Configs/Config_WebSocketStruct.h"
#include "Configs/Config_DiscITStruct.h"
#include "multi.generated.h"


USTRUCT(BlueprintType, Category = "Ficsit Remote Monitoring")
struct FChatReturn
{
	GENERATED_BODY()
public:
	
	UPROPERTY(BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	FString Chat;

	UPROPERTY(BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	FLinearColor Color;

	UPROPERTY(BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	EExecutionStatus Status;
};

UCLASS()
class FICSITREMOTEMONITORING_API AFRMCommand : public AChatCommandInstance {

	GENERATED_BODY()

public: 

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FChatReturn RemoteMonitoringCommand(UObject* WorldContext, class UCommandSender* Sender, const TArray<FString> Arguments);

};