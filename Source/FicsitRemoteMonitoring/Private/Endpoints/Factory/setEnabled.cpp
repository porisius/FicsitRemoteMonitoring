#include "Endpoints/Factory/setEnabled.h"

#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "FRM_Request.h"
#include "FRM_Library.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "FicsitRemoteMonitoring.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildableCircuitSwitch.h"
#include "../../../../../../../Source/FactoryGame/Public/Buildables/FGBuildableManufacturer.h"
#include "Containers/Array.h"

class FJsonValueObject;
class AFGBuildableGenerator;
class AFGBuildableCircuitSwitch;
class FJsonObject;
class FJsonValue;
class AFGBuildableManufacturer;
class AFGBuildable;
class AFGBuildableSubsystem;
class FString;

TArray<TSharedPtr<FJsonValue>> USetEnabled::setEnabled(UObject* WorldContext, FRequestData RequestData)
{

	TArray<TSharedPtr<FJsonValue>> JResponses;
	TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
	if (RequestData.Body.Num() == 0) return JResponses;
	
	for (const auto& BodyObject : RequestData.Body)
	{
		auto JsonObject = BodyObject->AsObject();

		// check if ID or status is present in this json object
		if (!JsonObject->HasField("ID") && !JsonObject->HasField("status"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field ID or status.");
			JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			return JResponses;;
		}

		FString RequestedBuildable = JsonObject->GetStringField("ID");

		TArray<AFGBuildableManufacturer*> Manufacturers;
		TArray<AFGBuildableCircuitSwitch*> CircuitSwitches;
		TArray<AFGBuildableGenerator*> PowerGenerators;

		AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext);
		BuildableSubsystem->GetTypedBuildable<AFGBuildableManufacturer>(Manufacturers);
		BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(CircuitSwitches);
		BuildableSubsystem->GetTypedBuildable<AFGBuildableGenerator>(PowerGenerators);

		bool PowerState = JsonObject->GetBoolField("status");
		
		for (AFGBuildableManufacturer* Manufacturer : Manufacturers)
		{
			if (Manufacturer && Manufacturer->GetName() == RequestedBuildable)
			{
				Manufacturer->SetIsProductionPaused(!PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(!Manufacturer->IsProductionPaused()));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			}
		}

		for (AFGBuildableCircuitSwitch* CircuitSwitch : CircuitSwitches)
		{
			if (CircuitSwitch && CircuitSwitch->GetName() == RequestedBuildable)
			{
				CircuitSwitch->SetSwitchOn(PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(CircuitSwitch->IsSwitchOn()));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			}
		}

		for (AFGBuildableGenerator* PowerGenerator : PowerGenerators)
		{
			if (PowerGenerator && PowerGenerator->GetName() == RequestedBuildable)
			{
				PowerGenerator->SetIsProductionPaused(!PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(!PowerGenerator->IsProductionPaused()));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			}
		}
	}

	return JResponses;
};
