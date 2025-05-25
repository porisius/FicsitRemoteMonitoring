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
	if (RequestData.Body.Num() == 0) return JResponses;

	TArray<AFGBuildableFactory*> FactoryBuildings;
	TArray<AFGBuildableCircuitSwitch*> CircuitSwitches;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFactory>(FactoryBuildings);
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(CircuitSwitches);
	
	for (const auto& BodyObject : RequestData.Body)
	{
		FString RequestedBuildable;
		auto JsonObject = BodyObject->AsObject();
		if (UFRM_RequestLibrary::TryGetStringField(JsonObject, "ID", RequestedBuildable, JResponses)) continue;

		// check if ID or status is present in this json object
		if (!JsonObject->HasField("status"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field status.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(RequestedBuildable));
			JResponses.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		bool foundBuildable = false;

		bool PowerState = JsonObject->GetBoolField("status");
		
		for (AFGBuildableFactory* FactoryBuilding : FactoryBuildings)
		{
			if (FactoryBuilding && FactoryBuilding->GetName() == RequestedBuildable)
			{
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
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
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();

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
