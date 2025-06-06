#include "FactoryLibrary.h"

#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Templates/SharedPointer.h"
#include "../../RemoteMonitoringLibrary.h"
#include "FGInventoryLibrary.h"
#include "FGSchematicManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

struct FItemAmount;
class FJsonValue;
class AFGBuildable;
class AFGBuildableSubsystem;

TArray<TSharedPtr<FJsonValue>> UFactoryLibrary::getFactory_Helper(UObject* WorldContext, UClass* TypedBuildable)
{
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);
	TArray<TSharedPtr<FJsonValue>> JFactoryArray;

	//UE_LOGFMT(LogFRMAPI, Warning, "Initial variables configured, executing getProdStats");

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		TSharedPtr<FJsonObject> JFactory = CreateBuildableBaseJsonObject(Buildable);
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		float Productivity = 0;

		//UE_LOGFMT(LogFRMAPI, Warning, "Loading FGBuildable {Manufacturer} to get data.", UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetClass()));

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Manufacturer->GetCurrentRecipe();
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			Productivity = Manufacturer->GetProductivity();
			auto ProductionBoost = Manufacturer->mCurrentProductionBoost;
						
			//UE_LOGFMT(LogFRMAPI, Warning, "Loading FGRecipe {Recipe} to get data.", UKismetSystemLibrary::GetClassDisplayName(CurrentRecipe->GetClass()));

			for (FItemAmount Product : CurrentRecipe.GetDefaultObject()->GetProducts()) {
				TSharedPtr<FJsonObject> JProduct = GetItemValueObject(Product);
				
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential * ProductionBoost;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential * ProductionBoost;

				JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(100 * UKismetMathLibrary::SafeDivide(CurrentProd, MaxProd)));

				JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			};

			for (FItemAmount Ingredient : CurrentRecipe.GetDefaultObject()->GetIngredients()) {
				TSharedPtr<FJsonObject> JIngredients = GetItemValueObject(Ingredient);

				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredient.Amount, UFGItemDescriptor::GetForm(Ingredient.ItemClass));
				auto CurrentConsumed = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxConsumed = RecipeAmount * ProdCycle * CurrentPotential;

				JIngredients->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(CurrentConsumed));
				JIngredients->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(MaxConsumed));
				JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(100 * UKismetMathLibrary::SafeDivide(CurrentConsumed, MaxConsumed)));

				JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
			};
			
		}
		else {
			TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

			JProduct->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(0));

			JIngredients->Values.Add("Name", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>(TEXT("Unassigned")));
			JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("CurrentConsumed", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("MaxConsumed", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(0));

			JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
		};

		FString RecipeName = TEXT("Unassigned");
		
		if (Manufacturer->GetCurrentRecipe() != nullptr)
		{
			RecipeName = UFGRecipe::GetRecipeName(Manufacturer->GetCurrentRecipe()).ToString();
		}	
		
		JFactory->Values.Add("Recipe", MakeShared<FJsonValueString>(RecipeName));
		JFactory->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Manufacturer->GetCurrentRecipe())));
		JFactory->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JFactory->Values.Add("ingredients", MakeShared<FJsonValueArray>(JIngredientsArray));
		JFactory->Values.Add("Productivity", MakeShared<FJsonValueNumber>(Productivity * 100));
		JFactory->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Manufacturer->GetCurrentPotential() * 100));
		JFactory->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Manufacturer->IsConfigured()));
		JFactory->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Manufacturer->IsProducing()));
		JFactory->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Manufacturer->IsProductionPaused()));
		JFactory->Values.Add("PowerInfo", MakeShared<FJsonValueObject>(getPowerConsumptionJSON(Manufacturer->GetPowerInfo())));
		JFactory->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(Cast<AActor>(Manufacturer), Manufacturer->mDisplayName.ToString(), Manufacturer->mDisplayName.ToString())));

		JFactoryArray.Add(MakeShared<FJsonValueObject>(JFactory));
	};

	return JFactoryArray;
}
