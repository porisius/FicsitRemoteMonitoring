#include "Endpoints/Factory/setEnabled.h"

#include "FGBuildableSubsystem.h"
#include "Buildables/FGBuildableCircuitSwitch.h"
#include "Buildables/FGBuildableFactory.h"
#include "FRM_Request.h"
#include "FRM_Library.h"
#include "FicsitRemoteMonitoring.h"
#include "Containers/Array.h"

class FJsonValueObject;
class AFGBuildableGenerator;
class AFGBuildableCircuitSwitch;
class FJsonObject;
class FJsonValue;
class AFGBuildableFactory;
class AFGBuildable;
class AFGBuildableSubsystem;
class FString;

TArray<TSharedPtr<FJsonValue>> USetEnabled::setEnabled(UObject* WorldContext, FRequestData RequestData)
{

	TArray<TSharedPtr<FJsonValue>> JResponses;
	TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
	if (RequestData.Body.Num() == 0) return JResponses;

	TArray<AFGBuildableFactory*> FactoryBuildings;
	TArray<AFGBuildableCircuitSwitch*> CircuitSwitches;
	TArray<AFGBuildableGenerator*> PowerGenerators;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFactory>(FactoryBuildings);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(CircuitSwitches);
	
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

		bool foundBuildable = false;

		FString RequestedBuildable = JsonObject->GetStringField("ID");

		bool PowerState = JsonObject->GetBoolField("status");
		
		for (AFGBuildableFactory* FactoryBuilding : FactoryBuildings)
		{
			if (FactoryBuilding && FactoryBuilding->GetName() == RequestedBuildable)
			{
				//Inverted PowerState as preferred True = On, False = Off. AFGBuildableFactory uses IsProductionPaused, True = Off, False = On
				FactoryBuilding->SetIsProductionPaused(!PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(!FactoryBuilding->IsProductionPaused()));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));

				foundBuildable = true;
			}

			// If found start on exit for
			if (foundBuildable) { break; }
		}

		// If found start on next RequestedBuildable
		if (foundBuildable) { continue; }
		
		for (AFGBuildableCircuitSwitch* CircuitSwitch : CircuitSwitches)
		{
			if (CircuitSwitch && CircuitSwitch->GetName() == RequestedBuildable)
			{
				CircuitSwitch->SetSwitchOn(PowerState);
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
				JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(CircuitSwitch->IsSwitchOn()));
				JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
				
				foundBuildable = true;
			}

			// If found start on exit for
			if (foundBuildable) { break; }
		}
	}

	return JResponses;
};
