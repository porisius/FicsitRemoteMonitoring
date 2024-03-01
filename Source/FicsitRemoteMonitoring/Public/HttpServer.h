// Http Server Example created by Aleksey Usanov
// Modified by Chris Wall

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystem/ModSubsystem.h"
#include "HttpServerRequest.h"
#include "HttpResultCallback.h"
#include "FRM_Drones.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_Power.h"
#include "FRM_Production.h"
#include "FRM_Trains.h"
#include "FRM_Vehicles.h"
#include "FGBuildableSubsystem.h"
#include "HttpServer.generated.h"

UCLASS()
class FICSITREMOTEMONITORING_API AHttpServer : public AModSubsystem
{
	GENERATED_BODY()
	
public:	
	AHttpServer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "Http")
	bool IsServerStarted() const { return _isServerStarted; }
	static AHttpServer* Get(UWorld* world);

private:
	void StartServer();
	void StopServer();

	bool RequestAssembler(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestBlender(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestConstructor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestDrone(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestDroneStation(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestDropPod(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestExplorer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestExtractor(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestFactory(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestFoundry(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestManufacturer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestParticle(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestPlayer(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestPower(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestPowerSlug(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestProdStats(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestRefinery(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestSmelter(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestStorageInv(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestTractors(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestTrains(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestTrucks(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestTruckStation(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestVehicles(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);
	bool RequestWorldInv(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete);

	// Just print request for example
	void RequestPrint(const FHttpServerRequest& Request, bool PrintBody = true);

public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Http")
	int32 ServerPort = 9090;

private:
	bool _isServerStarted = false;

};
