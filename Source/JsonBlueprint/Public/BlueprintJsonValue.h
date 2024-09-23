// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"

class UBlueprintJsonObject;

#include "BlueprintJsonValue.generated.h"

/**
 * Json Values, as they might occur in arrays or generally values without specific type.
 */
UCLASS(Blueprintable)
class JSONBLUEPRINT_API UBlueprintJsonValue : public UObject
{
	GENERATED_BODY()

private:
	TSharedPtr<FJsonValue> mJsonValue;

	void SetSharedPointer(TSharedPtr<FJsonValue> JsonValue) {
		mJsonValue = JsonValue;
	}

public:
	UBlueprintJsonValue();

	// Converts the UBlueprintJsonValue to a shared pointer representing the Json value in C++
	TSharedPtr<FJsonValue> ToSharedPointer();

	// Converts any shared pointer that holds a Json value to a UBlueprintJsonValue so it can be exported to blueprints. Note, that blueprint Json values do not know a type, so have to be re-interpreted in blueprint
	static UBlueprintJsonValue* FromSharedPointer(TSharedPtr<FJsonValue> JsonValue);

	// Blueprints

	// Parses a Json Array String (["some", "example"])
	//
	// Returns false if the parsing failed, true and a String otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Parse Array String", Keywords = "Json Parse Array String", ShortTooltip = "Parses a Json Array String ([\"some\", \"example\"])"), Category = "Json|Value")
	static bool ParseArray(FString JsonString, TArray<UBlueprintJsonValue*>& Values);

	// Converts a Json Array to a single string
	//
	// If Pretty Print is true, the string will contain newlines and indention, otherwise it will be as condensed as possible
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Stringify Array", Keywords = "Json Stringify Array", AdvancedDisplay = "PrettyPrint", ShortTooltip = "Converts a Json Array to a single string"), Category = "Json|Value")
	static FString StringifyArray(TArray<UBlueprintJsonValue*> Values, bool PrettyPrint = false);


	// Creates a Json value from a boolean
	//
	// Casts the boolean to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From Boolean", Keywords = "Json From Boolean", ShortTooltip = "Creates a Json value from a boolean"), Category = "Json|Value")
	static UBlueprintJsonValue* FromBoolean(bool Value);

	// Creates a Json value from an integer
	//
	// Casts the integer to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From Integer", Keywords = "Json From Integer", ShortTooltip = "Creates a Json value from an integer"), Category = "Json|Value")
	static UBlueprintJsonValue* FromInteger(int32 Value);

	// Creates a Json value from a float
	//
	// Casts the float to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From Float", Keywords = "Json From Float", ShortTooltip = "Creates a Json value from a float"), Category = "Json|Value")
	static UBlueprintJsonValue* FromFloat(float Value);

	// Creates a Json value from a string
	//
	// Casts the string to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From String", Keywords = "Json From String", ShortTooltip = "Creates a Json value from a string"), Category = "Json|Value")
	static UBlueprintJsonValue* FromString(FString Value);

	// Creates a Json value from a Json Object
	//
	// Casts the Json Object to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From Object", Keywords = "Json From Object", ShortTooltip = "Creates a Json value from a Object"), Category = "Json|Value")
	static UBlueprintJsonValue* FromObject(UBlueprintJsonObject* Value);

	// Creates a Json value from a Json Array
	//
	// Casts the Json Array to a Json value so it can be added to an array
	UFUNCTION(BlueprintPure, meta = (DisplayName = "From Array", Keywords = "Json From Array", ShortTooltip = "Creates a Json value from a Array"), Category = "Json|Value")
	static UBlueprintJsonValue* FromArray(TArray<UBlueprintJsonValue*> Value);

	
	// Casts the Json value to a boolean
	//
	// Returns false if the conversion fails, true and the boolean otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Boolean", Keywords = "Json To Boolean", ShortTooltip = "Casts the Json value to a boolean"), Category = "Json|Value")
	bool ToBoolean(bool& Value);

	// Casts the Json value to an integer
	//
	// Returns false if the conversion fails, true and the integer otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Integer", Keywords = "Json To Integer", ShortTooltip = "Casts the Json value to an integer"), Category = "Json|Value")
	bool ToInteger(int32& Value);

	// Casts the Json value to a float
	//
	// Returns false if the conversion fails, true and the float otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Float", Keywords = "Json To Float", ShortTooltip = "Casts the Json value to a float"), Category = "Json|Value")
	bool ToFloat(float& Value);

	// Casts the Json value to a string
	//
	// Returns false if the conversion fails, true and the string otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To String", Keywords = "Json To String", ShortTooltip = "Casts the Json value to a string"), Category = "Json|Value")
	bool ToString(FString& Value);

	// Casts the Json value to a Json Object
	//
	// Returns false if the conversion fails, true and the Json Object otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Object", Keywords = "Json To Object", ShortTooltip = "Casts the Json value to a Json Array"), Category = "Json|Value")
	bool ToObject(UBlueprintJsonObject*& Value);

	// Casts the Json value to a Json Array
	//
	// Returns false if the conversion fails, true and the Json Array otherwise
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To Array", Keywords = "Json To Array", ShortTooltip = "Casts the Json value to a Json Object"), Category = "Json|Value")
	bool ToArray(TArray<UBlueprintJsonValue*>& Values);
};
