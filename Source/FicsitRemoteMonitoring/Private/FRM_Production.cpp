// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Production.h"

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getProdStats(UObject* WorldContext) {

	TMap<TSubclassOf<UFGItemDescriptor>, float> CurrentConsumed;
	TMap<TSubclassOf<UFGItemDescriptor>, float> CurrentProduced;
	TMap<TSubclassOf<UFGItemDescriptor>, float> TotalConsumed;
	TMap<TSubclassOf<UFGItemDescriptor>, float> TotalProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableManufacturer")), Buildables);

	// Factory Building Production Stats
	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			auto Productivity = Manufacturer->GetProductivity();

			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {

				auto ItemClass = Product.ItemClass;
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential;

				if (CurrentProduced.Contains(ItemClass)) {
					CurrentProduced.Add(ItemClass) = CurrentProd + CurrentProduced.FindRef(ItemClass);
					TotalProduced.Add(ItemClass) = MaxProd + TotalProduced.FindRef(ItemClass);
				}
				else {
					CurrentProduced.Add(ItemClass) = CurrentProd;
					TotalProduced.Add(ItemClass) = MaxProd;
				};

			};

			for (FItemAmount Ingredients : CurrentRecipe.GetDefaultObject()->GetIngredients()) {

				auto ItemClass = Ingredients.ItemClass;
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredients.Amount, UFGItemDescriptor::GetForm(Ingredients.ItemClass));
				auto CurrentCons = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxConsumed = RecipeAmount * ProdCycle * CurrentPotential;

				if (CurrentConsumed.Contains(ItemClass)) {
					CurrentConsumed.Add(ItemClass) = CurrentCons + CurrentConsumed.FindRef(ItemClass);
					TotalConsumed.Add(ItemClass) = MaxConsumed + TotalConsumed.FindRef(ItemClass);
				}
				else {
					CurrentConsumed.Add(ItemClass) = CurrentCons;
					TotalConsumed.Add(ItemClass) = MaxConsumed;
				};
			};

		};
	};

	TArray<AFGBuildable*> ExtractorBuildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableResourceExtractor")), ExtractorBuildables);
	// Resource Building Production Stats
	for (AFGBuildable* BuildableExtractor : ExtractorBuildables) {

		AFGBuildableResourceExtractor* Extractor = Cast<AFGBuildableResourceExtractor>(BuildableExtractor);

		TScriptInterface<IFGExtractableResourceInterface> ResourceClass = Extractor->GetExtractableResource();
		TSubclassOf<UFGResourceDescriptor> ItemClass = ResourceClass->GetResourceClass();
		float ProdCycle = Extractor->GetExtractionPerMinute();;
		float Productivity = Extractor->GetProductivity();

		auto CurrentProd = ProdCycle * Productivity;

		if (CurrentProduced.Contains(ItemClass)) {
			CurrentProduced.Add(ItemClass) = CurrentProd + CurrentProduced.FindRef(ItemClass);
			TotalProduced.Add(ItemClass) = ProdCycle + TotalProduced.FindRef(ItemClass);
		}
		else {
			CurrentProduced.Add(ItemClass) = CurrentProd;
			TotalProduced.Add(ItemClass) = ProdCycle;
		};
	};

	TArray<AFGBuildable*> GeneratorBuildables;
	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableGenerator")), GeneratorBuildables);
	// Power Generator Building Production Stats
	for (AFGBuildable* BuildableGenerator : GeneratorBuildables) {

		AFGBuildableGeneratorFuel* Generator = Cast<AFGBuildableGeneratorFuel>(BuildableGenerator);

		auto FuelItemClass = Generator->GetCurrentFuelClass();
		auto EnergyValue = UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetEnergyValue(FuelItemClass), UFGItemDescriptor::GetForm(FuelItemClass));
		auto ProductionCapacity = Generator->GetCurrentPotential();
		auto Productivity = Generator->GetProductivity();
		
		auto MaxFuelConsumption = 60 * (ProductionCapacity / EnergyValue);
		auto FuelConsumption = MaxFuelConsumption * Productivity;

		if (CurrentProduced.Contains(FuelItemClass)) {
			CurrentConsumed.Add(FuelItemClass) = FuelConsumption + CurrentConsumed.FindRef(FuelItemClass);
			TotalConsumed.Add(FuelItemClass) = MaxFuelConsumption + TotalConsumed.FindRef(FuelItemClass);
		}
		else {
			CurrentConsumed.Add(FuelItemClass) = FuelConsumption;
			TotalConsumed.Add(FuelItemClass) = MaxFuelConsumption;
		};

		if (Generator->GetRequiresSupplementalResource()) {

			auto SupplementalItemClass = Generator->GetSupplementalResourceClass();
			auto Consumption = Generator->GetSupplementalConsumptionRateCurrent() * 60;
			auto MaxConsumption = Generator->GetSupplementalConsumptionRateMaximum() * 60;

			if (CurrentProduced.Contains(SupplementalItemClass)) {
				CurrentConsumed.Add(SupplementalItemClass) = Consumption + CurrentConsumed.FindRef(SupplementalItemClass);
				TotalConsumed.Add(SupplementalItemClass) = MaxConsumption + TotalConsumed.FindRef(SupplementalItemClass);
			}
			else {
				CurrentConsumed.Add(SupplementalItemClass) = Consumption;
				TotalConsumed.Add(SupplementalItemClass) = MaxConsumption;
			};
		};
	};

	TArray<TSubclassOf<UFGItemDescriptor>> ClassNames;
	UFGBlueprintFunctionLibrary::GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);
	TArray<TSharedPtr<FJsonValue>> JProductionStatsArray;

	for (TSubclassOf<UFGItemDescriptor> ClassName : ClassNames) {

		if (TotalConsumed.Contains(ClassName) || TotalProduced.Contains(ClassName)) {
			TSharedPtr<FJsonObject> JProductionStats = MakeShared<FJsonObject>();

			float Consumption = CurrentConsumed.FindRef(ClassName);
			float MaxConsumption = TotalConsumed.FindRef(ClassName);
			float Produced = CurrentProduced.FindRef(ClassName);
			float MaxProduced = TotalProduced.FindRef(ClassName);

			EResourceForm Form = UFGItemDescriptor::GetForm(ClassName);

			FString FormString = "Unknown";
			if (Form == EResourceForm::RF_SOLID) {
				FormString = "Solid";
			}
			else if (Form == EResourceForm::RF_LIQUID) {
				FormString = "Liquid";
			}
			else if (Form == EResourceForm::RF_GAS) {
				FormString = "Gas";
			}
			else if (Form == EResourceForm::RF_HEAT) {
				FormString = "Heat";
			}
			else if (Form == EResourceForm::RF_INVALID) {
				FormString = "Invalid";
			}
			else if (Form == EResourceForm::RF_LAST_ENUM) {
				FormString = "Unknown";
			};

			FString ProdPerMin = "P: ";
			ProdPerMin.Append(FString::SanitizeFloat(UFGBlueprintFunctionLibrary::RoundFloatWithPrecision(Produced, 2)));
			ProdPerMin.Append("/ min - C: ");
			ProdPerMin.Append(FString::SanitizeFloat(UFGBlueprintFunctionLibrary::RoundFloatWithPrecision(Consumption, 2)));
			ProdPerMin.Append("/ min");

			JProductionStats->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(ClassName).ToString()));
			JProductionStats->Values.Add("ClassName", MakeShared<FJsonValueString>(ClassName.GetDefaultObject()->GetClass()->GetName()));
			JProductionStats->Values.Add("ProdPerMin", MakeShared<FJsonValueString>(ProdPerMin));
			JProductionStats->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide(Produced, MaxProduced)))));
			JProductionStats->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide(Consumption, MaxConsumption)))));
			JProductionStats->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(Produced));
			JProductionStats->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProduced));
			JProductionStats->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(Consumption));
			JProductionStats->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumption));
			JProductionStats->Values.Add("Type", MakeShared<FJsonValueString>(FormString));

			JProductionStatsArray.Add(MakeShared<FJsonValueObject>(JProductionStats));
		};
	};

	return JProductionStatsArray;

};

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getSinkList(UObject* WorldContext) {

	TArray<FResourceSinkPointsData*> SinkRows;
	UDataTable* SinkTable = UFGResourceSinkSettings::GetPointsDataTable();

	//TArray<FName> SinkRowNames = SinkTable->GetRowNames();
	SinkTable->GetAllRows<FResourceSinkPointsData>("", SinkRows);

	TArray<TSharedPtr<FJsonValue>> JSinkPointsArray;

	for (FResourceSinkPointsData* SinkRow : SinkRows) {

		TSharedPtr<FJsonObject> JSinkRow = MakeShared<FJsonObject>();
		int32 SinkPoints = SinkRow->Points;
		int32 SinkOverridden = SinkRow->OverriddenResourceSinkPoints;

		if (SinkPoints == 0 && SinkOverridden == 0) {
			continue;
		}

		JSinkRow->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SinkRow->ItemClass).ToString()));
		JSinkRow->Values.Add("ClassName", MakeShared<FJsonValueString>(SinkRow->ItemClass.GetDefaultObject()->GetClass()->GetName()));
		JSinkRow->Values.Add("Points", MakeShared<FJsonValueNumber>(SinkPoints));
		JSinkRow->Values.Add("PointsOverride", MakeShared<FJsonValueNumber>(SinkOverridden));

		JSinkPointsArray.Add(MakeShared<FJsonValueObject>(JSinkRow));
	}

	return JSinkPointsArray;
}