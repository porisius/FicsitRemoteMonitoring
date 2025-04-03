#pragma once

#include "Command/ChatCommandInstance.h"
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
	FChatReturn RemoteMonitoringCommand(UObject* WorldContext, class UCommandSender* Sender, TArray<FString> Arguments);

};