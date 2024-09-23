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
#include "uWebSocketsWrapper.h"
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
#include "BlueprintJsonObject.h"

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

// Declare a critical section
FCriticalSection WebSocketCriticalSection;
TQueue<FString, EQueueMode::Mpsc> MessageQueue;
FThreadSafeBool bIsRunning = true;

UDELEGATE(BlueprintCallable) DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UBlueprintJsonObject*, FAPICallback, const UObject*, WorldContext, const UClass*, Class);

USTRUCT(BlueprintType, meta = (DontUseGenericSpawnObject = "True"))
struct FAPIEndpoint
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    FString APIName;

    UPROPERTY(BlueprintReadWrite)
    bool bGetAll;

    UPROPERTY(BlueprintReadWrite)
    bool bRequireGameThread;

    FAPICallback Callback;

    UPROPERTY(BlueprintReadWrite)
    UClass* ClassType;
};

UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem
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

	AFicsitRemoteMonitoring();
	virtual ~AFicsitRemoteMonitoring();

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
    void RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, FAPICallback InCallback, UClass* Class);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString HandleEndpoint (UObject* WorldContext, FString InEndpoin);
	
	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
    UBlueprintJsonObject* CallEndpoint(UObject* WorldContext, FString InEndpoin);

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

	// Array of API endpoints
	UPROPERTY()
	TArray<FAPIEndpoint> APIEndpoints;


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

	static TArray<TSharedPtr<FJsonValue>> getAll(UObject* WorldContext);

    UPROPERTY(EditAnywhere)
    TArray<FAPIEndpoint> Endpoints;

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

