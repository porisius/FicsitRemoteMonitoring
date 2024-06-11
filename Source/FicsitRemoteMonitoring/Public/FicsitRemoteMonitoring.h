#pragma once

#include <thread>
#include <atomic>

#include "CoreMinimal.h"
#include "Engine.h"
#include "EngineUtils.h"
#include "Misc/OutputDevice.h"
#include "Misc/OutputDeviceDebug.h"
#include "HAL/PlatformFileManager.h"
#include "UObject/NoExportTypes.h"
#include "Async/Async.h"
#include "FRM_Drones.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_Power.h"
#include "FRM_Production.h"
#include "FRM_Trains.h"
#include "FRM_Vehicles.h"
#include "Misc/EnumRange.h"
#include "JsonObjectWrapper.h"
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
#include "Misc/OutputDevice.h"
#include "Misc/ScopeLock.h"
#include "HAL/PlatformProcess.h"
#include "Async/Async.h"
#include "Misc/Paths.h"
#include "FicsitRemoteMonitoring.generated.h"

UENUM(BlueprintType)
enum class EAPIEndpoints : uint8
{
	// Read API Endpoints
	getAssembler,           //Done
	getBelts,
	getBiomassGenerator,    //Done
	getBlender,             //Done
	getCoalGenerator,       //Done
	getConstructor,         //Done
	getDoggo,
	getDrone,               //Done
	getDroneStation,        //Done
	getDropPod,             //Done
	getExplorationSink,
	getExplorer,            //Done
	getExtractor,           //Done
	getFactoryCart,         //Done
	getFoundry,             //Done
	getFuelGenerator,       //Done
	getGeothermalGenerator, //Done
	getHUBTerminal,
	getManufacturer,        //Done
	getModList,
	getNuclearGenerator,    //Done
	getPackager,
	getParticle,            //Done
	getPaths,
	getPipes,
	getPlayer,              //Done
	getPower,               //Done
	getPowerSlug,           //Done
	getProdStats,           //Done
	getRadarTower,
	getRecipes,
	getRefinery,            //Done
	getResourceGeyser,
	getResourceNode,
	getResourceSink,
	getResourceWell,
	getSchematics,
	getSinkList,
	getSmelter,             //Done
	getSpaceElevator,
	getStorageInv,          //Done
	getSwitches,
	getTractor,             //Done
	getTrains,              //Done
	getTrainStation,
	getTruck,               //Done
	getTruckStation,        //Done
	getWorldInv,            //Done

	// Read API Group Endpoints
	getAll,
	getFactory,             //Done
	getGenerators,          //Done
	getVehicles,            //Done

	Count UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EAPIEndpoints, EAPIEndpoints::Count);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FAPIModRegistry, FString&, Output, FJsonObjectWrapper&, Json);

// Declare a critical section
FCriticalSection WebSocketCriticalSection;
TQueue<FString, EQueueMode::Mpsc> MessageQueue;
FThreadSafeBool bIsRunning = true;

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

	std::thread WebSocketThread;
	std::atomic<bool> bRunning;

private:
	//friend class UHttpServer;
	friend class UFGPowerCircuitGroup;

	using FHttpServerPtr = TSharedPtr<uWS::App, ESPMode::ThreadSafe>;
	using FThreadPtr = TUniquePtr<std::thread>;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void InitSerialDevice();

	void InitOutageNotification();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void TextToAPI(UPARAM(ref) FString& API, bool& Success, EAPIEndpoints& enumAPI);

	void StartWebSocketServer();
	void StopWebSocketServer();
	void RunWebSocketServer();

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	static FString API_Endpoint(UObject* WorldContext, EAPIEndpoints APICall);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	static FString API_Endpoint_Interface(UObject* WorldContext, FJsonObjectWrapper Json);

	static TArray<TSharedPtr<FJsonValue>> getAll(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> API_Endpoint_Call(UObject* WorldContext, EAPIEndpoints APICall);

	void HandleAPICall(EAPIEndpoints APICall, UObject* WorldContext);

	TArray<TSharedPtr<FJsonValue>> Json;

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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};