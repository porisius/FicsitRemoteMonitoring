// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintJsonValue.h"

UBlueprintJsonValue::UBlueprintJsonValue()
{}

TSharedPtr<FJsonValue> UBlueprintJsonValue::ToSharedPointer() {
	return mJsonValue;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromSharedPointer(TSharedPtr<FJsonValue> JsonValue) {
	UBlueprintJsonValue* Value = NewObject<UBlueprintJsonValue>();
	Value->SetSharedPointer(JsonValue);
	return Value;
}

bool UBlueprintJsonValue::ParseArray(FString JsonString, TArray<UBlueprintJsonValue*>& Values) {
	TSharedPtr<FJsonValue> Value;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, Value))
		return false;

	const TArray<TSharedPtr<FJsonValue>>* vals;
	bool success = Value->TryGetArray(vals);
	if (!success)
		return false;

	for (TSharedPtr<FJsonValue> val : *vals) {
		Values.Add(UBlueprintJsonValue::FromSharedPointer(val));
	}

	return true;
}

FString UBlueprintJsonValue::StringifyArray(TArray<UBlueprintJsonValue*> Values, bool PrettyPrint) {
	TArray<TSharedPtr<FJsonValue>> vals;
	for (UBlueprintJsonValue* Value : Values) {
		if (Value != nullptr)
			vals.Add(Value->ToSharedPointer());
	}

	FString JsonString;
	if (PrettyPrint) {
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(vals, Writer);
	}
	else {
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(vals, Writer);
	}
	return JsonString;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromBoolean(bool Value) {
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueBoolean(Value));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromInteger(int32 Value) {
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueNumber(Value));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromFloat(float Value) {
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueNumber(Value));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromString(FString Value) {
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueString(Value));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromObject(UBlueprintJsonObject* Value) {
	if (Value == nullptr)
		return NewObject<UBlueprintJsonValue>();

	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueObject(Value->ToSharedPointer()));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

UBlueprintJsonValue* UBlueprintJsonValue::FromArray(TArray<UBlueprintJsonValue*> Value) {
	TArray<TSharedPtr<FJsonValue>> vals;
	for (UBlueprintJsonValue* Val : Value) {
		if (Val != nullptr)
			vals.Add(Val->ToSharedPointer());
	}
	TSharedPtr<FJsonValue> JsonValue = MakeShareable(new FJsonValueArray(vals));
	UBlueprintJsonValue* val = NewObject<UBlueprintJsonValue>();
	val->SetSharedPointer(JsonValue);
	return val;
}

bool UBlueprintJsonValue::ToBoolean(bool& Value) {
	return mJsonValue->TryGetBool(Value);
}

bool UBlueprintJsonValue::ToInteger(int32& Value) {
	return mJsonValue->TryGetNumber(Value);
}

bool UBlueprintJsonValue::ToFloat(float& Value) {
	double Number;
	bool success = mJsonValue->TryGetNumber(Number);
	if (success)
		Value = Number;
	return success;
}

bool UBlueprintJsonValue::ToString(FString& Value) {
	return mJsonValue->TryGetString(Value);
}

bool UBlueprintJsonValue::ToObject(UBlueprintJsonObject*& Value) {
	const TSharedPtr<FJsonObject>* JsonObject;
	bool success = mJsonValue->TryGetObject(JsonObject);
	if (success)
		Value = UBlueprintJsonObject::FromSharedPointer(*JsonObject);
	return success;
}

bool UBlueprintJsonValue::ToArray(TArray<UBlueprintJsonValue*>& Values) {
	const TArray<TSharedPtr<FJsonValue>>* vals;
	bool success = mJsonValue->TryGetArray(vals);
	if (!success)
		return false;

	for (TSharedPtr<FJsonValue> Value : *vals) {
		Values.Add(UBlueprintJsonValue::FromSharedPointer(Value));
	}

	return true;
}