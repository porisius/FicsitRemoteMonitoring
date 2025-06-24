#pragma once

#include "Power.h"

#include "FGBuildableGenerator.h"
#include "FGBuildableGeneratorGeoThermal.h"
#include "FGBuildableGeneratorNuclear.h"
#include "FGBuildablePriorityPowerSwitch.h"
#include "FGCircuitConnectionComponent.h"
#include "FGCircuitSubsystem.h"
#include "FGBuildableWire.h"
#include "FGInventoryLibrary.h"
#include "FGPowerInfoComponent.h"
#include "FicsitRemoteMonitoring.h"
#include "RemoteMonitoringLibrary.h"
#include "FRM_Request.h"
#include "FRM_RequestData.h"
#include "Kismet/KismetSystemLibrary.h"

void UPower::getPower(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGCircuitSubsystem* CircuitSubsystem = AFGCircuitSubsystem::Get(WorldContext->GetWorld());

	for (UFGCircuitGroup* CircuitGroup : CircuitSubsystem->mCircuitGroups) {
		UFGPowerCircuitGroup* PowerGroup = Cast<UFGPowerCircuitGroup>(CircuitGroup);
		UFGPowerCircuit* PowerCircuit = PowerGroup->mCircuits[0];
		UFGCircuit* Circuit = Cast<UFGCircuit>(PowerCircuit);

		TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();

		int32 CircuitID = Circuit->GetCircuitGroupID();

		TArray<TSharedPtr<FJsonValue>> JCircuits;
		for (UFGPowerCircuit* PCircuit : PowerGroup->mCircuits) {
			JCircuits.Add(MakeShared<FJsonValueNumber>(PCircuit->GetCircuitID()));
		}

		JCircuit->Values.Add("CircuitGroupID", MakeShared<FJsonValueNumber>(CircuitID));
		JCircuit->Values.Add("PowerProduction", MakeShared<FJsonValueNumber>(PowerGroup->mBaseProduction));
		JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(PowerGroup->mConsumption));
		JCircuit->Values.Add("PowerCapacity", MakeShared<FJsonValueNumber>(PowerGroup->mMaximumProductionCapacity));
		JCircuit->Values.Add("PowerMaxConsumed", MakeShared<FJsonValueNumber>(PowerGroup->mMaximumPowerConsumption));
		JCircuit->Values.Add("BatteryInput", MakeShared<FJsonValueNumber>(PowerCircuit->mBatterySumPowerInput));
		JCircuit->Values.Add("BatteryOutput", MakeShared<FJsonValueNumber>(PowerCircuit->GetBatterySumPowerOutput()));
		JCircuit->Values.Add("BatteryDifferential", MakeShared<FJsonValueNumber>(PowerCircuit->mBatterySumPowerInput - PowerCircuit->GetBatterySumPowerOutput()));
		JCircuit->Values.Add("BatteryPercent", MakeShared<FJsonValueNumber>(SafeDivide_Float(PowerGroup->mTotalPowerStore, PowerGroup->mTotalPowerStoreCapacity) * 100));
		JCircuit->Values.Add("BatteryCapacity", MakeShared<FJsonValueNumber>(PowerGroup->mTotalPowerStoreCapacity));
		JCircuit->Values.Add("BatteryTimeEmpty", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(PowerCircuit->mTimeToBatteriesEmpty)));
		JCircuit->Values.Add("BatteryTimeFull", MakeShared<FJsonValueString>(UFGBlueprintFunctionLibrary::SecondsToTimeString(PowerCircuit->mTimeToBatteriesFull)));
		JCircuit->Values.Add("AssociatedCircuits", MakeShared<FJsonValueArray>(JCircuits));
		JCircuit->Values.Add("FuseTriggered", MakeShared<FJsonValueBoolean>(PowerGroup->mIsAnyFuseTriggered));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JCircuit));
	};
};

void UPower::getSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableCircuitSwitch*> PowerSwitches;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(PowerSwitches);

	for (AFGBuildableCircuitSwitch* PowerSwitch : PowerSwitches) {

		TSharedPtr<FJsonObject> JSwitches = CreateBuildableBaseJsonObject(PowerSwitch);

		UFGCircuitConnectionComponent* ConnectionZero = PowerSwitch->GetConnection0();
		UFGCircuitConnectionComponent* ConnectionOne = PowerSwitch->GetConnection1();

		int32 Priority = -1;
		FString Type = TEXT("Power Switch");
		if (const auto* PriorityPowerSwitch = Cast<AFGBuildablePriorityPowerSwitch>(PowerSwitch) )
		{
			Type = TEXT("Priority Power Switch");
			Priority = PriorityPowerSwitch->GetPriority();
		}

		FString Name = PowerSwitch->GetBuildingTag_Implementation();
		JSwitches->Values.Add("SwitchTag", MakeShared<FJsonValueString>(Name));
		JSwitches->Values.Add("Connected0", MakeShared<FJsonValueNumber>(ConnectionZero->IsConnected()));
		JSwitches->Values.Add("IsOn", MakeShared<FJsonValueBoolean>(PowerSwitch->IsSwitchOn()));
		JSwitches->Values.Add("Connected1", MakeShared<FJsonValueNumber>(ConnectionOne->IsConnected()));
		JSwitches->Values.Add("Primary", MakeShared<FJsonValueNumber>(ConnectionZero->GetCircuitID()));
		JSwitches->Values.Add("Secondary", MakeShared<FJsonValueNumber>(ConnectionOne->GetCircuitID()));
		JSwitches->Values.Add("Priority", MakeShared<FJsonValueNumber>(Priority));
		JSwitches->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(PowerSwitch), Name, Type)));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSwitches));

	}
}

void UPower::setSwitches(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	if (RequestData.Body.Num() == 0) return;

	const AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableCircuitSwitch*> PowerSwitches;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(PowerSwitches);

	for (const auto& BodyObject : RequestData.Body)
	{
		auto JsonObject = BodyObject->AsObject();

		// get switch id from json object
		FString SwitchID;
		if (UFRM_RequestLibrary::TryGetStringField(JsonObject, "ID", SwitchID, OutJsonArray)) continue;

		// check if priority, status or name is present in this json object
		if (!JsonObject->HasField("priority") && !JsonObject->HasField("status") && !JsonObject->HasField("name"))
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Missing field priority, name or status.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(SwitchID));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
			continue;
		}

		// get updatable fields
		int32 Priority = -1;
		FString Name;
		bool bStatus;
		JsonObject->TryGetBoolField("status", bStatus);
		JsonObject->TryGetNumberField("priority", Priority);
		JsonObject->TryGetStringField("name", Name);

		bool bSuccess = false;
		for (AFGBuildableCircuitSwitch* PowerSwitch : PowerSwitches)
		{
			if (PowerSwitch->GetName() == SwitchID)
			{
				TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();
				JResponse->Values.Add("ID", MakeShared<FJsonValueString>(SwitchID));

				// change name
				if (JsonObject->HasField("name"))
				{
					PowerSwitch->SetBuildingTag_Implementation(Name);
					PowerSwitch->SetHasBuildingTag_Implementation(Name.Len() > 0);
					JResponse->Values.Add("Name", MakeShared<FJsonValueString>(PowerSwitch->GetBuildingTag_Implementation()));
					bSuccess = true;
				}
				
				// toggle switch
				if (JsonObject->HasField("status"))
				{
					PowerSwitch->SetSwitchOn(bStatus);
					JResponse->Values.Add("Status", MakeShared<FJsonValueBoolean>(PowerSwitch->IsSwitchOn()));
					bSuccess = true;
				}

				// update priority
				if (JsonObject->HasField("priority") && Priority >= 0)
				{
					if (auto* PriorityPowerSwitch = Cast<AFGBuildablePriorityPowerSwitch>(PowerSwitch))
					{
						PriorityPowerSwitch->SetPriority(Priority);
						JResponse->Values.Add("Priority", MakeShared<FJsonValueNumber>(PriorityPowerSwitch->GetPriority()));
						bSuccess = true;
					}
					else if (!bSuccess)
					{
						JResponse = UFRM_RequestLibrary::GenerateError("This Switch is not a Priority Power Switch.");
					}
				}

				if (bSuccess) OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));

				break;
			}
		}

		if (!bSuccess)
		{
			const TSharedPtr<FJsonObject> JResponse = UFRM_RequestLibrary::GenerateError("Power Switch not found.");
			JResponse->Values.Add("ID", MakeShared<FJsonValueString>(SwitchID));
			OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
		}
	}
}

void UPower::getPowerUsage(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableFactory*> BuildableFactories;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableFactory>(BuildableFactories);

	for (AFGBuildableFactory* BuildableFactory : BuildableFactories)
	{
		TSharedPtr<FJsonObject> JUsage = CreateBaseJsonObject(BuildableFactory);

		JUsage->Values.Add("Name", MakeShared<FJsonValueString>(BuildableFactory->mDisplayName.ToString()));
		JUsage->Values.Add("ClassName", MakeShared<FJsonValueString>(BuildableFactory->GetClass()->GetName()));
		JUsage->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(BuildableFactory)));
		JUsage->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(BuildableFactory->GetPowerInfo())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JUsage));		
	}
}

void UPower::getCables(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());

	TArray<AFGBuildableWire*> PowerWires;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableWire>(PowerWires);

	for (AFGBuildableWire* PowerWire : PowerWires) {

		if (!IsValid(PowerWire)) { continue; }

		TSharedPtr<FJsonObject> JPowerWire = CreateBuildableBaseJsonObject(PowerWire);

		JPowerWire->Values.Add("location0", MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(PowerWire->GetConnectionLocation(0))));
		JPowerWire->Values.Add("location1", MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(PowerWire->GetConnectionLocation(1))));
		JPowerWire->Values.Add("Length", MakeShared<FJsonValueNumber>(PowerWire->GetLength()));
		JPowerWire->Values.Add("features", MakeShared<FJsonValueObject>(GetActorLineFeaturesJSON(PowerWire->GetConnectionLocation(0), PowerWire->GetConnectionLocation(1), PowerWire->mDisplayName.ToString(), PowerWire->mDisplayName.ToString())));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JPowerWire));

	};
};

TArray<TSharedPtr<FJsonValue>> UPower::getGenerators_Helper(UObject* WorldContext, UClass* TypedBuildable)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);

	TArray<TSharedPtr<FJsonValue>> JGeneratorArray;

	for (AFGBuildable* Buildable : Buildables) {

		TSharedPtr<FJsonObject> JGenerator = CreateBuildableBaseJsonObject(Buildable);

		AFGBuildableGenerator* Generator = Cast<AFGBuildableGenerator>(Buildable);
		AFGBuildableGeneratorFuel* GeneratorFuel = Cast<AFGBuildableGeneratorFuel>(Buildable);
		AFGBuildableGeneratorNuclear* GeneratorNuclear = Cast<AFGBuildableGeneratorNuclear>(Buildable);
		AFGBuildableGeneratorGeoThermal* GeneratorGeo = Cast<AFGBuildableGeneratorGeoThermal>(Buildable);
		AFGBuildableFactory* BuildableFactory = Cast<AFGBuildableFactory>(Buildable);

		UFGPowerInfoComponent* PowerInfo = Generator->GetPowerInfo();

		float FuelAmount = 0;
		int32 Somersloops = 0;
		int32 PowerShards = 0;
		
		EResourceForm FuelForm;
		FString FormString = "Geothermal";

		TArray<TSharedPtr<FJsonValue>> JFuelInventory;
		TArray<TSharedPtr<FJsonValue>> JFuelArray;
		TSharedPtr<FJsonObject> JSupplemental = MakeShared<FJsonObject>();

		if (BuildableFactory)
		{
			GetOverclockingItemsFromInventory(BuildableFactory->GetPotentialInventory(), Somersloops, PowerShards);
		}
		
		if (IsValid(GeneratorFuel) || IsValid(GeneratorNuclear))
		{
			JFuelInventory = GetInventoryJSON(GetGroupedInventoryItems(GeneratorFuel->GetFuelInventory()));
		}

		if (IsValid(GeneratorFuel)) {
			TSubclassOf<UFGItemDescriptor> Supplemental = GeneratorFuel->GetSupplementalResourceClass();

			JSupplemental->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Supplemental).ToString()));
			JSupplemental->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Supplemental.Get())));
			JSupplemental->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalConsumptionRateCurrent() * 60));
			JSupplemental->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalConsumptionRateCurrent() * 60));
			JSupplemental->Values.Add("PercentFull", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalAmount() * 100));

			FuelAmount = GeneratorFuel->GetFuelAmount();
			FuelForm = UFGItemDescriptor::GetForm(GeneratorFuel->GetCurrentFuelClass());
			UFGInventoryComponent* FuelInventory = GeneratorFuel->GetFuelInventory();
			
			for (TSoftClassPtr<UFGItemDescriptor> SoftFuelClass : GeneratorFuel->GetDefaultFuelClasses())
			{
				TSubclassOf<UFGItemDescriptor> FuelClass = SoftFuelClass.LoadSynchronous();
				if (!FuelClass) continue;

				TSharedPtr<FJsonObject> JFuel = MakeShared<FJsonObject>();

				float EnergyValue = UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetEnergyValue(FuelClass), UFGItemDescriptor::GetForm(FuelClass));

				JFuel->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(FuelClass).ToString()));
				JFuel->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(FuelClass.Get())));
				JFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(EnergyValue));

				JFuelArray.Add(MakeShared<FJsonValueObject>(JFuel));			
			}

			switch (FuelForm)
			{
				case EResourceForm::RF_SOLID	:	FormString = TEXT("Solid");
					break;
				case EResourceForm::RF_LIQUID	:	FormString = TEXT("Liquid");
					break;
				case EResourceForm::RF_GAS		:	FormString = TEXT("Gas");
					break;
				case EResourceForm::RF_HEAT		:	FormString = TEXT("Heat");
					break;
				case EResourceForm::RF_INVALID	:	FormString = TEXT("Geothermal");
					break;
				case EResourceForm::RF_LAST_ENUM:	FormString = TEXT("Unknown");
			}

		};

		TMap<TSubclassOf<UFGItemDescriptor>, int32> WasteInventory;
		FString NuclearString = "";

		if (IsValid(GeneratorNuclear)) {
			WasteInventory = GetGroupedInventoryItems(GeneratorNuclear->mOutputInventory);

			switch (GeneratorNuclear->GetCurrentGeneratorNuclearWarning())
			{
				case EGeneratorNuclearWarning::GNW_None								:	NuclearString = TEXT("None");
					break;
				case EGeneratorNuclearWarning::GNW_WasteFull						:	NuclearString = TEXT("Waste Full");
					break;
				case EGeneratorNuclearWarning::GNW_MissmatchBetweenInputAndWaste	:	NuclearString = TEXT("Mismatch Between Input And Waste");
			}
		}

		float GeoMinPower = 0;
		float GeoMaxPower = 0;

		if (IsValid(GeneratorGeo)) {
			GeoMinPower = GeneratorGeo->GetMinPowerProduction();
			GeoMaxPower = GeneratorGeo->GetMaxPowerProduction();
		}

		float Potential = Generator->GetCurrentPotential();
		float PotentialCapacity = Generator->CalcPowerProductionCapacityForPotential(Potential);

		float BaseProduction = PowerInfo->GetBaseProduction();
		float DynProductionCapacity = PowerInfo->GetDynamicProductionCapacity();
		float DynProductionDemand = PowerInfo->GetDynamicProductionDemandFactor() * 100;
		float RegDynamicProduction = PowerInfo->GetRegulatedDynamicProduction();
		bool IsFullBlast = PowerInfo->IsFullBlast();

		JGenerator->Values.Add("BaseProd", MakeShared<FJsonValueNumber>(Generator->GetPowerProductionCapacity()));
		JGenerator->Values.Add("DynamicProdCapacity", MakeShared<FJsonValueNumber>(DynProductionCapacity));
		JGenerator->Values.Add("DynamicProdDemandFactor", MakeShared<FJsonValueNumber>(DynProductionDemand));
		JGenerator->Values.Add("RegulatedDemandProd", MakeShared<FJsonValueNumber>(RegDynamicProduction));
		JGenerator->Values.Add("IsFullSpeed", MakeShared<FJsonValueBoolean>(PowerInfo->IsFullBlast()));
		JGenerator->Values.Add("CanStart", MakeShared<FJsonValueBoolean>(Generator->CanStartPowerProduction()));
		JGenerator->Values.Add("LoadPercentage", MakeShared<FJsonValueNumber>(Generator->GetLoadPercentage() * 100));
		JGenerator->Values.Add("ProdPowerConsumption", MakeShared<FJsonValueNumber>(Generator->GetProducingPowerConsumption()));
		JGenerator->Values.Add("CurrentPotential", MakeShared<FJsonValueNumber>(Potential * 100));
		JGenerator->Values.Add("ProductionCapacity", MakeShared<FJsonValueNumber>(Generator->GetPowerProductionCapacity()));
		JGenerator->Values.Add("DefaultProductionCapacity", MakeShared<FJsonValueNumber>(Generator->GetDefaultPowerProductionCapacity()));
		JGenerator->Values.Add("PowerProductionPotential", MakeShared<FJsonValueNumber>(PotentialCapacity));
		JGenerator->Values.Add("Somersloops", MakeShared<FJsonValueNumber>(Somersloops));
		JGenerator->Values.Add("PowerShards", MakeShared<FJsonValueNumber>(PowerShards));
		JGenerator->Values.Add("FuelAmount", MakeShared<FJsonValueNumber>(FuelAmount));
		JGenerator->Values.Add("Supplement", MakeShared<FJsonValueObject>(JSupplemental));
		JGenerator->Values.Add("NuclearWarning", MakeShared<FJsonValueString>(NuclearString));
		JGenerator->Values.Add("FuelResource", MakeShared<FJsonValueString>(FormString));
		JGenerator->Values.Add("GeoMinPower", MakeShared<FJsonValueNumber>(GeoMinPower));
		JGenerator->Values.Add("GeoMaxPower", MakeShared<FJsonValueNumber>(GeoMaxPower));
		JGenerator->Values.Add("AvailableFuel", MakeShared<FJsonValueArray>(JFuelArray));
		JGenerator->Values.Add("WasteInventory", MakeShared<FJsonValueArray>(GetInventoryJSON(WasteInventory)));
		JGenerator->Values.Add("FuelInventory", MakeShared<FJsonValueArray>(JFuelInventory));
		JGenerator->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Generator->GetPowerInfo())));
		JGenerator->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Generator), Generator->mDisplayName.ToString(), Generator->mDisplayName.ToString())));

		JGeneratorArray.Add(MakeShared<FJsonValueObject>(JGenerator));
	};

	return JGeneratorArray;
};
