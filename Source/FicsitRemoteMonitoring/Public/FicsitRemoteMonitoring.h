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
//#include "FactoryDedicatedServer/Public/FGServerSubsystem.h"
//#include "FactoryDedicatedServer/Public/Networking/FGServerAPIManager.h"
#include "FRM_Drones.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_World.h"
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
#include "Configs/Config_DiscITStruct.h"
#include "Patching/NativeHookManager.h"
#include "FGResearchManager.h"
#include "FGRecipeManager.h"
#include "FGSchematicManager.h"
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
#include "FGResearchTreeNode.h"
#include "FRM_RequestData.h"

THIRD_PARTY_INCLUDES_START
#include "ThirdParty/uWebSockets/App.h"
THIRD_PARTY_INCLUDES_END

#include "FicsitRemoteMonitoring.generated.h"

// Declare a critical section
FCriticalSection WebSocketCriticalSection;
TQueue<FString, EQueueMode::Mpsc> MessageQueue;
FThreadSafeBool bIsRunning = true;

struct FWebSocketUserData {
	// Add any fields here you want to track for each WebSocket client
	int32 ClientID;
	FString ClientName;
};

struct FClientInfo
{
	FString SubscribedEndpoints;  // Keep track of all endpoints that have been subscribed
	TArray<uWS::WebSocket<false, true, FWebSocketUserData>*> Client;  // Add the third template argument for USERDATA
};

UDELEGATE(BlueprintCallable) 
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(TArray<UBlueprintJsonValue*>, FAPICallback, const UObject*, WorldContext, FRequestData&, RequestData);

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
    bool bUseFirstObject;

    UPROPERTY(BlueprintReadWrite)
    bool bRequireGameThread;

	UPROPERTY(BlueprintReadWrite)
    FAPICallback Callback;

};

USTRUCT(BlueprintType)
struct FCallEndpointResponse
{
	GENERATED_BODY()

	TArray<UBlueprintJsonValue*> JsonValues;
	bool bUseFirstObject;
};

UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem
{
	GENERATED_BODY()

private:

	TFuture<void> WebServer;

	bool JSONDebugMode;
	
	friend class UFGPowerCircuitGroup;

public:

	AFicsitRemoteMonitoring();
	virtual ~AFicsitRemoteMonitoring();

	friend class UFGServerSubsystem;
	friend class UFGServerAPIManager;

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
    void BlueprintEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, FAPICallback InCallback);

	void RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, UObject* TargetObject, FName FunctionName);
	void RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, bool bUseFirstObject, UObject* TargetObject, FName FunctionName);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString HandleEndpoint (UObject* WorldContext, FString InEndpoint, const FRequestData RequestData, bool& bSuccess);

	//FFGServerErrorResponse HandleCSSEndpoint(FString& out_json, FString InEndpoin);
	
	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FCallEndpointResponse CallEndpoint(UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDropPodInfo_BIE(const AFGDropPod* Droppod, TSubclassOf<UFGItemDescriptor>& ItemClass, int32& Amount, float& Power);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDoggoInfo_BIE(const AActor* Doggo, FString& DisplayName, TArray<FInventoryStack>& Inventory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void SchematicToRecipes_BIE(UObject* WorldContext, TSubclassOf<class UFGSchematic> schematicClass, TArray<TSubclassOf< class UFGRecipe >>& out_RecipeClasses, bool& Purchased, bool& HasUnlocks, bool& LockedAny, bool& LockedTutorial, bool& LockedDependent, bool& LockedPhase, bool& Tutorial);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void RecipeNames_BIE(TSubclassOf<class UFGRecipe> recipeClass, FString& Name, FString& ClassName, FString& CategoryName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void CircuitID_BIE(AFGBuildableFactory* Buildable, int32& CircuitID, float& PowerConsumption);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void PowerCircuit_BIE(AFGBuildableFactory* Buildable, UFGPowerCircuit*& PowerCircuit, float& PowerConsumption);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void PlayerName_BIE(AFGPlayerState* PlayerState, FString& PlayerName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void IconGenerator_BIE();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void ResearchTreeNodeUnlockData_BIE(UFGResearchTreeNode* ResearchTreeNode, TArray<FIntPoint>& Parents, TArray<FIntPoint>& UnhiddenBy, FIntPoint& Coordinates);

	// Array of API endpoints
	UPROPERTY()
	TArray<FAPIEndpoint> APIEndpoints;

	TMap<FString, TSet<uWS::WebSocket<false, true, FWebSocketUserData>*>> EndpointSubscribers;

	TSet<uWS::WebSocket<false, true, FWebSocketUserData>*> ConnectedClients; 

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void InitSerialDevice();

	void HandleApiRequest(UObject* World, uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString Endpoint);

	void InitAPIRegistry();
	void InitOutageNotification();

	void StartWebSocketServer();
	void StopWebSocketServer();

    void OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message);
    void OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode);
	void ProcessClientRequest(uWS::WebSocket<false, true, FWebSocketUserData>* ws, const TSharedPtr<FJsonObject>& JsonRequest);

	void PushUpdatedData();

	void HandleGetRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString FilePath);
	void AddResponseHeaders(uWS::HttpResponse<false>* res, bool bIncludeContentType);

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

	// Timer handle for updating data
	FTimerHandle TimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	// Store the APIName for later use in the function
	FString StoredAPIName;

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getAssembler(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getBelts(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getBelts(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getBiomassGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass_Automated.Build_GeneratorBiomass_Automated_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getBlender(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getCloudInv(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getCloudInv(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getCoalGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getConstructor(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getConverter(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Converter/Build_Converter.Build_Converter_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getDoggo(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Player::getDoggo(WorldContext));
	}
	
	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResearchTrees(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_World::GetResearchTrees(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getDrone(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Drones::getDrone(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getDroneStation(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Drones::getDroneStation(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getDropPod(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getDropPod(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getEncoder(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/QuantumEncoder/Build_QuantumEncoder.Build_QuantumEncoder_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getExplorationSink(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Exploration));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getExplorer(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getExtractor(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getResourceExtractor(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getFactoryCart(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getFoundry(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getFrackingActivator(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFrackingActivator(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getFuelGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorFuel/Build_GeneratorFuel.Build_GeneratorFuel_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getGeothermalGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getHUBTerminal(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getHubTerminal(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getHypertube(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getHypertube(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getManufacturer(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getModList(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getModList(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getNuclearGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPackager(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Packager/Build_Packager.Build_Packager_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getParticle(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/HadronCollider/Build_HadronCollider.Build_HadronCollider_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPaths(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getPipes(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPipes(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getPipes(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPlayer(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Player::getPlayer(WorldContext));
	}

	UFUNCTION()
    TArray<UBlueprintJsonValue*> getPortal(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getPortal(WorldContext, RequestData));
	}

	UFUNCTION()
    TArray<UBlueprintJsonValue*> getPower(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getPower(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPowerSlug(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getPowerSlug(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPowerUsage(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getPowerUsage(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getProdStats(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getProdStats(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getPump(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getPump(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getRadarTower(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getRadarTower(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getRecipes(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getRecipes(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getRefinery(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResourceGeyser(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingCore::StaticClass()));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResourceNode(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingSatellite::StaticClass()));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResourceSink(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Default));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getResourceSinkBuilding(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getResourceWell(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNode::StaticClass()));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSchematics(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getSchematics(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSinkList(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Production::getSinkList(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSmelter(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSessionInfo(UObject* WorldContext, FRequestData RequestData) {
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getSessionInfo(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSpaceElevator(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getSpaceElevator(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getStorageInv(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getStorageInv(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getSwitches(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getSwitches(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getTractor(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Tractor/BP_Tractor.BP_Tractor_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getTrains(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Trains::getTrains(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getTrainStation(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Trains::getTrainStation(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getTruck(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck_C"))));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getTruckStation(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getTruckStation(WorldContext));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getWorldInv(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getWorldInv(WorldContext, RequestData));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getAll(UObject* WorldContext, FRequestData RequestData);

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getFactory(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Factory::getFactory(WorldContext, RequestData, AFGBuildableManufacturer::StaticClass()));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getGenerators(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Power::getGenerators(WorldContext, AFGBuildableGenerator::StaticClass()));
	}

	UFUNCTION()
	TArray<UBlueprintJsonValue*> getVehicles(UObject* WorldContext, FRequestData RequestData) {		
		return UBlueprintJsonValue::FromJsonArray(UFRM_Vehicles::getVehicles(WorldContext, AFGWheeledVehicle::StaticClass()));
	}

};

