
#include "Endpoints/Travel/Hypertubes.h"

#include "Buildables/FGBuildablePipeHyper.h"
#include "Buildables/FGBuildablePipeHyperJunction.h"
#include "FGFactoryConnectionComponent.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildablePipeHyper.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"
#include "../../../Public/RemoteMonitoringLibrary.h"
#include "Buildables/FGPipeHyperStart.h"

class AFGPipeHyperStart;
class AFGBuildableSubsystem;
class AFGBuildablePipeHyper;
class UFGPipeConnectionComponentBase;

void UHypertubes::getHyperEntrance(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGPipeHyperStart*> HyperStarts;
	BuildableSubsystem->GetTypedBuildable<AFGPipeHyperStart>(HyperStarts);

	for (AFGPipeHyperStart* HyperStart : HyperStarts) {
		TSharedPtr<FJsonObject> JHypertube = CreateBuildableBaseJsonObject(HyperStart);
		JHypertube->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(HyperStart->GetPowerInfo())));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
}

void UHypertubes::getHypertube(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	const float SampleDistance = UFRMConfigManager::GetConfigOrDefault<float>(TEXT("General.SplineSampleDistance"), 75.0f);
	
	TArray<AFGBuildablePipeHyper*> Hypertubes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeHyper>(Hypertubes);

	for (AFGBuildablePipeHyper* Hypertube : Hypertubes) {
		TSharedPtr<FJsonObject> JHypertube = CreateBuildableBaseJsonObject(Hypertube);

		UFGPipeConnectionComponentBase* ConnectionZero = Hypertube->GetConnection0();
		UFGPipeConnectionComponentBase* ConnectionOne = Hypertube->GetConnection1();
		
		JHypertube->Values.Add("location0", MakeShared<FJsonValueObject>(getActorHyperXYZ(Hypertube, ConnectionZero)));
		JHypertube->Values.Add("location1", MakeShared<FJsonValueObject>(getActorHyperXYZ(Hypertube, ConnectionOne)));
		JHypertube->Values.Add("SplineData", MakeShared<FJsonValueArray>(SplineToJSON(Hypertube->GetSplineComponent(), SampleDistance)));
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
}

void UHypertubes::getHyperJunctions(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipeHyperJunction*> Hypertubes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeHyperJunction>(Hypertubes);

	for (AFGBuildablePipeHyperJunction* Hypertube : Hypertubes) {
		TSharedPtr<FJsonObject> JHypertube = CreateBuildableBaseJsonObject(Hypertube);

		UFGPipeConnectionComponentBase* ConnectionZero = Hypertube->GetConnection0();
		UFGPipeConnectionComponentBase* ConnectionOne = Hypertube->GetConnection1();
		
		JHypertube->Values.Add("location0", MakeShared<FJsonValueObject>(getActorHyperXYZ(Hypertube, ConnectionZero)));
		JHypertube->Values.Add("location1", MakeShared<FJsonValueObject>(getActorHyperXYZ(Hypertube, ConnectionOne)));
		
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
}