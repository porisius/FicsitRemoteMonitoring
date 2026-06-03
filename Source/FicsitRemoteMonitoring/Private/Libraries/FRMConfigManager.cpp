#include "Libraries/FRMConfigManager.h"

#include "Misc/Variant.h"

class UFGGameUserSettings;

bool UFRMConfigManager::GetStoredConfigType(const FString& StrID, EVariantTypes& OutType)
{
	UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if (!UserSettings)
	{
		UE_LOG(LogFRMConfigManager, Error, TEXT("GetStoredConfigType: UserSettings is null"));
		return false;
	}

	const FString FullSettingName = TEXT("FicsitRemoteMonitoring.") + FString(IsRunningDedicatedServer() ? "Server." : "") + StrID;
	const FVariant ConfigVariant = UserSettings->GetOptionValue(FullSettingName);
	OutType = ConfigVariant.GetType();

	if (OutType == EVariantTypes::Empty)
	{
		UE_LOG(LogFRMConfigManager, Warning, TEXT("GetStoredConfigType: Setting '%s' is empty or invalid"), *FullSettingName);
		return false;
	}

	return true;
}

bool UFRMConfigManager::GetExpectedConfigType(const FString& StrID, EVariantTypes& OutType)
{
	if (StrID == TEXT("uWS.Port"))
	{
		OutType = EVariantTypes::Int32;
		return true;
	}

	if (StrID == TEXT("uWS.PushCycle"))
	{
		OutType = EVariantTypes::Float;
		return true;
	}

	// Fallback to whatever is currently stored.
	return GetStoredConfigType(StrID, OutType);
}

bool UFRMConfigManager::ValidateConfigValue(const FString& StrID, const FVariant& InValue, FString& OutReason)
{
	OutReason.Empty();

	const EVariantTypes VariantType = InValue.GetType();

	if (StrID == TEXT("uWS.Port"))
	{
		int32 Port = 0;

		if (VariantType == EVariantTypes::Int32)
		{
			Port = InValue.GetValue<int32>();
		}
		else if (VariantType == EVariantTypes::String)
		{
			const FString StringValue = InValue.GetValue<FString>();
			if (!LexTryParseString(Port, *StringValue))
			{
				OutReason = TEXT("Port must be a valid integer.");
				return false;
			}
		}
		else
		{
			OutReason = TEXT("Port must be an integer.");
			return false;
		}

		if (Port < 1 || Port > 65535)
		{
			OutReason = TEXT("Port must be between 1 and 65535.");
			return false;
		}

		return true;
	}

	if (StrID == TEXT("uWS.PushCycle"))
	{
		float PushCycle = 0.0f;

		if (VariantType == EVariantTypes::Float)
		{
			PushCycle = InValue.GetValue<float>();
		}
		else if (VariantType == EVariantTypes::Int32)
		{
			PushCycle = static_cast<float>(InValue.GetValue<int32>());
		}
		else if (VariantType == EVariantTypes::String)
		{
			const FString StringValue = InValue.GetValue<FString>();
			if (!LexTryParseString(PushCycle, *StringValue))
			{
				OutReason = TEXT("PushCycle must be a valid number.");
				return false;
			}
		}
		else
		{
			OutReason = TEXT("PushCycle must be a number.");
			return false;
		}

		if (PushCycle < 0.05f)
		{
			OutReason = TEXT("PushCycle must be at least 0.05.");
			return false;
		}

		return true;
	}

	return true;
}

bool UFRMConfigManager::ParseConfigInputToVariant(
	const FString& StrID,
	const FString& RawTextValue,
	bool bCheckboxValue,
	FVariant& OutVariant,
	FString& OutReason
)
{
	OutReason.Empty();

	EVariantTypes ExpectedType;
	if (!GetExpectedConfigType(StrID, ExpectedType))
	{
		OutReason = TEXT("Could not determine expected setting type.");
		return false;
	}

	switch (ExpectedType)
	{
		case EVariantTypes::Bool:
		{
			OutVariant = FVariant(bCheckboxValue);
			return true;
		}

		case EVariantTypes::Int32:
		{
			int32 ParsedValue = 0;
			if (!LexTryParseString(ParsedValue, *RawTextValue))
			{
				OutReason = FString::Printf(TEXT("'%s' is not a valid integer."), *RawTextValue);
				UE_LOG(
					LogFRMConfigManager,
					Warning,
					TEXT("ParseConfigInputToVariant: Failed to parse int32 from '%s' for setting '%s'"),
					*RawTextValue,
					*StrID
				);
				return false;
			}

			OutVariant = FVariant(ParsedValue);
			return true;
		}

		case EVariantTypes::Float:
		{
			float ParsedValue = 0.0f;
			if (!LexTryParseString(ParsedValue, *RawTextValue))
			{
				OutReason = FString::Printf(TEXT("'%s' is not a valid float."), *RawTextValue);
				UE_LOG(
					LogFRMConfigManager,
					Warning,
					TEXT("ParseConfigInputToVariant: Failed to parse float from '%s' for setting '%s'"),
					*RawTextValue,
					*StrID
				);
				return false;
			}

			OutVariant = FVariant(ParsedValue);
			return true;
		}

		case EVariantTypes::String:
		{
			OutVariant = FVariant(RawTextValue);
			return true;
		}

		default:
		{
			OutReason = FString::Printf(TEXT("Unsupported variant type %d."), static_cast<int32>(ExpectedType));
			UE_LOG(
				LogFRMConfigManager,
				Warning,
				TEXT("ParseConfigInputToVariant: Unsupported variant type %d for setting '%s'"),
				static_cast<int32>(ExpectedType),
				*StrID
			);
			return false;
		}
	}
}

bool UFRMConfigManager::SetConfigFromInput(
	const FString& StrID,
	const FString& RawTextValue,
	bool bCheckboxValue
)
{
	UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if (!UserSettings)
	{
		UE_LOG(LogFRMConfigManager, Error, TEXT("SetConfigFromInput: UserSettings is null"));
		return false;
	}
	
	FVariant ParsedVariant;
	FString ParseReason;
	if (!ParseConfigInputToVariant(StrID, RawTextValue, bCheckboxValue, ParsedVariant, ParseReason))
	{
		UE_LOG(
			LogFRMConfigManager,
			Warning,
			TEXT("SetConfigFromInput: Failed to parse setting '%s': %s"),
			*StrID,
			*ParseReason
		);
		return false;
	}

	FString ValidationReason;
	if (!ValidateConfigValue(StrID, ParsedVariant, ValidationReason))
	{
		UE_LOG(
			LogFRMConfigManager,
			Warning,
			TEXT("SetConfigFromInput: Validation failed for setting '%s': %s"),
			*StrID,
			*ValidationReason
		);
		return false;
	}

	const FString FullSettingName = TEXT("FicsitRemoteMonitoring.") + FString(IsRunningDedicatedServer() ? "Server." : "") + StrID;
	UserSettings->SetOptionValue(FullSettingName, ParsedVariant);
	UserSettings->ApplyChanges();
	UserSettings->SaveSettings();
	

	UE_LOG(
		LogFRMConfigManager,
		Log,
		TEXT("SetConfigFromInput: Applied setting '%s'"),
		*FullSettingName
	);

	return true;
}

bool UFRMConfigManager::SetConfigFromInput(const FString& StrID, float Value)
{
	return SetConfigFromInput(StrID, FString::SanitizeFloat(Value), false);
}

bool UFRMConfigManager::SetConfigFromInput(const FString& StrID, int32 Value)
{
	return SetConfigFromInput(StrID, FString::FromInt(Value), false);
}

bool UFRMConfigManager::SetConfigFromInput(const FString& StrID, bool bValue)
{
	return SetConfigFromInput(StrID, TEXT(""), bValue);
}

bool UFRMConfigManager::SetConfigFromInput(const FString& StrID, const FString& Value)
{
	return SetConfigFromInput(StrID, Value, false);
}