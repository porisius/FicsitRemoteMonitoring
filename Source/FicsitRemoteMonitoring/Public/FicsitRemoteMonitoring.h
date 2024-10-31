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
#include "FGResearchTreeNode.h"
#include "FRM_RequestData.h"

THIRD_PARTY_INCLUDES_START
#include "ThirdParty/uWebSockets/App.h"
THIRD_PARTY_INCLUDES_END

#include "FicsitRemoteMonitoring.generated.h"

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
 
DECLARE_DELEGATE_RetVal_TwoParams(TArray<TSharedPtr<FJsonValue>>, FAPICallback, const UObject* /* WorldContext */, FRequestData /* RequestData */);

USTRUCT(BlueprintType, meta = (DontUseGenericSpawnObject = "True"))
struct FAPIEndpoint
{
    GENERATED_BODY()

public:
	
    FString APIName;
    bool bGetAll;
    bool bUseFirstObject;
    bool bRequireGameThread;
    FAPICallback Callback;

};

USTRUCT(BlueprintType)
struct FCallEndpointResponse
{
	GENERATED_BODY()

	TArray<TSharedPtr<FJsonValue>> JsonValues;
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

	void RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, UObject* TargetObject, FName FunctionName);
	void RegisterEndpoint(const FString& APIName, bool bGetAll, bool bRequireGameThread, bool bUseFirstObject, UObject* TargetObject, FName FunctionName);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString HandleEndpoint (UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess);

	//FFGServerErrorResponse HandleCSSEndpoint(FString& out_json, FString InEndpoin);

	FString JsonArrayToString(bool JSONDebugMode, TArray<TSharedPtr<FJsonValue>> JsonArray);
	FString JsonObjectToString(bool JSONDebugMode, TSharedPtr<FJsonObject> JsonObject);
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

	static TArray<TSharedPtr<FJsonValue>> getAssembler(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getBelts(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getBelts(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getBiomassGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass_Automated.Build_GeneratorBiomass_Automated_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getBlender(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getCloudInv(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getCloudInv(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getCoalGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getConstructor(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getConverter(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Converter/Build_Converter.Build_Converter_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getDoggo(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Player::getDoggo(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getResearchTrees(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_World::GetResearchTrees(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getDrone(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Drones::getDrone(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getDroneStation(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Drones::getDroneStation(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getDropPod(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getDropPod(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getEncoder(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/QuantumEncoder/Build_QuantumEncoder.Build_QuantumEncoder_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getExplorationSink(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Exploration);
	}

	static TArray<TSharedPtr<FJsonValue>> getExplorer(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getExtractor(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getResourceExtractor(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getFactoryCart(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getFoundry(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getFrackingActivator(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getFrackingActivator(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getFuelGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorFuel/Build_GeneratorFuel.Build_GeneratorFuel_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getGeothermalGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getHUBTerminal(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getHubTerminal(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getHypertube(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getHypertube(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getManufacturer(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getModList(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getModList(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getNuclearGenerator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getPackager(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Packager/Build_Packager.Build_Packager_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getParticle(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/HadronCollider/Build_HadronCollider.Build_HadronCollider_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getPaths(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getPipes(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getPipes(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getPipes(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getPlayer(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Player::getPlayer(WorldContext);
	}

    static TArray<TSharedPtr<FJsonValue>> getPortal(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getPortal(WorldContext, RequestData);
	}

    static TArray<TSharedPtr<FJsonValue>> getPower(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getPower(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getPowerSlug(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getPowerSlug(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getPowerUsage(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getPowerUsage(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getProdStats(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getProdStats(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getPump(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getPump(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getRadarTower(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getRadarTower(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getRecipes(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getRecipes(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getRefinery(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getResourceGeyser(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingCore::StaticClass());
	}

	static TArray<TSharedPtr<FJsonValue>> getResourceNode(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingSatellite::StaticClass());
	}

	static TArray<TSharedPtr<FJsonValue>> getResourceSink(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Default);
	}

	static TArray<TSharedPtr<FJsonValue>> getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getResourceSinkBuilding(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getResourceWell(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNode::StaticClass());
	}

	static TArray<TSharedPtr<FJsonValue>> getSchematics(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getSchematics(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getSinkList(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Production::getSinkList(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getSmelter(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getSessionInfo(UObject* WorldContext, FRequestData RequestData) {
		return UFRM_Factory::getSessionInfo(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getSpaceElevator(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getSpaceElevator(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getStorageInv(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getStorageInv(WorldContext, RequestData);
	}

	static TArray<TSharedPtr<FJsonValue>> getSwitches(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getSwitches(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getTractor(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Tractor/BP_Tractor.BP_Tractor_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getTrains(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Trains::getTrains(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getTrainStation(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Trains::getTrainStation(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getTruck(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck_C")));
	}

	static TArray<TSharedPtr<FJsonValue>> getTruckStation(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getTruckStation(WorldContext);
	}

	static TArray<TSharedPtr<FJsonValue>> getWorldInv(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getWorldInv(WorldContext, RequestData);
	}

	TArray<TSharedPtr<FJsonValue>> getAll(UObject* WorldContext, FRequestData RequestData);

	static TArray<TSharedPtr<FJsonValue>> getFactory(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Factory::getFactory(WorldContext, RequestData, AFGBuildableManufacturer::StaticClass());
	}

	static TArray<TSharedPtr<FJsonValue>> getGenerators(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Power::getGenerators(WorldContext, AFGBuildableGenerator::StaticClass());
	}

	static TArray<TSharedPtr<FJsonValue>> getVehicles(UObject* WorldContext, FRequestData RequestData) {		
		return UFRM_Vehicles::getVehicles(WorldContext, AFGWheeledVehicle::StaticClass());
	}

};

