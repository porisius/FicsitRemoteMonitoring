// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FRM_Production.generated.h"

enum class EResourceSinkTrack : uint8;
class UFGRecipe;

/**
 * 
 */
UCLASS()
class FICSITREMOTEMONITORING_API UFRM_Production : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static TArray<TSharedPtr<FJsonValue>> getProdStats(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getSinkList(UObject* WorldContext);
	static TArray<TSharedPtr<FJsonValue>> getResourceSink(UObject* WorldContext, EResourceSinkTrack ResourceSinkTrack);
	static TArray<TSharedPtr<FJsonValue>> getRecipes(UObject* WorldContext);
	static TSharedPtr<FJsonObject> getRecipe(UObject* WorldContext, TSubclassOf<UFGRecipe> Recipe);
	static TArray<TSharedPtr<FJsonValue>> getSchematics(UObject* WorldContext);

	friend class AFGRecipeManager;
	friend class AFGSchematicManager;
	friend class AFGBuildableFactory;
};
