#pragma once

#include "FRM_Power.h"

#undef GetForm

TArray<TSharedPtr<FJsonValue>> UFRM_Power::getCircuit(UObject* WorldContext)
{
	AFGCircuitSubsystem* CircuitSubsystem = AFGCircuitSubsystem::Get(WorldContext->GetWorld());

	TArray<TSharedPtr<FJsonValue>> JCircuitArray;

	for (UFGCircuitGroup* CircuitGroup : CircuitSubsystem->mCircuitGroups) {
		UFGPowerCircuitGroup* PowerGroup = Cast<UFGPowerCircuitGroup>(CircuitGroup);
		UFGPowerCircuit* PowerCircuit = (PowerGroup->mCircuits[0]);
		UFGCircuit* Circuit = Cast<UFGCircuit>(PowerCircuit);

		TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();

		int32 CircuitID = Circuit->GetCircuitGroupID();

		JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(CircuitID));
		JCircuit->Values.Add("PowerProduction", MakeShared<FJsonValueNumber>(PowerGroup->mBaseProduction));
		JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(PowerGroup->mConsumption));
		JCircuit->Values.Add("PowerCapacity", MakeShared<FJsonValueNumber>(PowerGroup->mMaximumProductionCapacity));
		JCircuit->Values.Add("PowerMaxConsumed", MakeShared<FJsonValueNumber>(PowerGroup->mMaximumPowerConsumption));
		JCircuit->Values.Add("BatteryDifferential", MakeShared<FJsonValueNumber>(PowerCircuit->mBatterySumPowerInput));
		JCircuit->Values.Add("BatteryPercent", MakeShared<FJsonValueNumber>(100 * (PowerGroup->mTotalPowerStore / PowerGroup->mTotalPowerStoreCapacity)));
		JCircuit->Values.Add("BatteryCapacity", MakeShared<FJsonValueNumber>(PowerGroup->mTotalPowerStoreCapacity));
		JCircuit->Values.Add("BatteryTimeEmpty", MakeShared<FJsonValueString>(SecondsToTimeString(PowerCircuit->mTimeToBatteriesEmpty)));
		JCircuit->Values.Add("BatteryTimeFull", MakeShared<FJsonValueString>(SecondsToTimeString(PowerCircuit->mTimeToBatteriesFull)));
		JCircuit->Values.Add("FuseTriggered", MakeShared<FJsonValueBoolean>(PowerGroup->mIsAnyFuseTriggered));

		JCircuitArray.Add(MakeShared<FJsonValueObject>(JCircuit));
	};

	return JCircuitArray;
};

TArray<TSharedPtr<FJsonValue>> UFRM_Power::getSwitches(UObject* WorldContext)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableCircuitSwitch*> PowerSwitches;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableCircuitSwitch>(PowerSwitches);

	TArray<TSharedPtr<FJsonValue>> JSwitchesArray;

	for (AFGBuildableCircuitSwitch* PowerSwitch : PowerSwitches) {

		TSharedPtr<FJsonObject> JSwitches = MakeShared<FJsonObject>();

		UFGCircuitConnectionComponent* ConnectionZero = PowerSwitch->GetConnection0();
		UFGCircuitConnectionComponent* ConnectionOne = PowerSwitch->GetConnection1();

		JSwitches->Values.Add("Name", MakeShared<FJsonValueString>(PowerSwitch->mDisplayName.ToString()));
		JSwitches->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(PowerSwitch->GetClass())));
		JSwitches->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(PowerSwitch)));
		JSwitches->Values.Add("SwitchTag", MakeShared<FJsonValueString>(PowerSwitch->GetBuildingTag_Implementation()));
		JSwitches->Values.Add("Connected0", MakeShared<FJsonValueNumber>(ConnectionZero->IsConnected()));
		JSwitches->Values.Add("IsOn", MakeShared<FJsonValueBoolean>(PowerSwitch->IsSwitchOn()));
		JSwitches->Values.Add("Connected1", MakeShared<FJsonValueNumber>(ConnectionOne->IsConnected()));
		JSwitches->Values.Add("Primary", MakeShared<FJsonValueNumber>(ConnectionZero->GetCircuitID()));
		JSwitches->Values.Add("Secondary", MakeShared<FJsonValueNumber>(ConnectionOne->GetCircuitID()));
		JSwitches->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(PowerSwitch), PowerSwitch->GetBuildingTag_Implementation(), TEXT("Power Switch"))));

		JSwitchesArray.Add(MakeShared<FJsonValueObject>(JSwitches));

	}

	return JSwitchesArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Power::getGenerators(UObject* WorldContext, UClass* TypedBuildable)
{

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);

	TArray<TSharedPtr<FJsonValue>> JGeneratorArray;

	for (AFGBuildable* Buildable : Buildables) {

		TSharedPtr<FJsonObject> JGenerator = MakeShared<FJsonObject>();

		AFGBuildableGenerator* Generator = Cast<AFGBuildableGenerator>(Buildable);
		AFGBuildableGeneratorFuel* GeneratorFuel = Cast<AFGBuildableGeneratorFuel>(Buildable);
		AFGBuildableGeneratorNuclear* GeneratorNuclear = Cast<AFGBuildableGeneratorNuclear>(Buildable);
		AFGBuildableGeneratorGeoThermal* GeneratorGeo = Cast<AFGBuildableGeneratorGeoThermal>(Buildable);

		UFGPowerInfoComponent* PowerInfo = Generator->GetPowerInfo();

		float FuelAmount = 0;
		EResourceForm FuelForm;
		FString FormString = "Geothermal";

		TArray<TSharedPtr<FJsonValue>> JFuelArray;
		TSharedPtr<FJsonObject> JSupplemental = MakeShared<FJsonObject>();

		if (IsValid(GeneratorFuel)) {

			TSubclassOf<UFGItemDescriptor> Supplemental = GeneratorFuel->GetSupplementalResourceClass();

			JSupplemental->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Supplemental).ToString()));
			JSupplemental->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Supplemental.Get())));
			JSupplemental->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalConsumptionRateCurrent() * 60));
			JSupplemental->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalConsumptionRateCurrent() * 60));
			JSupplemental->Values.Add("PercentFull", MakeShared<FJsonValueNumber>(GeneratorFuel->GetSupplementalAmount() * 100));

			FuelAmount = GeneratorFuel->GetFuelAmount();
			FuelForm = GeneratorFuel->GetFuelResourceForm();
			
			TArray<TSubclassOf<UFGItemDescriptor>> FuelClasses = GeneratorFuel->GetAvailableFuelClasses();
			for (TSubclassOf<UFGItemDescriptor> FuelClass : FuelClasses) {

				TSharedPtr<FJsonObject> JFuel = MakeShared<FJsonObject>();

				auto EnergyValue = UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetEnergyValue(FuelClass), UFGItemDescriptor::GetForm(FuelClass));

				JFuel->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(FuelClass).ToString()));
				JFuel->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(FuelClass.Get())));
				JFuel->Values.Add("Amount", MakeShared<FJsonValueNumber>(EnergyValue));

				JFuelArray.Add(MakeShared<FJsonValueObject>(JFuel));
			}

			switch (FuelForm)
			{
			case EResourceForm::RF_SOLID	:	FormString = TEXT("Solid");
			case EResourceForm::RF_LIQUID	:	FormString = TEXT("Liquid");
			case EResourceForm::RF_GAS		:	FormString = TEXT("Gas");
			case EResourceForm::RF_HEAT		:	FormString = TEXT("Heat");
			case EResourceForm::RF_INVALID	:	FormString = TEXT("Geothermal");
			case EResourceForm::RF_LAST_ENUM:	FormString = TEXT("Unknown");
			}

		};

		TArray<TSharedPtr<FJsonValue>> JWasteArray;
		FString NuclearString = "";

		if (IsValid(GeneratorNuclear)) {
			
			TArray<FInventoryStack> WasteStacks;

			UFGInventoryComponent* InventoryComponent = GeneratorNuclear->mOutputInventory;
			InventoryComponent->GetInventoryStacks(WasteStacks);

			for (FInventoryStack WasteStack : WasteStacks) {

				TSharedPtr<FJsonObject> JWaste = MakeShared<FJsonObject>();

				AActor* ItemState;
				TSubclassOf<UFGItemDescriptor> WasteClass;
				FInventoryItem WasteClassItem;
				int32 Amount = 0;

				UFGInventoryLibrary::BreakInventoryStack(WasteStack, Amount, WasteClassItem);
				UFGInventoryLibrary::BreakInventoryItem(WasteClassItem, WasteClass, ItemState);

				JWaste->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(WasteClass).ToString()));
				JWaste->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(WasteClass.Get())));
				JWaste->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));

				JWasteArray.Add(MakeShared<FJsonValueObject>(JWaste));
			};

			switch (GeneratorNuclear->GetCurrentGeneratorNuclearWarning())
			{
			case EGeneratorNuclearWarning::GNW_None								:	NuclearString = TEXT("None");
			case EGeneratorNuclearWarning::GNW_WasteFull						:	NuclearString = TEXT("Waste Full");
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

		int32 CircuitID = 0; // PowerInfo->GetPowerCircuit()->GetCircuitID();

		float BaseProduction = PowerInfo->GetBaseProduction();
		float DynProductionCapacity = PowerInfo->GetDynamicProductionCapacity();
		float DynProductionDemand = PowerInfo->GetDynamicProductionDemandFactor() * 100;
		float RegDynamicProduction = PowerInfo->GetRegulatedDynamicProduction();
		bool IsFullBlast = PowerInfo->IsFullBlast();

		JGenerator->Values.Add("Name", MakeShared<FJsonValueString>(Generator->mDisplayName.ToString()));
		JGenerator->Values.Add("ClassName", MakeShared<FJsonValueString>(Generator->GetClass()->GetName()));
		JGenerator->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Generator))));
		JGenerator->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(CircuitID));
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
		JGenerator->Values.Add("FuelAmount", MakeShared<FJsonValueNumber>(FuelAmount));
		JGenerator->Values.Add("Supplement", MakeShared<FJsonValueObject>(JSupplemental));
		JGenerator->Values.Add("NuclearWarning", MakeShared<FJsonValueString>(NuclearString));
		JGenerator->Values.Add("FuelResource", MakeShared<FJsonValueString>(FormString));
		JGenerator->Values.Add("GeoMinPower", MakeShared<FJsonValueNumber>(GeoMinPower));
		JGenerator->Values.Add("GeoMaxPower", MakeShared<FJsonValueNumber>(GeoMaxPower));
		JGenerator->Values.Add("AvailableFuel", MakeShared<FJsonValueArray>(JFuelArray));
		JGenerator->Values.Add("WasteInventory", MakeShared<FJsonValueArray>(JWasteArray));
		JGenerator->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Generator), Generator->mDisplayName.ToString(), Generator->mDisplayName.ToString())));

		JGeneratorArray.Add(MakeShared<FJsonValueObject>(JGenerator));
	};

	return JGeneratorArray;
};
