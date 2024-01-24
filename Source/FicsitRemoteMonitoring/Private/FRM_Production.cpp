// Fill out your copyright notice in the Description page of Project Settings.


#include "FRM_Production.h"

FString UFRM_Production::getProdStats(UObject* WorldContext) {

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
			auto CurrentRecipe = Cast<UFGRecipe>(Manufacturer->GetCurrentRecipe());
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			auto Productivity = Manufacturer->GetProductivity();

			for (FItemAmount Product : CurrentRecipe->GetProducts()) {

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

			for (FItemAmount Ingredients : CurrentRecipe->GetIngredients()) {

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

	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableResourceExtractor")), Buildables);
	// Resource Building Production Stats
	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableResourceExtractor* Extractor = Cast<AFGBuildableResourceExtractor>(Buildable);

		auto ItemClass = Extractor->GetExtractableResource()->GetResourceClass();
		auto ProdCycle = Extractor->GetExtractionPerMinute();;
		auto Productivity = Extractor->GetProductivity();

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


	BuildableSubsystem->GetTypedBuildable(LoadObject<UClass>(nullptr, TEXT("/Script/FactoryGame.FGBuildableGenerator")), Buildables);
	// Power Generator Building Production Stats
	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableGeneratorFuel* Generator = Cast<AFGBuildableGeneratorFuel>(Buildable);

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

			auto Consumption = CurrentConsumed.FindRef(ClassName);
			auto MaxConsumption = TotalConsumed.FindRef(ClassName);
			auto Produced = CurrentProduced.FindRef(ClassName);
			auto MaxProduced = TotalProduced.FindRef(ClassName);

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
				// This is used to specify no form change in patches so it's okay now
				// UE_LOG(LogContentLib, Error, TEXT("Encountered EResourceForm::RF_LAST_ENUM, should be impossible"));
				FormString = "Unknown";
			};

			FString ProdPerMin = "P: ";
			ProdPerMin.Append(FString::SanitizeFloat(UFGBlueprintFunctionLibrary::RoundFloatWithPrecision(Produced, 2)));
			ProdPerMin.Append("/ min - C: ");
			ProdPerMin.Append(FString::SanitizeFloat(UFGBlueprintFunctionLibrary::RoundFloatWithPrecision(Consumption, 2)));
			ProdPerMin.Append("/ min");

			JProductionStats->Values.Add("Name", MakeShared<FJsonValueString>(ClassName->GetDisplayNameText().ToString()));
			JProductionStats->Values.Add("ClassName", MakeShared<FJsonValueString>(ClassName->GetClass()->GetName()));
			JProductionStats->Values.Add("ProdPerMin", MakeShared<FJsonValueString>(ProdPerMin));
			JProductionStats->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(100 * (Produced / MaxProduced)));
			JProductionStats->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(100 * (Consumption / MaxConsumption)));
			JProductionStats->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(Produced));
			JProductionStats->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProduced));
			JProductionStats->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(Consumption));
			JProductionStats->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumption));
			JProductionStats->Values.Add("Type", MakeShared<FJsonValueString>(FormString));

			JProductionStatsArray.Add(MakeShared<FJsonValueObject>(JProductionStats));
		};
	};

	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JProductionStatsArray, JsonWriter);

	return Write;

};