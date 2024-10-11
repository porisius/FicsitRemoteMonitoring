// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include <cmath>
#include <typeinfo>
#include "JsonUtilities/Public/JsonObjectConverter.h"
#include "BlueprintJsonValue.h"

#include "BlueprintJsonObject.generated.h"


/**
 * A Class for handling Json in C++ as well as Blueprints. If you only want to use Json in C++, use the integrated Json module; use this plugin only if you want to export Json Objects to Blueprint
 */
UCLASS(Blueprintable)
class JSONBLUEPRINT_API UBlueprintJsonObject : public UObject
{
	GENERATED_BODY()

private:
	TSharedPtr<FJsonObject> mJsonObject;

	void SetSharedPointer(TSharedPtr<FJsonObject> JsonObject) {
		mJsonObject = JsonObject;
	}

public:
	UBlueprintJsonObject();

	// Converts the UBlueprintJsonObject to a shared pointer representing the Json object in C++
	TSharedPtr<FJsonObject> ToSharedPointer();

	// Converts any shared pointer that holds a Json object to a UBlueprintJsonObject so it can be exported to blueprints
	static UBlueprintJsonObject* FromSharedPointer(TSharedPtr<FJsonObject> JsonObject);

	// Blueprints
	// Creates an empty Json object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Create Object", Keywords = "Json Create Object", ShortTooltip = "Creates an empty Json Object"), Category = "Json|Object")
	static UBlueprintJsonObject* Create();

	// Parses a Json Object String ({"some": "example"})
	//
	// Returns false if the parsing failed, true and a String otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Parse Object String", Keywords = "Json Parse Object String", ShortTooltip = "Parses a Json Object String ({\"some\": \"example\"})"), Category = "Json|Object")
	static bool Parse(FString JsonString, UBlueprintJsonObject*& JsonObject);

	// Converts a Json Object to a single string
	//
	// If Pretty Print is true, the string will contain newlines and indention, otherwise it will be as condensed as possible
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Stringify Object", Keywords = "Json Stringify Object", AdvancedDisplay = "PrettyPrint", ShortTooltip = "Converts a Json Object to a single string"), Category = "Json|Object")
	FString Stringify(bool PrettyPrint = false);

	// Does the Json Object have this key (does not check for any type)?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Value", Keywords = "Json Has Value", ShortTooltip = "Does the Json Object have this key?"), Category = "Json|Object")
	bool HasValue(FString Key);

	// Gets a certain value from the Json Object without specifying it's type
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Value", Keywords = "Json Get Value", ShortTooltip = "Gets a certain value from the Json Object without specifying it's type"), Category = "Json|Object")
	bool GetValue(FString Key, UBlueprintJsonValue*& Value);

	// Sets a certain value without specifying its type (can be any)
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Value", Keywords = "Json Set Value", ShortTooltip = "Sets a certain value without specifying its type (can be any)"), Category = "Json|Object")
	UBlueprintJsonObject* SetValue(FString Key, UBlueprintJsonValue* Value);

	// Deletes a Json value from a Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Delete Value", Keywords = "Json Delete Value", ShortTooltip = "Deletes a Json value from a Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* DeleteValue(FString Key);
	
	// Does the Json Object have this key and is it a boolean?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Boolean", Keywords = "Json Has Boolean", ShortTooltip = "Does the Json Object have this key and is it a boolean?"), Category = "Json|Object")
	bool HasBoolean(FString Key);

	// Gets a certain boolean from the Json Object
	//
	// Returns false if the value does not exists or is not a boolean, otherwise true and the value
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Boolean", Keywords = "Json Get Boolean", ShortTooltip = "Gets a certain boolean from the Json Object"), Category = "Json|Object")
	bool GetBoolean(FString Key, bool& Value);

	// Sets a certain boolean as value in the Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Boolean", Keywords = "Json Set Boolean", ShortTooltip = "Sets a certain boolean as value in the Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* SetBoolean(FString Key, bool Value);


	// Does the Json Object have this key and is it an integer?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Integer", Keywords = "Json Has Integer", ShortTooltip = "Does the Json Object have this key and is it an integer?"), Category = "Json|Object")
	bool HasInteger(FString Key);

	// Gets a certain integer from the Json Object
	//
	// Returns false if the value does not exists or is not an integer, otherwise true and the value
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Integer", Keywords = "Json Get Integer", ShortTooltip = "Gets a certain integer from the Json Object"), Category = "Json|Object")
	bool GetInteger(FString Key, int32& Value);

	// Sets a certain integer as value in the Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Integer", Keywords = "Json Set Integer", ShortTooltip = "Sets a certain integer as value in the Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* SetInteger(FString Key, int32 Value);


	// Does the Json Object have this key and is it a float?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Float", Keywords = "Json Has Float", ShortTooltip = "Does the Json Object have this key and is it a float?"), Category = "Json|Object")
	bool HasFloat(FString Key);

	// Gets a certain float from the Json Object
	//
	// Returns false if the value does not exists or is not a float, otherwise true and the value
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Float", Keywords = "Json Get Float", ShortTooltip = "Gets a certain float from the Json Object"), Category = "Json|Object")
	bool GetFloat(FString Key, float& Value);

	// Sets a certain float as value in the Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Float", Keywords = "Json Set Float", ShortTooltip = "Sets a certain float as value in the Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* SetFloat(FString Key, float Value);


	// Does the Json Object have this key and is it a string?
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has String", Keywords = "Json Has String", ShortTooltip = "Does the Json Object have this key and is it a string?"), Category = "Json|Object")
	bool HasString(FString Key);

	// Gets a certain string from the Json Object
	//
	// Returns false if the value does not exists or is not a string, otherwise true and the value
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get String", Keywords = "Json Get String", ShortTooltip = "Gets a certain string from the Json Object"), Category = "Json|Object")
	bool GetString(FString Key, FString& Value);

	// Sets a certain string as value in the Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set String", Keywords = "Json Set String", ShortTooltip = "Sets a certain string as value in the Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* SetString(FString Key, FString Value);


	// Does the Json Object have this key and is it a Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Object", Keywords = "Json Has Object", ShortTooltip = "Does the Json Object have this key and is it another Json Object?"), Category = "Json|Object")
	bool HasObject(FString Key);

	// Gets a certain Json Object from the Json Object
	//
	// Returns false if the value does not exists or is not a Json Object, otherwise true and the Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Object", Keywords = "Json Get Object", ShortTooltip = "Gets a certain Json Object from the Json Object"), Category = "Json|Object")
	bool GetObject(FString Key, UBlueprintJsonObject*& Value);

	// Sets a Json Object as child of another Json Object
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Object", Keywords = "Json Set Object", ShortTooltip = "Sets a Json Object as child of another Json Object"), Category = "Json|Object")
	UBlueprintJsonObject* SetObject(FString Key, UBlueprintJsonObject* Value);


	// Does the Json Object have this key and is it a Json Array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Has Array", Keywords = "Json Has Array", ShortTooltip = "Does the Json Object have this key and is it a Json Array?"), Category = "Json|Object")
	bool HasArray(FString Key);

	// Gets a certain Json Array from the Json Object without specifying a type
	//
	// Returns false if the value does not exists or is not a Json Array, otherwise true and the Json Array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Array", Keywords = "Json Get Array", ShortTooltip = "Gets a certain Json Array from the Json Object without specifying a type"), Category = "Json|Object")
	bool GetArray(FString Key, TArray<UBlueprintJsonValue*>& Values);

	// Sets a certain Json Array as value in a Json Object without specifying a type
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Set Array", Keywords = "Json Set Array", ShortTooltip = "Sets a certain Json Array as value in a Json Object without specifying a type"), Category = "Json|Object")
	UBlueprintJsonObject* SetArray(FString Key, TArray<UBlueprintJsonValue*> Values);

	// BlueprintCallable function using Custom Thunk to support arbitrary struct conversion
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "JSON", meta = (CustomStructureParam = "Struct"))
	static UBlueprintJsonObject* StructToJsonObject(const UStruct* Struct);  // Placeholder

	// Declare the custom exec function (thunk)
	DECLARE_FUNCTION(execStructToJsonObject)
	{
		// Step into the struct parameter passed via Blueprint
		Stack.StepCompiledIn<FStructProperty>(nullptr); // Retrieve the struct
		FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
		void* StructPtr = Stack.MostRecentPropertyAddress;

		// Check for validity of the struct
		if (!StructProperty)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid struct parameter."));
			return;
		}

		// Finish processing the call
		P_FINISH;

		// Call the C++ conversion function to serialize the struct to JSON
		*(UBlueprintJsonObject**)RESULT_PARAM = StructToJson(StructProperty->Struct, StructPtr);
	}

	// Helper function to convert struct to JSON
	static UBlueprintJsonObject* StructToJson(const UStruct* StructDefinition, const void* StructData)
	{
		if (!StructDefinition || !StructData)
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid input to StructToJson."));
			return nullptr;
		}

		// Create a new JSON object
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		// Use FJsonObjectConverter to convert the struct to a JSON object
		if (FJsonObjectConverter::UStructToJsonObject(StructDefinition, StructData, JsonObject.ToSharedRef(), 0, 0))
		{
			// Create a UBlueprintJsonObject to wrap the JSON data
			UBlueprintJsonObject* BlueprintJsonObject = NewObject<UBlueprintJsonObject>();

			// Populate the UBlueprintJsonObject with the serialized JSON data
			BlueprintJsonObject->FromSharedPointer(JsonObject);

			return BlueprintJsonObject;
		}

		UE_LOG(LogTemp, Error, TEXT("Failed to serialize struct to JSON."));
		return nullptr;
	}

};
