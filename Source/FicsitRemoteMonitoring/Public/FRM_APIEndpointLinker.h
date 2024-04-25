#pragma once

#include "FRM_Drones.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_Power.h"
#include "FRM_Production.h"
#include "FRM_Trains.h"
#include "FRM_Vehicles.h"
#include "FRM_APIEndpointLinker.generated.h"

UENUM()
enum class EAPIEndpoints : uint8
{

	// Read API Endpoints
    getAssembler,
    getBelts,
    getBiomassGenerator,
    getBlender,
    getConstructor,
    getDoggo,
    getDrone,
    getDroneStation,
    getDropPod,
    getExplorationSink,
    getExplorer,
    getExtractor,
    getFactoryCart,
    getFoundry,
    getFuelGenerator,
    getGeothermalGenerator,
    getHUBTerminal,
    getManufacturer,
    getModList,
    getNuclearGenerator,
    getParticle,
    getPaths,
    getPipes,
	getPlayer,
	getPower,
    getPowerSlug,
    getProdStats,
    getRadarTower,
    getRecipes,
    getRefinery,
    getResourceGeyser,
    getResourceNode,
	getResourceSink,
    getResourceWell,
    getSchematics,
    getSinkList,
    getSmelter,
    getSpaceElevator,
    getStorageInv,
	getSwitches,
	getTractor,
	getTrains,
	getTrainStation,
	getTruck,
	getTruckStation,
	getWorldInv,

	// Read API Group Endpoints
	getAll,
	getFactory,
	getGenerators,
	getVehicles,
};

UCLASS()
class FICSITREMOTEMONITORING_API UAPI_Endpoints : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    static TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, FString APICall);
    static TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, EAPIEndpoints APICall);

};