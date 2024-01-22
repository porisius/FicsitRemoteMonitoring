#include "FRM_Factory.h"
#include <NiagaraPerfBaseline.h>

FString UFRM_Factory::getFactory(UObject* WorldContext, UClass* TypedBuildable)
{
	
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	TArray<AFGBuildable*> Buildables;
	BuildableSubsystem->GetTypedBuildable(TypedBuildable, Buildables);
	TArray<TSharedPtr<FJsonValue>> JFactoryArray;

	for (AFGBuildable* Buildable : Buildables) {

		AFGBuildableManufacturer* Manufacturer = Cast<AFGBuildableManufacturer>(Buildable);

		TSharedPtr<FJsonObject> JFactory = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> JProductArray;
		TArray<TSharedPtr<FJsonValue>> JIngredientsArray;

		if (IsValid(Manufacturer->GetCurrentRecipe())) {
			auto CurrentRecipe = Cast<UFGRecipe>(Manufacturer->GetCurrentRecipe());
			auto ProdCycle = 60 / Manufacturer->GetProductionCycleTimeForRecipe(Manufacturer->GetCurrentRecipe());
			auto CurrentPotential = Manufacturer->GetCurrentPotential();
			auto Productivity = Manufacturer->GetProductivity();
						
			for (FItemAmount Product : CurrentRecipe->GetProducts()) {
				TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
				
				auto Amount = Manufacturer->GetOutputInventory()->GetNumItems(Product.ItemClass);
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Product.Amount, UFGItemDescriptor::GetForm(Product.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential;

				JProduct->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Product.ItemClass).ToString()));
				JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>((Product.ItemClass)->GetDefaultObjectName().ToString()));
				JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(CurrentProd / MaxProd));

				JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			};

			for (FItemAmount Ingredients : CurrentRecipe->GetIngredients()) {
				TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

				auto Amount = Manufacturer->GetOutputInventory()->GetNumItems(Ingredients.ItemClass);
				auto RecipeAmount = UFGInventoryLibrary::GetAmountConvertedByForm(Ingredients.Amount, UFGItemDescriptor::GetForm(Ingredients.ItemClass));
				auto CurrentProd = RecipeAmount * ProdCycle * Productivity * CurrentPotential;
				auto MaxProd = RecipeAmount * ProdCycle * CurrentPotential;

				JIngredients->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Ingredients.ItemClass).ToString()));
				JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>((Ingredients.ItemClass)->GetDefaultObjectName().ToString()));
				JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(Amount));
				JIngredients->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(CurrentProd));
				JIngredients->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(MaxProd));
				JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(CurrentProd / MaxProd));

				JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
			};
			
		}
		else {
			TSharedPtr<FJsonObject> JProduct = MakeShared<FJsonObject>();
			TSharedPtr<FJsonObject> JIngredients = MakeShared<FJsonObject>();

			JProduct->Values.Add("Name", MakeShared<FJsonValueString>("Unassigned"));
			JProduct->Values.Add("ClassName", MakeShared<FJsonValueString>("Unassigned"));
			JProduct->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(0));
			JProduct->Values.Add("ProdPercent", MakeShared<FJsonValueNumber>(0));

			JIngredients->Values.Add("Name", MakeShared<FJsonValueString>("Unassigned"));
			JIngredients->Values.Add("ClassName", MakeShared<FJsonValueString>("Unassigned"));
			JIngredients->Values.Add("Amount", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("CurrentProd", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("MaxProd", MakeShared<FJsonValueNumber>(0));
			JIngredients->Values.Add("ConsPercent", MakeShared<FJsonValueNumber>(0));

			JProductArray.Add(MakeShared<FJsonValueObject>(JProduct));
			JIngredientsArray.Add(MakeShared<FJsonValueObject>(JIngredients));
		};
		
		TArray<TSharedPtr<FJsonValue>> JCircuitArray;

		TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();
		JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(Manufacturer->GetPowerInfo()->GetPowerCircuit()->GetCircuitGroupID()));
		JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(Manufacturer->GetPowerInfo()->GetPowerCircuit()->mPowerConsumed));
		JCircuitArray.Add(MakeShared<FJsonValueObject>(JCircuit));

		JFactory->Values.Add("Name", MakeShared<FJsonValueString>(Manufacturer->mDisplayName.ToString()));
		JFactory->Values.Add("ClassName", MakeShared<FJsonValueString>(Manufacturer->GetClass()->GetName()));
		JFactory->Values.Add("location", MakeShared<FJsonValueObject>(UFRM_Library::getActorJSON(Cast<AActor>(Manufacturer))));
		JFactory->Values.Add("Recipe", MakeShared<FJsonValueString>(Manufacturer->GetCurrentRecipe()->GetFName().ToString()));
		JFactory->Values.Add("RecipeClassName", MakeShared<FJsonValueString>(Manufacturer->GetCurrentRecipe()->GetClass()->GetName()));
		JFactory->Values.Add("production", MakeShared<FJsonValueArray>(JProductArray));
		JFactory->Values.Add("ingredients", MakeShared<FJsonValueArray>(JIngredientsArray));
		JFactory->Values.Add("ManuSpeed", MakeShared<FJsonValueNumber>(Manufacturer->GetManufacturingSpeed() * 100));
		JFactory->Values.Add("IsConfigured", MakeShared<FJsonValueBoolean>(Manufacturer->IsConfigured()));
		JFactory->Values.Add("IsProducing", MakeShared<FJsonValueBoolean>(Manufacturer->IsProducing()));
		JFactory->Values.Add("IsPaused", MakeShared<FJsonValueBoolean>(Manufacturer->IsProductionPaused()));
		JFactory->Values.Add("PowerInfo", MakeShared<FJsonValueArray>(JCircuitArray));
		JFactory->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(Manufacturer->GetPowerInfo()->GetPowerCircuit()->GetCircuitGroupID()));
		JFactory->Values.Add("features", MakeShared<FJsonValueObject>(UFRM_Library::getActorFeaturesJSON(Cast<AActor>(Manufacturer), Manufacturer->mDisplayName.ToString())));

		JFactoryArray.Add(MakeShared<FJsonValueObject>(JFactory));
	};
	FString Write;
	const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
	FJsonSerializer::Serialize(JFactoryArray, JsonWriter);

	return Write;
};