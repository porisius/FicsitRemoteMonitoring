#include "FRM_Production.h"
#include <FicsitRemoteMonitoring.h>

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getProdStats(UObject* WorldContext) {

	TMap<TSubclassOf<UFGItemDescriptor>, float> CurrentConsumed;
	TMap<TSubclassOf<UFGItemDescriptor>, float> CurrentProduced;
	TMap<TSubclassOf<UFGItemDescriptor>, float> TotalConsumed;
	TMap<TSubclassOf<UFGItemDescriptor>, float> TotalProduced;

	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildableManufacturer*> Buildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableManufacturer>(Buildables);

	// Factory Building Production Stats
	for (AFGBuildableManufacturer* Manufacturer : Buildables) {

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = UKismetMathLibrary::SafeDivide(60, Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe()));
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

	TArray<AFGBuildableResourceExtractor*> ExtractorBuildables;
	BuildableSubsystem->GetTypedBuildable<AFGBuildableResourceExtractor>(ExtractorBuildables);
	// Resource Building Production Stats
	for (AFGBuildableResourceExtractor* Extractor : ExtractorBuildables) {

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

	TArray<AFGBuildableGeneratorFuel*> GeneratorBuildables;
	BuildableSubsystem->GetTypedBuildable< AFGBuildableGeneratorFuel>(GeneratorBuildables);
	// Power Generator Building Production Stats
	for (AFGBuildableGeneratorFuel* Generator : GeneratorBuildables) {

		TSubclassOf<UFGItemDescriptor> FuelItemClass = Generator->GetCurrentFuelClass();
		float EnergyValue = UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetEnergyValue(FuelItemClass), UFGItemDescriptor::GetForm(FuelItemClass));
		float ProductionCapacity = Generator->GetCurrentPotential();
		float Productivity = Generator->GetProductivity();
		
		float MaxFuelConsumption = 60 * (UFRM_Library::SafeDivide_Float(ProductionCapacity, EnergyValue));
		float FuelConsumption = MaxFuelConsumption * Productivity;

		if (CurrentProduced.Contains(FuelItemClass)) {
			CurrentConsumed.Add(FuelItemClass) = FuelConsumption + CurrentConsumed.FindRef(FuelItemClass);
			TotalConsumed.Add(FuelItemClass) = MaxFuelConsumption + TotalConsumed.FindRef(FuelItemClass);
		}
		else {
			CurrentConsumed.Add(FuelItemClass) = FuelConsumption;
			TotalConsumed.Add(FuelItemClass) = MaxFuelConsumption;
		};

		if (Generator->GetRequiresSupplementalResource()) {

			TSubclassOf<UFGItemDescriptor> SupplementalItemClass = Generator->GetSupplementalResourceClass();
			float Consumption = Generator->GetSupplementalConsumptionRateCurrent() * 60;
			float MaxConsumption = Generator->GetSupplementalConsumptionRateMaximum() * 60;

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
			JProductionStats->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ClassName.Get())));
			JProductionStats->Values.Add("ProdPerMin", MakeShared<FJsonValueString>(ProdPerMin));
			JProductionStats->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide_Float(Produced, MaxProduced)))));
			JProductionStats->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>((100 * (UFRM_Library::SafeDivide_Float(Consumption, MaxConsumption)))));
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
		JSinkRow->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SinkRow->ItemClass->GetClass())));
		JSinkRow->Values.Add("Points", MakeShared<FJsonValueNumber>(SinkPoints));
		JSinkRow->Values.Add("PointsOverride", MakeShared<FJsonValueNumber>(SinkOverridden));

		JSinkPointsArray.Add(MakeShared<FJsonValueObject>(JSinkRow));
	}

	return JSinkPointsArray;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getResourceSink(UObject* WorldContext, EResourceSinkTrack ResourceSinkTrack) {

	TArray<TSharedPtr<FJsonValue>> JResourceSinkArray;
	TSharedPtr<FJsonObject> JResourceSink = MakeShared<FJsonObject>();
	FString SinkName;

	AFGResourceSinkSubsystem* ResourceSinkSubsystem = AFGResourceSinkSubsystem::Get(WorldContext);

	switch (ResourceSinkTrack) {
		case EResourceSinkTrack::RST_Default		: SinkName = "Resource";
		case EResourceSinkTrack::RST_Exploration	: SinkName = "Exploration";
	}

	TSharedPtr<FJsonObject> JCoupon = MakeShared<FJsonObject>();
	TSubclassOf<UFGItemDescriptor> CouponClass = ResourceSinkSubsystem->GetCouponClass();
	TArray<TSharedPtr<FJsonValue>> PointHistory;
	
	for (int32 PointGraph : ResourceSinkSubsystem->GetGlobalPointHistory(ResourceSinkTrack)) {
		PointHistory.Add(MakeShared<FJsonValueNumber>(PointGraph));
	}

	JCoupon->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(CouponClass).ToString()));
	JCoupon->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(CouponClass)));

	JResourceSink->Values.Add("Name", MakeShared<FJsonValueString>(SinkName));
	JResourceSink->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ResourceSinkSubsystem->GetClass())));
	JResourceSink->Values.Add("CouponType", MakeShared<FJsonValueObject>(JCoupon));
	JResourceSink->Values.Add("NumCoupon", MakeShared<FJsonValueNumber>(ResourceSinkSubsystem->GetNumCoupons()));
	JResourceSink->Values.Add("Percent", MakeShared<FJsonValueNumber>(ResourceSinkSubsystem->GetProgressionTowardsNextCoupon(ResourceSinkTrack)));
	JResourceSink->Values.Add("GraphPoints", MakeShared<FJsonValueArray>(PointHistory));
	JResourceSink->Values.Add("TotalPoints", MakeShared<FJsonValueNumber>(ResourceSinkSubsystem->GetNumTotalPoints(ResourceSinkTrack)));
	JResourceSink->Values.Add("PointsToCoupon", MakeShared<FJsonValueNumber>(ResourceSinkSubsystem->GetNumCoupons()));

	JResourceSinkArray.Add(MakeShared<FJsonValueObject>(JResourceSink));

	return JResourceSinkArray;
}

TSharedPtr<FJsonObject> UFRM_Production::getRecipe(UObject* WorldContext, TSubclassOf<UFGRecipe> Recipe) {
	
	TSharedPtr<FJsonObject> JRecipe = MakeShared<FJsonObject>();
	
	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	if (!IsValid(Recipe)) { return JRecipe; };

	float ManualDuration = UFGRecipe::GetManualManufacturingDuration(Recipe);
	float FactoryDuration = UFGRecipe::GetManufacturingDuration(Recipe);

	TArray<TSharedPtr<FJsonValue>> JEventsArray;

	for (EEvents Event : UFGRecipe::GetRelevantEvents(Recipe)) {

		FString EventString;

		switch (Event) {
		case EEvents::EV_Birthday: EventString = TEXT("Satisfactory Birthday");
		case EEvents::EV_Christmas: EventString = TEXT("Christmas");
		case EEvents::EV_CSSBirthday: EventString = TEXT("Coffee Stain Studios Birthday");
		case EEvents::EV_FirstOfApril: EventString = TEXT("April Fools Day");
		case EEvents::EV_None: EventString = TEXT("");
		}

		JEventsArray.Add(MakeShared<FJsonValueString>(EventString));
	}

	TArray<TSharedPtr<FJsonValue>> JIngredientArray;

	for (FItemAmount Ingredient : UFGRecipe::GetIngredients(Recipe)) {
		TSharedPtr<FJsonObject> JIngredient = MakeShared<FJsonObject>();

		auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredient.Amount, UFGItemDescriptor::GetForm(Ingredient.ItemClass));
		auto ManualRate = (UKismetMathLibrary::SafeDivide(60, ManualDuration)) * RecipeAmount;
		auto FactoryRate = (UKismetMathLibrary::SafeDivide(60, FactoryDuration)) * RecipeAmount;

		JIngredient->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Ingredient.ItemClass).ToString()));
		JIngredient->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Ingredient.ItemClass)));
		JIngredient->Values.Add("Amount", MakeShared<FJsonValueNumber>(RecipeAmount));
		JIngredient->Values.Add("ManualRate", MakeShared<FJsonValueNumber>(ManualRate));
		JIngredient->Values.Add("FactoryRate", MakeShared<FJsonValueNumber>(FactoryRate));

		JIngredientArray.Add(MakeShared<FJsonValueObject>(JIngredient));
	};

	TArray<TSharedPtr<FJsonValue>> JProductArray;

	for (FItemAmount Product : UFGRecipe::GetProducts(Recipe)) {
		TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();

		float RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
		double ManualRate = (UKismetMathLibrary::SafeDivide(60, ManualDuration)) * RecipeAmount;
		double FactoryRate = (UKismetMathLibrary::SafeDivide(60, FactoryDuration)) * RecipeAmount;

		JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Product.ItemClass).ToString()));
		JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Product.ItemClass)));
		JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(RecipeAmount));
		JProduct->Values.Add("ManualRate", MakeShared<FJsonValueNumber>(ManualRate));
		JProduct->Values.Add("FactoryRate", MakeShared<FJsonValueNumber>(FactoryRate));

		JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
	};

	TArray<TSharedPtr<FJsonValue>> JProducedInArray;

	for (TSubclassOf<UObject> Workshop : UFGRecipe::GetProducedIn(Recipe)) {
		JProducedInArray.Add(MakeShared<FJsonValueString>(UKismetSystemLibrary::GetDisplayName(Workshop)));
	}

	FString DisplayName;
	FString ClassName;
	FString CategoryName;

	ModSubsystem->RecipeNames_BIE(Recipe, DisplayName, ClassName, CategoryName);

	UE_LOG(LogFRMAPI, Warning, TEXT("Recipe: %s - Class: %s - Category Name: %s"), *FString(DisplayName), *FString(ClassName), *FString(CategoryName));

	JRecipe->Values.Add("Name", MakeShared<FJsonValueString>(DisplayName));
	JRecipe->Values.Add("ClassName", MakeShared<FJsonValueString>(ClassName));
	JRecipe->Values.Add("Category", MakeShared<FJsonValueString>((CategoryName)));
	JRecipe->Values.Add("Events", MakeShared<FJsonValueArray>(JEventsArray));
	JRecipe->Values.Add("Ingredients", MakeShared<FJsonValueArray>(JIngredientArray));
	JRecipe->Values.Add("Products", MakeShared<FJsonValueArray>(JProductArray));
	JRecipe->Values.Add("ProducedIn", MakeShared<FJsonValueArray>(JProducedInArray));
	JRecipe->Values.Add("ManualDuration", MakeShared<FJsonValueNumber>(ManualDuration));
	JRecipe->Values.Add("FactoryDuration", MakeShared<FJsonValueNumber>(FactoryDuration));

	return JRecipe;
}

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getRecipes(UObject* WorldContext) {

	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());

	TArray<TSharedPtr<FJsonValue>> JRecipeArray;

	TArray<TSubclassOf<UFGSchematic>> Schematics;
	SchematicManager->GetAllSchematics(Schematics);

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	for (TSubclassOf<UFGSchematic> Schematic : Schematics) {

		TSharedPtr<FJsonObject> JSchematic = MakeShared<FJsonObject>();

		TArray<TSubclassOf<UFGRecipe>> Recipes;

		bool Purchased = false;
		bool HasUnlocks = false;
		bool LockedAny = false;
		bool LockedTutorial = false;
		bool LockedDependent = false;
		bool LockedPhase = false;
		bool Tutorial = false;

		ModSubsystem->SchematicToRecipes_BIE(WorldContext, Schematic, Recipes, Purchased, HasUnlocks, LockedAny, LockedTutorial, LockedDependent, LockedPhase, Tutorial);

		for (TSubclassOf<UFGRecipe> Recipe : Recipes) {

			TSharedPtr<FJsonObject> JRecipe = UFRM_Production::getRecipe(WorldContext, Recipe);
			JRecipeArray.Add(MakeShared<FJsonValueObject>(JRecipe));

		}

	}

	return JRecipeArray;

}

TArray<TSharedPtr<FJsonValue>> UFRM_Production::getSchematics(UObject* WorldContext) {

	AFGSchematicManager* SchematicManager = AFGSchematicManager::Get(WorldContext->GetWorld());

	TArray<TSharedPtr<FJsonValue>> JSchematicsArray;

	TArray<TSubclassOf<UFGSchematic>> Schematics;
	SchematicManager->GetAllSchematics(Schematics);

	AFicsitRemoteMonitoring* ModSubsystem = AFicsitRemoteMonitoring::Get(WorldContext->GetWorld());
	fgcheck(ModSubsystem);

	for (TSubclassOf<UFGSchematic> Schematic : Schematics) {

		TSharedPtr<FJsonObject> JSchematic = MakeShared<FJsonObject>();

		TArray<TSharedPtr<FJsonValue>> JRecipeArray;
		TArray<TSubclassOf<UFGRecipe>> Recipes;

		bool Purchased = false;
		bool HasUnlocks = false;
		bool LockedAny = false;
		bool LockedTutorial = false;
		bool LockedDependent = false;
		bool LockedPhase = false;
		bool Tutorial = false;
				
		ModSubsystem->SchematicToRecipes_BIE(WorldContext, Schematic, Recipes, Purchased, HasUnlocks, LockedAny, LockedTutorial, LockedDependent, LockedPhase, Tutorial);

		for (TSubclassOf<UFGRecipe> Recipe : Recipes) {

			TSharedPtr<FJsonObject> JRecipe = UFRM_Production::getRecipe(WorldContext, Recipe);
			JRecipeArray.Add(MakeShared<FJsonValueObject>(JRecipe));

		}

		FString SchematicType;

		switch (UFGSchematic::GetType(Schematic)) {
			case ESchematicType::EST_Alternate: SchematicType = TEXT("Alternate");
			case ESchematicType::EST_Cheat: SchematicType = TEXT("Cheat");
			case ESchematicType::EST_Custom: SchematicType = TEXT("Custom");
			case ESchematicType::EST_HardDrive: SchematicType = TEXT("Hard Drive");
			case ESchematicType::EST_MAM: SchematicType = TEXT("M.A.M.");
			case ESchematicType::EST_Milestone: SchematicType = TEXT("Milestone");
			case ESchematicType::EST_Prototype: SchematicType = TEXT("Prototype");
			case ESchematicType::EST_ResourceSink: SchematicType = TEXT("Resource Sink");
			case ESchematicType::EST_Story: SchematicType = TEXT("Story");
			case ESchematicType::EST_Tutorial: SchematicType = TEXT("Tutorial");
		}

		JSchematic->Values.Add("Name", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDisplayName(Schematic).ToString()));
		JSchematic->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Schematic->GetClass())));
		JSchematic->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(Schematic)));
		JSchematic->Values.Add("Type", MakeShared<FJsonValueString>(SchematicType));
		JSchematic->Values.Add("Recipes", MakeShared<FJsonValueArray>(JRecipeArray));
		JSchematic->Values.Add("HasUnlocks", MakeShared<FJsonValueBoolean>(HasUnlocks));
		JSchematic->Values.Add("Locked", MakeShared<FJsonValueBoolean>(LockedAny));
		JSchematic->Values.Add("Purchased", MakeShared<FJsonValueBoolean>(Purchased));
		JSchematic->Values.Add("DepLocked", MakeShared<FJsonValueBoolean>(LockedDependent));
		JSchematic->Values.Add("LockedTutorial", MakeShared<FJsonValueBoolean>(LockedTutorial));
		JSchematic->Values.Add("LockedPhase", MakeShared<FJsonValueBoolean>(LockedPhase));
		JSchematic->Values.Add("Tutorial", MakeShared<FJsonValueBoolean>(Tutorial));

		JSchematicsArray.Add(MakeShared<FJsonValueObject>(JSchematic));
	}

	return JSchematicsArray;

}
