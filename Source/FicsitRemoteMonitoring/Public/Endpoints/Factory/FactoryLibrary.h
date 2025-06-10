#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonValue.h"
#include "RemoteMonitoringLibrary.h"
#include "FGBuildableManufacturer.h"
#include "FactoryLibrary.generated.h"

struct FRequestData;

UCLASS()
class FICSITREMOTEMONITORING_API UFactoryLibrary : public URemoteMonitoringLibrary
{
	GENERATED_BODY()

public:

	static void getAssembler(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/AssemblerMk1/Build_AssemblerMk1.Build_AssemblerMk1_C")));
	}
	
	static void getBlender(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Blender/Build_Blender.Build_Blender_C")));
	}
	
	static void getConstructor(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ConstructorMk1/Build_ConstructorMk1.Build_ConstructorMk1_C")));
	}
	
	static void getConverter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Converter/Build_Converter.Build_Converter_C")));
	}

	static void getEncoder(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/QuantumEncoder/Build_QuantumEncoder.Build_QuantumEncoder_C")));
	}

	static void getFactory(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, AFGBuildableManufacturer::StaticClass());
	}
	
	static void getFoundry(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/FoundryMk1/Build_FoundryMk1.Build_FoundryMk1_C")));
	}

	static void getManufacturer(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/ManufacturerMk1/Build_ManufacturerMk1.Build_ManufacturerMk1_C")));
	}

	static void getPackager(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/Packager/Build_Packager.Build_Packager_C")));
	}
	
	static void getParticle(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/HadronCollider/Build_HadronCollider.Build_HadronCollider_C")));
	}
	
	static void getRefinery(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/OilRefinery/Build_OilRefinery.Build_OilRefinery_C")));
	}
	
	static void getSmelter(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {		
		OutJsonArray = getFactory_Helper(WorldContext, LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Buildable/Factory/SmelterMk1/Build_SmelterMk1.Build_SmelterMk1_C")));
	}
	
	static TArray<TSharedPtr<FJsonValue>> getFactory_Helper(UObject* WorldContext, UClass* TypedBuildable);
	
};
