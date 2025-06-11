#include "Session.h"

#include "FGBlueprintSubsystem.h"
#include "FGBuildableGeneratorFuel.h"
#include "FGBuildableManufacturer.h"
#include "FGBuildableResourceExtractor.h"
#include "FGTimeSubsystem.h"
#include "FGGameState.h"
#include "FGResourceSinkSettings.h"
#include "Runtime/Engine/Classes/Engine/GameInstance.h"
#include "ModLoadingLibrary.h"
#include "OnlineSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"


void USession::getSessionInfo(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	const auto GameState = WorldContext->GetWorld()->GetGameState<AFGGameState>();
	const AFGTimeOfDaySubsystem* TimeOfDaySubSystem = AFGTimeOfDaySubsystem::Get(WorldContext);

	const auto PlayDuration = GameState->GetTotalPlayDuration();

	TSharedPtr<FJsonObject> JSessionInfo = MakeShared<FJsonObject>();
	JSessionInfo->Values.Add("SessionName", MakeShared<FJsonValueString>(GameState->GetSessionName()));
	JSessionInfo->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(WorldContext->GetWorld()->IsPaused()));
	JSessionInfo->Values.Add("DayLength", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetDayLength()));
	JSessionInfo->Values.Add("NightLength", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetNightLength()));
	JSessionInfo->Values.Add("PassedDays", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetPassedDays()));
	JSessionInfo->Values.Add("NumberOfDaysSinceLastDeath", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetNumberOfDaysSinceLastDeath()));
	JSessionInfo->Values.Add("Hours", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetHours()));
	JSessionInfo->Values.Add("Minutes", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetMinutes()));
	JSessionInfo->Values.Add("Seconds", MakeShared<FJsonValueNumber>(TimeOfDaySubSystem->GetSeconds()));
	JSessionInfo->Values.Add("IsDay", MakeShared<FJsonValueBoolean>(TimeOfDaySubSystem->IsDay()));
	JSessionInfo->Values.Add("TotalPlayDuration", MakeShared<FJsonValueNumber>(PlayDuration));
	JSessionInfo->Values.Add("TotalPlayDurationText", MakeShared<FJsonValueString>(SecondsToTimeString(PlayDuration)));
	
	OutJsonArray.Add(MakeShared<FJsonValueObject>(JSessionInfo));

}

void USession::getModList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	const UGameInstance* GameInstance = WorldContext->GetWorld()->GetGameInstance();
	UModLoadingLibrary* ModLoadingLibrary = GameInstance->GetSubsystem<UModLoadingLibrary>();
	TArray<FModInfo> ModInfos = ModLoadingLibrary->GetLoadedMods();

	for (FModInfo ModInfo : ModInfos) {

		TSharedPtr<FJsonObject> JModInfos = MakeShared<FJsonObject>();

		JModInfos->Values.Add("Name", MakeShared<FJsonValueString>(ModInfo.FriendlyName));
		JModInfos->Values.Add("SMRName", MakeShared<FJsonValueString>(ModInfo.Name));
		JModInfos->Values.Add("Version", MakeShared<FJsonValueString>(ModInfo.Version.ToString()));
		JModInfos->Values.Add("Description", MakeShared<FJsonValueString>(ModInfo.Description));
		JModInfos->Values.Add("DocsURL", MakeShared<FJsonValueString>(ModInfo.DocsURL));
		JModInfos->Values.Add("SupportURL", MakeShared<FJsonValueString>(ModInfo.SupportURL));
		JModInfos->Values.Add("CreatedBy", MakeShared<FJsonValueString>(ModInfo.CreatedBy));
		JModInfos->Values.Add("RemoteVersionRange", MakeShared<FJsonValueString>(ModInfo.RemoteVersionRange.ToString()));
		JModInfos->Values.Add("RequiredOnRemote", MakeShared<FJsonValueBoolean>(ModInfo.bRequiredOnRemote));
		
		OutJsonArray.Add(MakeShared<FJsonValueObject>(JModInfos));

	}
}

void USession::getSinkList(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

	TArray<FResourceSinkPointsData*> SinkRows;
	UDataTable* SinkTable = UFGResourceSinkSettings::GetPointsDataTable();

	SinkTable->GetAllRows<FResourceSinkPointsData>("", SinkRows);

	for (FResourceSinkPointsData* SinkRow : SinkRows) {

		TSharedPtr<FJsonObject> JSinkRow = MakeShared<FJsonObject>();
		int32 SinkPoints = SinkRow->Points;
		int32 SinkOverridden = SinkRow->OverriddenResourceSinkPoints;

		if (SinkPoints == 0 && SinkOverridden == 0) {
			continue;
		}

		JSinkRow->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(SinkRow->ItemClass).ToString()));
		JSinkRow->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(SinkRow->ItemClass)));
		JSinkRow->Values.Add("Points", MakeShared<FJsonValueNumber>(SinkPoints));
		JSinkRow->Values.Add("PointsOverride", MakeShared<FJsonValueNumber>(SinkOverridden));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JSinkRow));
	}
}

TArray<TSharedPtr<FJsonValue>> USession::getResourceSink_Helper(UObject* WorldContext, EResourceSinkTrack ResourceSinkTrack) {

	TArray<TSharedPtr<FJsonValue>> JResourceSinkArray;
	TSharedPtr<FJsonObject> JResourceSink = MakeShared<FJsonObject>();
	FString SinkName;

	AFGResourceSinkSubsystem* ResourceSinkSubsystem = AFGResourceSinkSubsystem::Get(WorldContext);

	switch (ResourceSinkTrack) {
		case EResourceSinkTrack::RST_Default		: SinkName = "Resource";
			break;
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
	JResourceSink->Values.Add("PointsToCoupon", MakeShared<FJsonValueNumber>(ResourceSinkSubsystem->GetNumPointsToNextCoupon(ResourceSinkTrack)));

	JResourceSinkArray.Add(MakeShared<FJsonValueObject>(JResourceSink));

	return JResourceSinkArray;
}

void USession::getProdStats(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray) {

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
			auto ProductionBoost = Manufacturer->mCurrentProductionBoost;
			
			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {

				auto ItemClass = Product.ItemClass;
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
		
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential * ProductionBoost;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential * ProductionBoost;
								
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
		if (ResourceClass != nullptr) {
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
		}
	};

	TArray<AFGBuildableGeneratorFuel*> GeneratorBuildables;
	BuildableSubsystem->GetTypedBuildable< AFGBuildableGeneratorFuel>(GeneratorBuildables);
	// Power Generator Building Production Stats
	for (AFGBuildableGeneratorFuel* Generator : GeneratorBuildables) {

		TSubclassOf<UFGItemDescriptor> FuelItemClass = Generator->GetCurrentFuelClass();
		float EnergyValue = UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetEnergyValue(FuelItemClass), UFGItemDescriptor::GetForm(FuelItemClass));
		float ProductionCapacity = Generator->GetCurrentPotential();
		float Productivity = Generator->GetProductivity();
		
		float MaxFuelConsumption = 60 * (SafeDivide_Float(ProductionCapacity, EnergyValue));
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
	GetAllDescriptorsSorted(WorldContext->GetWorld(), ClassNames);

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
			JProductionStats->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>((100 * (SafeDivide_Float(Produced, MaxProduced)))));
			JProductionStats->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>((100 * (SafeDivide_Float(Consumption, MaxConsumption)))));
			JProductionStats->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(Produced));
			JProductionStats->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProduced));
			JProductionStats->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(Consumption));
			JProductionStats->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumption));
			JProductionStats->Values.Add("Type", MakeShared<FJsonValueString>(FormString));

			OutJsonArray.Add(MakeShared<FJsonValueObject>(JProductionStats));
		};
	};
};

void USession::getUObjectCount(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	TSharedPtr<FJsonObject> JResponse = MakeShared<FJsonObject>();

	int32 UObjectCount = GUObjectArray.GetObjectArrayNumMinusAvailable();
	int32 UObjectCapacity = GUObjectArray.GetObjectArrayCapacity();

	JResponse->Values.Add("UObjectCount", MakeShared<FJsonValueNumber>(UObjectCount));
	JResponse->Values.Add("UObjectCapacity", MakeShared<FJsonValueNumber>(UObjectCapacity));
	OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
}

void USession::getBlueprints(UObject* WorldContext, FRequestData RequestData, TArray<TSharedPtr<FJsonValue>>& OutJsonArray)
{
	AFGBlueprintSubsystem* BlueprintSubsystem = AFGBlueprintSubsystem::GetBlueprintSubsystem(WorldContext);

	TArray<UFGBlueprintDescriptor*> BlueprintDescriptors;
	BlueprintSubsystem->GetBlueprintDescriptors(BlueprintDescriptors, WorldContext);

	for (UFGBlueprintDescriptor* BlueprintDescriptor : BlueprintDescriptors)
	{

		TSharedPtr<FJsonObject> JResponse = CreateBaseJsonObject(BlueprintDescriptor);
		
		TArray<FItemAmount> BlueprintCost;
		BlueprintDescriptor->GetBlueprintCost(BlueprintCost);
		
		FIntVector BlueprintDimensions = BlueprintDescriptor->GetDimensionsOnInstance();

		FBlueprintRecord BlueprintRecord = BlueprintDescriptor->GetDescriptorAsRecord();
	
		//FText BlueprintCategory = BlueprintRecord.Category->GetCategoryNameFromInstance();
		//FText BlueprintSubCategory = BlueprintRecord.SubCategory->GetCategoryNameFromInstance();
		
		//TArray<FLocalUserNetIdBundle> LocalUserNetIdBundle = BlueprintDescriptor->GetLastEditedBy();

		JResponse->Values.Add("Name", MakeShared<FJsonValueString>(BlueprintRecord.BlueprintName));
		JResponse->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(BlueprintDescriptor->GetClass())));
		JResponse->Values.Add("Description", MakeShared<FJsonValueString>(BlueprintRecord.BlueprintDescription));
		JResponse->Values.Add("Dimensions", MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(BlueprintDimensions)));
		//JResponse->Values.Add("Category", MakeShared<FJsonValueString>(BlueprintCategory.ToString()));
		//JResponse->Values.Add("SubCategory", MakeShared<FJsonValueString>(BlueprintSubCategory.ToString()));
		JResponse->Values.Add("BlueprintCost", MakeShared<FJsonValueArray>(GetInventoryJSON(BlueprintCost)));

		OutJsonArray.Add(MakeShared<FJsonValueObject>(JResponse));
	}
}
