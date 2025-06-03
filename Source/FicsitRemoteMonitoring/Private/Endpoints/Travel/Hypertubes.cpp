#include "Hypertubes.h"

#include "FGBuildablePipeHyper.h"
#include "FGPipeHyperStart.h"
#include "FGFactoryConnectionComponent.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildablePipeHyper.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"
#include "../../../Public/FRM_Library.h"

class AFGPipeHyperStart;
class AFGBuildableSubsystem;
class AFGBuildablePipeHyper;
class UFGPipeConnectionComponentBase;

TArray<TSharedPtr<FJsonValue>> UHypertubes::getHyperEntrance(UObject* WorldContext) {

	TArray<TSharedPtr<FJsonValue>> JHyperStartArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGPipeHyperStart*> HyperStarts;
	BuildableSubsystem->GetTypedBuildable<AFGPipeHyperStart>(HyperStarts);

	for (AFGPipeHyperStart* HyperStart : HyperStarts) {
		TSharedPtr<FJsonObject> JHypertube = UFRM_Library::CreateBuildableBaseJsonObject(HyperStart);
		JHypertube->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(UFRM_Library::getPowerConsumptionJSON(HyperStart->GetPowerInfo())));
		JHyperStartArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
	return JHyperStartArray;
}

TArray<TSharedPtr<FJsonValue>> UHypertubes::getHypertube(UObject* WorldContext) {

	TArray<TSharedPtr<FJsonValue>> JHypertubeArray;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildablePipeHyper*> Hypertubes;
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipeHyper>(Hypertubes);

	for (AFGBuildablePipeHyper* Hypertube : Hypertubes) {
		TSharedPtr<FJsonObject> JHypertube = UFRM_Library::CreateBuildableBaseJsonObject(Hypertube);

		UFGPipeConnectionComponentBase* ConnectionZero = Hypertube->GetConnection0();
		UFGPipeConnectionComponentBase* ConnectionOne = Hypertube->GetConnection1();
		
		JHypertube->Values.Add("location0", MakeShared<FJsonValueObject>(UFRM_Library::getActorHyperXYZ(Hypertube, ConnectionZero)));
		JHypertube->Values.Add("location1", MakeShared<FJsonValueObject>(UFRM_Library::getActorHyperXYZ(Hypertube, ConnectionOne)));
		JHypertube->Values.Add("SplineData", MakeShared<FJsonValueArray>(UFRM_Library::SplineToJSON(Hypertube, Hypertube->GetSplinePointData())));
		JHypertubeArray.Add(MakeShared<FJsonValueObject>(JHypertube));
	}
	return JHypertubeArray;
}