#pragma once

#include "CoreMinimal.h"
#include "RemoteMonitoringLibrary.h"
#include "Communication.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UCommunication : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getChatMessages(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void sendChatMessage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void createPing(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void setEnabled(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void setModSetting(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
};
