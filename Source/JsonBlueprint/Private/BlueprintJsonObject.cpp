// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintJsonObject.h"

UBlueprintJsonObject::UBlueprintJsonObject()
{
	mJsonObject = MakeShareable(new FJsonObject);
}

TSharedPtr<FJsonObject> UBlueprintJsonObject::ToSharedPointer() {
	return mJsonObject;
}

UBlueprintJsonObject* UBlueprintJsonObject::FromSharedPointer(TSharedPtr<FJsonObject> JsonObject) {
	UBlueprintJsonObject* Object = NewObject<UBlueprintJsonObject>();
	Object->SetSharedPointer(JsonObject);
	return Object;
}

// Blueprint

UBlueprintJsonObject* UBlueprintJsonObject::Create() {
	return NewObject<UBlueprintJsonObject>();
}

bool UBlueprintJsonObject::Parse(FString JsonString, UBlueprintJsonObject*& JsonObject) {
	TSharedPtr<FJsonObject> Object;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, Object))
		return false;

	JsonObject = UBlueprintJsonObject::FromSharedPointer(Object);
	return true;
}

FString UBlueprintJsonObject::Stringify(bool PrettyPrint) {
	FString JsonString;
	if (PrettyPrint) {
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(mJsonObject.ToSharedRef(), Writer);
	}
	else {
		TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
		FJsonSerializer::Serialize(mJsonObject.ToSharedRef(), Writer);
	}
	return JsonString;
}

bool UBlueprintJsonObject::HasValue(FString Key) {
	return mJsonObject->HasField(Key);
}

bool UBlueprintJsonObject::GetValue(FString Key, UBlueprintJsonValue*& Value) {
	if (!HasValue(Key))
		return false;

	TSharedPtr<FJsonValue> val = mJsonObject->TryGetField(Key);
	if (!val.IsValid())
		return false;

	Value = UBlueprintJsonValue::FromSharedPointer(val);
	return true;
}

UBlueprintJsonObject* UBlueprintJsonObject::SetValue(FString Key, UBlueprintJsonValue* Value) {
	if (Value != nullptr)
		mJsonObject->SetField(Key, Value->ToSharedPointer());
	return this;
}

UBlueprintJsonObject* UBlueprintJsonObject::DeleteValue(FString Key) {
	mJsonObject->RemoveField(Key);
	return this;
}

bool UBlueprintJsonObject::HasBoolean(FString Key) {
	return mJsonObject->HasTypedField<EJson::Boolean>(Key);
}

bool UBlueprintJsonObject::GetBoolean(FString Key, bool& Value) {
	if (!HasBoolean(Key))
		return false;
	return mJsonObject->TryGetBoolField(Key, Value);
}

UBlueprintJsonObject* UBlueprintJsonObject::SetBoolean(FString Key, bool Value) {
	mJsonObject->SetBoolField(Key, Value);
	return this;
}

bool UBlueprintJsonObject::HasInteger(FString Key) {
	if (!mJsonObject->HasTypedField<EJson::Number>(Key))
		return false;

	double val;
	bool success = mJsonObject->TryGetNumberField(Key, val);
	if (!success)
		return false;

	double intpart;
	return std::modf(val, &intpart) == 0.0;
}

bool UBlueprintJsonObject::GetInteger(FString Key, int32& Value) {
	if (!HasInteger(Key))
		return false;
	return mJsonObject->TryGetNumberField(Key, Value);
}

UBlueprintJsonObject* UBlueprintJsonObject::SetInteger(FString Key, int32 Value) {
	mJsonObject->SetNumberField(Key, Value);
	return this;
}

bool UBlueprintJsonObject::HasFloat(FString Key) {
	return mJsonObject->HasTypedField<EJson::Number>(Key);
}

bool UBlueprintJsonObject::GetFloat(FString Key, float& Value) {
	if (!HasFloat(Key))
		return false;
	double val;
	bool success = mJsonObject->TryGetNumberField(Key, val);
	if (success)
		Value = val;
	return success;	
}

UBlueprintJsonObject* UBlueprintJsonObject::SetFloat(FString Key, float Value) {
	mJsonObject->SetNumberField(Key, Value);
	return this;
}

bool UBlueprintJsonObject::HasString(FString Key) {
	return mJsonObject->HasTypedField<EJson::String>(Key);
}

bool UBlueprintJsonObject::GetString(FString Key, FString& Value) {
	if (!HasString(Key))
		return false;
	return mJsonObject->TryGetStringField(Key, Value);
}

UBlueprintJsonObject* UBlueprintJsonObject::SetString(FString Key, FString Value) {
	mJsonObject->SetStringField(Key, Value);
	return this;
}

bool UBlueprintJsonObject::HasObject(FString Key) {
	return mJsonObject->HasTypedField<EJson::Number>(Key);
}

bool UBlueprintJsonObject::GetObject(FString Key, UBlueprintJsonObject*& Value) {
	const TSharedPtr<FJsonObject>* JsonObject;
	bool Success = mJsonObject->TryGetObjectField(Key, JsonObject);
	if (Success)
		Value = UBlueprintJsonObject::FromSharedPointer(*JsonObject);

	return Success;
}

UBlueprintJsonObject* UBlueprintJsonObject::SetObject(FString Key, UBlueprintJsonObject* Value) {
	if (Value != nullptr)
		mJsonObject->SetObjectField(Key, Value->ToSharedPointer());
	return this;
}

bool UBlueprintJsonObject::HasArray(FString Key) {
	return mJsonObject->HasTypedField<EJson::Array>(Key);
}

bool UBlueprintJsonObject::GetArray(FString Key, TArray<UBlueprintJsonValue*>& Values) {
	if (!HasArray(Key))
		return false;

	const TArray<TSharedPtr<FJsonValue>>* vals;
	bool success = mJsonObject->TryGetArrayField(Key, vals);
	if (!success)
		return false;

	for (TSharedPtr<FJsonValue> Value : *vals) {
		Values.Add(UBlueprintJsonValue::FromSharedPointer(Value));
	}
	return true;
}

UBlueprintJsonObject* UBlueprintJsonObject::SetArray(FString Key, TArray<UBlueprintJsonValue*> Values) {
	TArray<TSharedPtr<FJsonValue>> vals;
	for (UBlueprintJsonValue* Value : Values) {
		if (Value != nullptr)
			vals.Add(Value->ToSharedPointer());
	}
	mJsonObject->SetArrayField(Key, vals);
	return this;
}