#pragma once

#include "Buildables/FGBuildableConveyorBelt.h"
#include "Buildables/FGBuildableConveyorLift.h"
#include "FGResearchTreeNode.h"
#include "Resources/FGResourceDeposit.h"
#include "Endpoints/Factory/Power.h"
#include "FRM_RequestData.h"
#include "FGRailroadSubsystem.h"
#include "FGTrain.h"
#include "Subsystem/ModSubsystem.h"
#include "RemoteMonitoringLibrary.h"

THIRD_PARTY_INCLUDES_START
#include "ThirdParty/uWebSockets/App.h"
THIRD_PARTY_INCLUDES_END

#include "FicsitRemoteMonitoring.generated.h"

struct FWebSocketUserData {
	// Add any fields here you want to track for each WebSocket client
	int32 ClientID{};
	FString ClientName{};
};

struct FClientInfo
{
	FString SubscribedEndpoints{};  // Keep track of all endpoints that have been subscribed
	TArray<uWS::WebSocket<false, true, FWebSocketUserData>*> Client{};  // Add the third template argument for USERDATA
};

typedef void (*FEndpointFunction)(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

USTRUCT()
struct FAPIEndpoint {
	GENERATED_BODY()  // Required for USTRUCT

	UPROPERTY()
	FString APIName{};

	UPROPERTY()
	FString Method = "GET";

	UPROPERTY()
	bool bUseFirstObject = false;

	UPROPERTY()
	bool bRequireGameThread = false;

	UPROPERTY()
	bool bRequiresAuthentication = false;

	// Function pointer to the endpoint handler (not a UPROPERTY because function pointers aren’t supported by UPROPERTY)
	FEndpointFunction FunctionPtr{};

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

};

USTRUCT(BlueprintType)
struct FArduinoConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	FString RS232_Port;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	int32 BaudRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	int32 SerialStack;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	bool AutoStart;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ficsit Remote Monitoring")
	float SerialDelay;
	
};

USTRUCT(BlueprintType)
struct FCallEndpointResponse
{
	GENERATED_BODY()

	TArray<TSharedPtr<FJsonValue>> JsonValues{};
	bool bUseFirstObject{};
};

UCLASS()
class FICSITREMOTEMONITORING_API AFicsitRemoteMonitoring : public AModSubsystem
{
	GENERATED_BODY()

private:

	TFuture<void> WebServer{};
	
	bool bShouldStop = false;
	bool bHasRunningPushDataLoop = false;

	FString AuthenticationToken{};
	
	friend class UFGPowerCircuitGroup;
	friend class AFGRailroadSubsystem;

public:

	AFicsitRemoteMonitoring();
	virtual ~AFicsitRemoteMonitoring();

	friend class UFGServerSubsystem;
	friend class UFGServerAPIManager;

	/** Get the subsystem in the current world, can be nullptr, e.g. on game ending (destroy) or game startup. */
	static AFicsitRemoteMonitoring* Get(UWorld* world);

	void RegisterEndpoint(const FAPIEndpoint& Endpoint);
	
	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	void HandleEndpoint (FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode, FString& Out_Data, EInterfaceType Interface);
	
	FCallEndpointResponse CallEndpoint(UObject* WorldContext, FString InEndpoint, FRequestData RequestData, bool& bSuccess, int32& ErrorCode);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDropPodInfo_BIE(const AFGDropPod* Droppod, TSubclassOf<UFGItemDescriptor>& ItemClass, int32& Amount, float& Power);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void GetDoggoInfo_BIE(const AActor* Doggo, FString& DisplayName, TArray<FInventoryStack>& Inventory);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void SchematicToRecipes_BIE(UObject* WorldContext, TSubclassOf<class UFGSchematic> schematicClass, TArray<TSubclassOf< class UFGRecipe >>& out_RecipeClasses, bool& Purchased, bool& HasUnlocks, bool& LockedAny, bool& LockedTutorial, bool& LockedDependent, bool& LockedPhase, bool& Tutorial);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void CircuitID_BIE(AFGBuildableFactory* Buildable, int32& CircuitID, float& PowerConsumption);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void PowerCircuit_BIE(AFGBuildableFactory* Buildable, UFGPowerCircuit*& PowerCircuit, float& PowerConsumption);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void IconGenerator_BIE();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void ResearchTreeNodeUnlockData_BIE(UFGResearchTreeNode* ResearchTreeNode, TArray<FIntPoint>& Parents, TArray<FIntPoint>& UnhiddenBy, FIntPoint& Coordinates);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void SporeFlower_BIE(AFGSporeFlower* SporeFlower, FString& State, FString& GasState, bool& bIsSignificant, bool& bIsDead);
	
	// Array of API endpoints
	TArray<FAPIEndpoint> APIEndpoints{};

	TMap<FString, TSet<uWS::WebSocket<false, true, FWebSocketUserData>*>> EndpointSubscribers{};

	TSet<uWS::WebSocket<false, true, FWebSocketUserData>*> ConnectedClients{};

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void InitSerialDevice();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ficsit Remote Monitoring")
	void StopSerialDevice();

	void HandleApiRequest(UObject* World, uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString Endpoint, FRequestData RequestData);

	void InitAPIRegistry();
	void InitOutageNotification();
	void InitTrainDerailNotification();

	void StartWebSocketServer(bool bSkipIfRunning = false);
	void StopWebSocketServer();
	
	UFUNCTION(BlueprintCallable, Category = "Ficsit Remote Monitoring")
	FArduinoConfig GetSerialConfig();

	void OnClientDisconnected(uWS::WebSocket<false, true, FWebSocketUserData>* ws, int code, std::string_view message);
	void OnMessageReceived(uWS::WebSocket<false, true, FWebSocketUserData>* ws, std::string_view message, uWS::OpCode opCode);
	void ProcessClientRequest(uWS::WebSocket<false, true, FWebSocketUserData>* ws, const TSharedPtr<FJsonObject>& JsonRequest);

	void PushUpdatedData();

	void HandleGetRequest(uWS::HttpResponse<false>* res, uWS::HttpRequest* req, FString FilePath);
	bool IsAuthorizedRequest(uWS::HttpRequest* req, FString RequiredToken);
	void AddResponseHeaders(uWS::HttpResponse<false>* res, bool bIncludeContentType);
	void AddErrorJson(TArray<TSharedPtr<FJsonValue>>& JsonArray, const FString& ErrorMessage);
		
	TArray<FString> Flavor_Battery{};
	TArray<FString> Flavor_Doggo{};
	TArray<FString> Flavor_Player{};
	TArray<FString> Flavor_Power{};
	TArray<FString> Flavor_Research{};
	TArray<FString> Flavor_Train{};

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	FString GenerateAuthToken(int32 Length);
	void StartWebSocketPushDataLoop();

public:

	// Store the APIName for later use in the function
	FString StoredAPIName{};
	
	static void getAll(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
	{
		TSharedPtr<FJsonObject> OutJson = MakeShared<FJsonObject>();
		OutJson->SetStringField("error", "Retired endpoint due to large number in the pool causing disruptions.");
		OutJsonArray.Add(MakeShared<FJsonValueObject>(OutJson));
	};
};