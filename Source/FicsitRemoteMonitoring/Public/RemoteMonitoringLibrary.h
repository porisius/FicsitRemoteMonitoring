#pragma once

#include "CoreMinimal.h"
#include "FGBlueprintFunctionLibrary.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "FRM_RequestData.h"
#include "SessionSettingsManager.h"
#include "Components/SplineComponent.h"
#include "RemoteMonitoringLibrary.generated.h"

struct FSplinePointData;
class AFicsitRemoteMonitoring;
class UFGPowerInfoComponent;
class UFGFactoryConnectionComponent;
class UFGPipeConnectionComponent;
class UFGPipeConnectionComponentBase;
class UFGInventoryComponent;

UCLASS()
class FICSITREMOTEMONITORING_API URemoteMonitoringLibrary : public UFGBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static TSharedPtr<FJsonObject> getActorJSON(AActor* Actor);
	static TSharedPtr<FJsonObject> getActorFactoryCompXYZ(AFGBuildable* BeltPipe, UFGFactoryConnectionComponent* ConnectionComponent);
	static TSharedPtr<FJsonObject> getActorPipeXYZ(AFGBuildable* BeltPipe, UFGPipeConnectionComponent* ConnectionComponent);
	static TSharedPtr<FJsonObject> getActorHyperXYZ(AFGBuildable* HyperTube, UFGPipeConnectionComponentBase* ConnectionComponent);
	static TSharedPtr<FJsonObject> getActorFeaturesJSON(AActor* Actor, FString DisplayName, FString TypeName);
	static TArray<TSharedPtr<FJsonValue>> SplineToJSON(USplineComponent* SplineComp, float SampleDistance);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> GetGroupedInventoryItems(const UFGInventoryComponent* Inventory);
	static TMap<TSubclassOf<UFGItemDescriptor>, int32> GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks);
	static void GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems);
	static void GetGroupedInventoryItems(const UFGInventoryComponent* Inventory, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems);
	static TArray<TSharedPtr<FJsonValue>> GetInventoryJSON(const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, float StackSizeMuliplier = 1.0f);
	static TArray<TSharedPtr<FJsonValue>> GetInventoryJSON(const TArray<FItemAmount>& Items, float StackSizeMuliplier = 1.0f);
	static TSharedPtr<FJsonObject> GetSchematicJson(AFicsitRemoteMonitoring* ModSubsystem, UObject* WorldContext, TSubclassOf<UFGSchematic> Schematic);
	static TSharedPtr<FJsonObject> GetItemValueObject(const TSubclassOf<UFGItemDescriptor>& Item, const int Amount, float StackSizeMuliplier = 1.0f);
	static TSharedPtr<FJsonObject> GetItemValueObject(const FItemAmount Item, float StackSizeMuliplier = 1.0f);
	static TSharedPtr<FJsonObject> GetItemValueObject(const FInventoryStack& Item, float StackSizeMuliplier = 1.0f);
	static TSharedPtr<FJsonObject> GetResourceNodeJSON(AActor* Actor, const bool bIncludeFeatures = false);
	static TSharedPtr<FJsonObject> CreateBaseJsonObject(const UObject* Actor);
	static TSharedPtr<FJsonObject> CreateBuildableBaseJsonObject(AFGBuildable* Buildable);
	static FString APItoJSON(TArray<TSharedPtr<FJsonValue>> JSONArray, UObject* WorldContext);
	static bool IsIntInRange(int32 Number, int32 LowerBound, int32 UpperBound);
	static TSharedPtr<FJsonObject> GetActorLineFeaturesJSON(FVector PointOne, FVector PointTwo, FString DisplayName, FString TypeName);
	static void GetOverclockingItemsFromInventory(const UFGInventoryComponent* Inventory, int32& Somersloops, int32& PowerShards);

	static FString GetPlayerName(AFGCharacterPlayer* Character);

	static double SafeDivide_Double(double Numerator, double Denominator)
	{
		return (Denominator == 0.0) ? 0.0 : (Numerator / Denominator);
	}

	static float SafeDivide_Float(float Numerator, float Denominator)
	{
		return (Denominator == 0.0f) ? 0.0f : (Numerator / Denominator);
	}
	
	static TSharedPtr<FJsonValue> ConvertStringToFJsonValue(const FString& JsonString);
	static TSharedPtr<FJsonObject> FBoxToJson(AFGBuildable* Buildable, const FBox& Box);
	static TSharedPtr<FJsonObject> ColorSlotToJson(AFGBuildable* Buildable);
	static TSharedPtr<FJsonObject> getPowerConsumptionJSON(UFGPowerInfoComponent* powerInfo);
	static TSharedPtr<FJsonObject> ConvertVectorToFJsonObject(FVector JsonVector);
	static TSharedPtr<FJsonObject> ConvertVectorToFJsonObject(FIntVector JsonVector);
	static TSharedPtr<FJsonValueArray> GetSplineVector(TArray<FSplinePointData>);
};
