#pragma once

#include "FRM_Drones.h"
#include "FRM_Factory.h"
#include "FRM_Player.h"
#include "FRM_Power.h"
#include "FRM_Production.h"
#include "FRM_Trains.h"
#include "FRM_Vehicles.h"
#include "Misc/EnumRange.h"
#include "FRM_APIEndpointLinker.generated.h"

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

UCLASS()
class FICSITREMOTEMONITORING_API UAPI_Endpoints : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

   
    UFUNCTION(BlueprintCallable)
    static FString API_Endpoint(UObject* WorldContext, EAPIEndpoints APICall);

    static TArray<TSharedPtr<FJsonValue>> getAll(UObject* WorldContext);
    static TArray<TSharedPtr<FJsonValue>> API_Endpoint_Call(UObject* WorldContext, EAPIEndpoints APICall);
};