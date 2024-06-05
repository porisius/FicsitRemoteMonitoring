#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "FicsitRemoteMonitoringModule.h"
#include "Json.h"
#include "Misc/FileHelper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "ModLoading/ModLoadingLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Logging/StructuredLog.h"
#include "Configs/Config_HTTPStruct.h"
#include "Configs/Config_SerialStruct.h"
#include "Configs/Config_WebSocketStruct.h"
#include "Configs/Config_DiscITStruct.h"
#include "Patching/NativeHookManager.h"
#include "FGResearchManager.h"
#include "FGRecipeManager.h"
#include "FGSchematicManager.h"
#include "FRM_APIEndpointLinker.h"
#include "uWebSocketsWrapper.h"
//#include "Subsystems/HttpServer.h"
#include "Resources/FGItemDescriptor.h"
#include "FGDropPod.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Subsystem/ModSubsystem.h"
#include "NotificationLoader.h"
#include "FicsitRemoteMonitoring.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FAPIModRegistry, FString&, Output, FJsonObjectWrapper&, Json);

USTRUCT(BlueprintType)
struct FAPIRegistry
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool getAllGroup;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FAPIModRegistry Callback;
};

/*class UAPIRegisterInterface : public UInterface
{
	GENERATED_BODY()
};

UINTERFACE(MinimalAPI, BlueprintType)
class FICSITREMOTEMONITORING_API IAPIRegistry
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void RegisterAPIEndpoint(FString Name, FAPIRegistry Callback);
};*/

UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem //, public IAPIRegistry
{
	GENERATED_BODY()

private:
	//friend class UHttpServer;
	friend class UFGPowerCircuitGroup;

public:

	//virtual void RegisterAPIEndpoint_Implementation(FString Name, FAPIRegistry Callback) override;

	AFicsitRemoteMonitoring();
	virtual ~AFicsitRemoteMonitoring();

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDropPodInfo_BIE(const AFGDropPod* Droppod, TSubclassOf<UFGItemDescriptor>& ItemClass, int32& Amount, float& Power);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDoggoInfo_BIE(const AActor* Doggo, FString& DisplayName, TArray<FInventoryStack>& Inventory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void SchematicToRecipes_BIE(UObject* WorldContext, TSubclassOf<class UFGSchematic> schematicClass, TArray<TSubclassOf< class UFGRecipe >>& out_RecipeClasses, bool& Purchased, bool& HasUnlocks, bool& LockedAny, bool& LockedTutorial, bool& LockedDependent, bool& LockedPhase, bool& Tutorial);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void RecipeNames_BIE(TSubclassOf<class UFGRecipe> recipeClass, FString& Name, FString& ClassName, FString& CategoryName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void CircuitID_BIE(UFGPowerInfoComponent* PowerInfo, TSubclassOf<class UFGBuildingDescriptor> Buildable, int32& CircuitID, float& PowerConsumption, float& MinPower, float& MaxPower, bool& VariblePower);

	//UFUNCTION(BlueprintImplementableEvent)
	//void OpenSerial(const FString ComPort, int32 BaudRate, int32 StackSize, bool& Success);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void ReadSerial(FString& SerialBytes);

	//void InitHttpService();

	void InitWSService();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void InitSerialDevice();

	void InitOutageNotification();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void TextToAPI(UPARAM(ref) FString& API, bool& Success, EAPIEndpoints& enumAPI);

	//UPROPERTY(EditAnywhere)
	//UHttpServer* HttpServer;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FAPIRegistry> APIRegister;

	UPROPERTY()
	TArray<FString> Flavor_Battery;

	UPROPERTY()
	TArray<FString> Flavor_Doggo;

	UPROPERTY()
	TArray<FString> Flavor_Player;

	UPROPERTY()
	TArray<FString> Flavor_Power;

	UPROPERTY()
	TArray<FString> Flavor_Research;

	UPROPERTY()
	TArray<FString> Flavor_Train;

protected:
	virtual void BeginPlay() override;
	//virtual void Tick(DeltaTime) override;

};