#pragma once

#include "FGBuildableGenerator.h"
#include "FGBuildableManufacturer.h"
#include "FGResourceDeposit.h"
#include "FGResourceNodeFrackingCore.h"
#include "FGResourceNodeFrackingSatellite.h"
#include "FGResourceSinkSubsystem.h"
#include "FGWheeledVehicle.h"
#include "FRM_Drones.h"
#include "FRM_Events.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_Power.h"
#include "FRM_Production.h"
#include "FRM_RequestData.h"
#include "FRM_Trains.h"
#include "FRM_Vehicles.h"
#include "FRM_World.h"
#include "ModSubsystem.h"
#include "Endpoints/Factory/setEnabled.h"

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

UENUM( BlueprintType )
enum class EFlavorType : uint8
{
	Battery,
	Doggo,
	Player,
	Power,
	Research,
	Train
};

typedef void (AFicsitRemoteMonitoring::*FEndpointFunction)(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

USTRUCT()
struct FAPIEndpoint {
	GENERATED_BODY()  // Required for USTRUCT

	UPROPERTY()
	FString APIName;

	UPROPERTY()
	FString Method = "GET";

	UPROPERTY()
	bool bGetAll = false;

	UPROPERTY()
	bool bUseFirstObject = false;

	UPROPERTY()
	bool bRequireGameThread = false;

	UPROPERTY()
	bool bRequiresAuthentication = false;

	// Function pointer to the endpoint handler (not a UPROPERTY because function pointers aren’t supported by UPROPERTY)
	FEndpointFunction FunctionPtr;

	FAPIEndpoint(const FString& InMethod = "GET", const FString& InAPIName = "", const FEndpointFunction InFunctionPtr = nullptr)
		: APIName(InAPIName),
		Method(InMethod),
		FunctionPtr(InFunctionPtr) {}

	FAPIEndpoint& RequiresAuthentication() {
		bRequiresAuthentication = true;
		return *this;
	}

	FAPIEndpoint& RequiresGameThread() {
		bRequireGameThread = true;
		return *this;
	}

	FAPIEndpoint& UseFirstObject() {
		bUseFirstObject = true;
		return *this;
	}

	FAPIEndpoint& GetAll() {
		bGetAll = true;
		return *this;
	}
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
	
	bool JSONDebugMode = false;
	
	friend class UFGPowerCircuitGroup;

public:

	AFicsitRemoteMonitoring();
	virtual ~AFicsitRemoteMonitoring();

	friend class UFGServerSubsystem;
	friend class UFGServerAPIManager;

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	void RegisterEndpoint(const FAPIEndpoint& Endpoint);
	
	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	//FString HandleEndpoint (UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode);
	void HandleEndpoint (FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode, FString& Out_Data, EInterfaceType Interface);
	
	FCallEndpointResponse CallEndpoint(UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode);

	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FString FlavorTextRandomizer(EFlavorType FlavorType);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDropPodInfo_BIE(const AFGDropPod* Droppod, TSubclassOf<UFGItemDescriptor>& ItemClass, int32& Amount, float& Power);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDoggoInfo_BIE(const AActor* Doggo, FString& DisplayName, TArray<FInventoryStack>& Inventory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void SchematicToRecipes_BIE(UObject* WorldContext, TSubclassOf<class UFGSchematic> schematicClass, TArray<TSubclassOf< class UFGRecipe >>& out_RecipeClasses, bool& Purchased, bool& HasUnlocks, bool& LockedAny, bool& LockedTutorial, bool& LockedDependent, bool& LockedPhase, bool& Tutorial);

	// TODO unused can be removed
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

	void HandleApiRequest(UObject* World, uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString Endpoint, FRequestData RequestData);

	void InitAPIRegistry();
	void InitOutageNotification();

	void StartWebSocketServer();
	void StopWebSocketServer();

	void OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message);
	void OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode);
	void ProcessClientRequest(uWS::WebSocket<false, true, FWebSocketUserData>* ws, const TSharedPtr<FJsonObject>& JsonRequest);

	void PushUpdatedData();

	void HandleGetRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString FilePath);
	bool IsAuthorizedRequest(uWS::HttpRequest* req, FString RequiredToken);
	void AddResponseHeaders(uWS::HttpResponse<false>* res, bool bIncludeContentType);
	void AddErrorJson(TArray<TSharedPtr<FJsonValue>>& JsonArray, const FString& ErrorMessage);

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
	FString GenerateAuthToken(int32 Length);

public:

	// Store the APIName for later use in the function
	FString StoredAPIName;
	
	void getAssembler(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1_C")));
	}
	
	void getBelts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getBelts(WorldContext, RequestData);
	}

	void getElevators(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getElevators(WorldContext, RequestData);
	}
	
	void getBiomassGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass_Automated.Build_GeneratorBiomass_Automated_C")));
	}
	
	void getBlender(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender_C")));
	}

	void getCables(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getCables(WorldContext, RequestData);
	}
	
	void getCloudInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getCloudInv(WorldContext, RequestData);
	}
	
	void getCoalGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal_C")));
	}
		
	void getConstructor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1_C")));
	}
		
	void getConverter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Converter/Build_Converter.Build_Converter_C")));
	}
		
	void getDoggo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Player::getDoggo(WorldContext);
	}
	
	void getResearchTrees(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_World::GetResearchTrees(WorldContext);
	}

	void GetChatMessages(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_World::GetChatMessages(WorldContext);
	}
	
	void SendChatMessage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_World::SendChatMessage(WorldContext, RequestData);
	}
	
	void CreatePing(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_World::CreatePing(WorldContext, RequestData);
	}
	
	void GetItemPickups(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_World::GetItemPickups(WorldContext);
	}

	void getFallingGiftBundles(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Events::GetFallingGiftBundles(WorldContext);
	}
	
	void getDrone(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Drones::getDrone(WorldContext);
	}
	
	void getDroneStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Drones::getDroneStation(WorldContext);
	}
	
	void getDropPod(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getDropPod(WorldContext, RequestData);
	}
	
	void getEncoder(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/QuantumEncoder/Build_QuantumEncoder.Build_QuantumEncoder_C")));
	}
	
	void getExplorationSink(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Exploration);
	}
	
	void getExplorer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer_C")));
	}
	
	void getExtractor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getResourceExtractor(WorldContext, RequestData);
	}
	
	void getFactoryCart(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart_C")));
	}
	
	void getFoundry(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1_C")));
	}
	
	void getFrackingActivator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getFrackingActivator(WorldContext, RequestData);
	}
	
	void getFuelGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorFuel/Build_GeneratorFuel.Build_GeneratorFuel_C")));
	}
	
	void getGeothermalGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal_C")));
	}
	
	void getHUBTerminal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getHubTerminal(WorldContext, RequestData);
	}
	
	void getHypertube(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getHypertube(WorldContext, RequestData);
	}
	
	void getManufacturer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1_C")));
	}
	
	void getModList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getModList(WorldContext, RequestData);
	}
	
	void getNuclearGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear_C")));
	}
	
	void getPackager(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Packager/Build_Packager.Build_Packager_C")));
	}
	
	void getParticle(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/HadronCollider/Build_HadronCollider.Build_HadronCollider_C")));
	}
	
	void getPipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getPipes(WorldContext, RequestData);
	}
	
	void getPlayer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Player::getPlayer(WorldContext);
	}

	
	void getPortal(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getPortal(WorldContext, RequestData);
	}
	
	void getPower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getPower(WorldContext);
	}

	
	void getPowerSlug(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getPowerSlug(WorldContext, RequestData);
	}

	void getArtifacts(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_World::GetArtifacts(WorldContext);
	}
	
	void getPowerUsage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getPowerUsage(WorldContext);
	}
	
	void getProdStats(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getProdStats(WorldContext);
	}
	
	void getPump(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getPump(WorldContext, RequestData);
	}

	void getRadarTower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getRadarTower(WorldContext, RequestData);
	}
		
	void getRecipes(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getRecipes(WorldContext);
	}
	
	void getRefinery(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery_C")));
	}
	
	void getResourceGeyser(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingCore::StaticClass());
	}
	
	void getResourceNode(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNode::StaticClass());
	}

	void getResourceDeposit(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceDeposit::StaticClass());
	}
	
	void getResourceSink(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getResourceSink(WorldContext, EResourceSinkTrack::RST_Default);
	}
	
	void getResourceSinkBuilding(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getResourceSinkBuilding(WorldContext, RequestData);
	}
	
	void getResourceWell(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getResourceNode(WorldContext, RequestData, AFGResourceNodeFrackingSatellite::StaticClass());
	}
	
	void getSchematics(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getSchematics(WorldContext);
	}
	
	void getSinkList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Production::getSinkList(WorldContext);
	}
	
	void getSmelter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1_C")));
	}
	
	void getSessionInfo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
		OutJsonArray = UFRM_Factory::getSessionInfo(WorldContext, RequestData);
	}
	
	void getSpaceElevator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getSpaceElevator(WorldContext, RequestData);
	}
	
	void getStorageInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getStorageInv(WorldContext, RequestData);
	}
	
	void getSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getSwitches(WorldContext);
	}
	
	void setSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::setSwitches(WorldContext, RequestData);
	}

	void setEnabled(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = USetEnabled::setEnabled(WorldContext, RequestData);
	}
	
	void getTractor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Tractor/BP_Tractor.BP_Tractor_C")));
	}
	
	void getTrains(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Trains::getTrains(WorldContext);
	}

	void getTrainRails(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Trains::getTrainRails(WorldContext, RequestData);
	}
	
	void getTrainStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Trains::getTrainStation(WorldContext);
	}
	
	void getTruck(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Vehicle/Truck/BP_Truck.BP_Truck_C")));
	}
	
	void getTruckStation(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getTruckStation(WorldContext);
	}
	
	void getUObjectCount(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_World::GetUObjectCount();
	}

	void getVehiclePaths(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Vehicles::getVehiclePaths(WorldContext);
	}

	void getWorldInv(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getWorldInv(WorldContext, RequestData);
	}

	void getAll(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	void getFactory(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Factory::getFactory(WorldContext, RequestData, AFGBuildableManufacturer::StaticClass());
	}

	void getGenerators(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = UFRM_Power::getGenerators(WorldContext, AFGBuildableGenerator::StaticClass());
	}

	void getVehicles(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		OutJsonArray = UFRM_Vehicles::getVehicles(WorldContext, AFGWheeledVehicle::StaticClass());
	}
};