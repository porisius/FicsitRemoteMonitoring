#pragma once

#include "CoreMinimal.h"
#include "FGBuildableGenerator.h"
#include "RemoteMonitoringLibrary.h"
#include "Power.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UPower : public URemoteMonitoringLibrary
{
	GENERATED_BODY()
	
public:

	static void getBiomassGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorBiomass/Build_GeneratorBiomass_Automated.Build_GeneratorBiomass_Automated_C")));
	}
	
	static void getCoalGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorCoal/Build_GeneratorCoal.Build_GeneratorCoal_C")));
	}
	
	static void getFuelGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorFuel/Build_GeneratorFuel.Build_GeneratorFuel_C")));
	}
	
	static void getGeothermalGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorGeoThermal/Build_GeneratorGeoThermal.Build_GeneratorGeoThermal_C")));
	}

	static void getNuclearGenerator(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/GeneratorNuclear/Build_GeneratorNuclear.Build_GeneratorNuclear_C")));
	}

	static void getGenerators(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getGenerators_Helper(WorldContext, AFGBuildableGenerator::StaticClass());
	}
	
	static void getPower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void setSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getPowerUsage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);
	static void getCables(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray);

	//Helper Functions

	static TArray<TSharedPtr<FJsonValue>> getGenerators_Helper(UObject* WorldContext, UClass* TypedBuildable);
	
private:
	friend class UFGPowerCircuit;
	friend class UFGPowerCircuitGroup;
	friend class AFGBuildableGenerator;
	friend class AFGBuildableGeneratorFuel;
	friend class AFGBuildableGeneratorNuclear;
	friend class AFGBuildableGeneratorGeoThermal;
	friend class AFGCircuitSubsystem;

};