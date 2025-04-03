#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FRM_Library.generated.h"

class UFGPowerInfoComponent;
class UFGFactoryConnectionComponent;
class UFGPipeConnectionComponent;
class UFGInventoryComponent;

UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Library : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TSharedPtr<FJsonObject> getActorJSON(AActor* Actor);
	static TSharedPtr<FJsonObject> getActorFactoryCompXYZ(UFGFactoryConnectionComponent* BeltPipe);
	static TSharedPtr<FJsonObject> getActorPipeXYZ(UFGPipeConnectionComponent* BeltPipe);
	static TSharedPtr<FJsonObject> getActorFeaturesJSON(AActor* Actor, FString DisplayName, FString TypeName);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> GetGroupedInventoryItems(const UFGInventoryComponent* Inventory);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks);
	static void GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems);
	static void GetGroupedInventoryItems(const UFGInventoryComponent* Inventory, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems);
	static TArray<TSharedPtr<FJsonValue>> GetInventoryJSON(const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items);
	static TArray<TSharedPtr<FJsonValue>> GetInventoryJSON(const TArray<FItemAmount>& Items);
	static TSharedPtr<FJsonObject> GetItemValueObject(const TSubclassOf<UFGItemDescriptor>& Item, const int Amount);
	static TSharedPtr<FJsonObject> GetResourceNodeJSON(AActor* Actor, const bool bIncludeFeatures = false);
	static TSharedPtr<FJsonObject> CreateBaseJsonObject(const UObject* Actor);
	static FString APItoJSON(TArray<TSharedPtr<FJsonValue>> JSONArray, UObject* WorldContext);
	static bool IsIntInRange(int32 Number, int32 LowerBound, int32 UpperBound);
	static TSharedPtr<FJsonObject> GetActorLineFeaturesJSON(FVector PointOne, FVector PointTwo, FString DisplayName, FString TypeName);

	static double SafeDivide_Double(double Numerator, double Denominator)
	{
		return (Denominator == 0.0) ? 0.0 : (Numerator / Denominator);
	}

	static float SafeDivide_Float(float Numerator, float Denominator)
	{
		return (Denominator == 0.0f) ? 0.0f : (Numerator / Denominator);
	}
	
	static TSharedPtr<FJsonValue> ConvertStringToFJsonValue(const FString& JsonString);
	static TSharedPtr<FJsonObject> getPowerConsumptionJSON(UFGPowerInfoComponent* powerInfo);
	static TSharedPtr<FJsonObject> ConvertVectorToFJsonObject(FVector JsonVector);
};
