#pragma once

#include "FRM_APIEndpointLinker.h"

TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, FString APICall) 
{
	TArray<TSharedPtr<FJsonValue>> Json;


	return Json;
}

TArray<TSharedPtr<FJsonValue>> API_Endpoint(UObject* WorldContext, const EAPIEndpoints APICall)
{
	TArray<TSharedPtr<FJsonValue>> Json;

	switch (APICall)
	{
		case EAPIEndpoints::getAssembler: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1'")));
		case EAPIEndpoints::getBelts: return Json;
		case EAPIEndpoints::getBiomassGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass.Build_GeneratorBiomass'")));
		case EAPIEndpoints::getBlender: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender'")));
		case EAPIEndpoints::getCoalGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal'")));
		case EAPIEndpoints::getConstructor: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1'")));
		case EAPIEndpoints::getDoggo: return Json;
		case EAPIEndpoints::getDrone: return UFRM_Drones::getDrone(WorldContext);
		case EAPIEndpoints::getDroneStation: return UFRM_Drones::getDroneStation(WorldContext);
		case EAPIEndpoints::getDropPod: return UFRM_Factory::getDropPod(WorldContext);
		case EAPIEndpoints::getExplorationSink: return Json;
		case EAPIEndpoints::getExplorer: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Vehicle/Explorer/BP_Explorer.BP_Explorer'")));
		case EAPIEndpoints::getExtractor: return UFRM_Factory::getResourceExtractor(WorldContext);
		case EAPIEndpoints::getFactoryCart: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart'")));
		case EAPIEndpoints::getFoundry: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1'")));
		case EAPIEndpoints::getFuelGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal'")));
		case EAPIEndpoints::getGeothermalGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal'")));
		case EAPIEndpoints::getHUBTerminal: return UFRM_Factory::getHubTerminal(WorldContext);
		case EAPIEndpoints::getManufacturer: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1'")));
		case EAPIEndpoints::getModList: return Json;
		case EAPIEndpoints::getNuclearGenerator: return UFRM_Power::getGenerators(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear'")));
		case EAPIEndpoints::getParticle: return UFRM_Factory::getFactory(WorldContext, AFGBuildableManufacturerVariablePower::StaticClass());
		case EAPIEndpoints::getPaths: return Json;
		case EAPIEndpoints::getPipes: return Json;
		case EAPIEndpoints::getPlayer: return UFRM_Player::getPlayer(WorldContext);
		case EAPIEndpoints::getPower: return UFRM_Power::getCircuit(WorldContext);
		case EAPIEndpoints::getPowerSlug: return UFRM_Factory::getPowerSlug(WorldContext);
		case EAPIEndpoints::getProdStats: return UFRM_Production::getProdStats(WorldContext);
		case EAPIEndpoints::getRadarTower: return Json;
		case EAPIEndpoints::getRecipes: return Json;
		case EAPIEndpoints::getRefinery: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1'")));
		case EAPIEndpoints::getResourceGeyser: return Json;
		case EAPIEndpoints::getResourceNode: return Json;
		case EAPIEndpoints::getResourceSink: return Json;
		case EAPIEndpoints::getResourceWell: return Json;
		case EAPIEndpoints::getSchematics: return Json;
		case EAPIEndpoints::getSinkList: return Json;
		case EAPIEndpoints::getSmelter: return UFRM_Factory::getFactory(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1'")));
		case EAPIEndpoints::getSpaceElevator: return Json;
		case EAPIEndpoints::getStorageInv: return UFRM_Factory::getStorageInv(WorldContext);
		case EAPIEndpoints::getSwitches: return Json;
		case EAPIEndpoints::getTractor: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart'")));
		case EAPIEndpoints::getTrains: return UFRM_Trains::getTrains(WorldContext);
		case EAPIEndpoints::getTrainStation: return Json;
		case EAPIEndpoints::getTruck: return UFRM_Vehicles::getVehicles(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Script/Engine.Blueprint'/Game/FactoryGame/Buildable/Vehicle/Golfcart/BP_Golfcart.BP_Golfcart'")));
		case EAPIEndpoints::getTruckStation: return UFRM_Vehicles::getTruckStation(WorldContext);
		case EAPIEndpoints::getWorldInv: return UFRM_Factory::getWorldInv(WorldContext);

			// Read API Group Endpoints
		case EAPIEndpoints::getAll: return Json;
		case EAPIEndpoints::getFactory: return Json;
		case EAPIEndpoints::getGenerators: return Json;
		case EAPIEndpoints::getVehicles: return Json;
	}


	return Json;
}
