#pragma once

#include <thread>
#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Misc/FileHelper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Logging/StructuredLog.h"
#include "Configs/Config_HTTPStruct.h"
#include "Configs/Config_SerialStruct.h"
#include "Configs/Config_WebSocketStruct.h"
#include "Patching/NativeHookManager.h"
#include "FRM_APIEndpointLinker.h"
#include "Subsystems/HttpServer.h"
#include "Resources/FGItemDescriptor.h"
#include "FGDropPod.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Subsystem/ModSubsystem.h"
#include "FicsitRemoteMonitoringModule.h"
#include "FicsitRemoteMonitoring.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem
{
	GENERATED_BODY()

private:
	friend class UHttpServer;

public:

	AFicsitRemoteMonitoring();
	~AFicsitRemoteMonitoring();

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	UFUNCTION(BlueprintImplementableEvent)
	void GetDropPodInfo_BIE(const AFGDropPod* Droppod, TSubclassOf<UFGItemDescriptor>& ItemClass, int32& Amount, float& Power);

	//UFUNCTION(BlueprintImplementableEvent)
	//void OpenSerial(const FString ComPort, int32 BaudRate, int32 StackSize, bool& Success);

	UFUNCTION(BlueprintImplementableEvent)
	void ReadSerial(FString& SerialBytes);

	//UFUNCTION(BlueprintImplementableEvent)
	//void BytesToString(const TArray<uint8>* SerialBytes, FString& APIEndpoint);

	//UFUNCTION(BlueprintImplementableEvent)
	//void WriteSerial(FString* Json);

	void InitHttpService();
	void InitSerialDevice();

	void SerialListen();

protected:
	virtual void BeginPlay() override;
	//virtual void Tick(DeltaTime) override;

};

