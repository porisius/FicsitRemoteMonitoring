// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "UE4Duino/Public/Serial.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSerial() {}
// Cross Module References
	UE4DUINO_API UEnum* Z_Construct_UEnum_UE4Duino_ELineEnd();
	UPackage* Z_Construct_UPackage__Script_UE4Duino();
	UE4DUINO_API UClass* Z_Construct_UClass_USerial_NoRegister();
	UE4DUINO_API UClass* Z_Construct_UClass_USerial();
	COREUOBJECT_API UClass* Z_Construct_UClass_UObject();
// End Cross Module References
	static UEnum* ELineEnd_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_UE4Duino_ELineEnd, Z_Construct_UPackage__Script_UE4Duino(), TEXT("ELineEnd"));
		}
		return Singleton;
	}
	template<> UE4DUINO_API UEnum* StaticEnum<ELineEnd>()
	{
		return ELineEnd_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_ELineEnd(ELineEnd_StaticEnum, TEXT("/Script/UE4Duino"), TEXT("ELineEnd"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_UE4Duino_ELineEnd_Hash() { return 963233376U; }
	UEnum* Z_Construct_UEnum_UE4Duino_ELineEnd()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_UE4Duino();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("ELineEnd"), 0, Get_Z_Construct_UEnum_UE4Duino_ELineEnd_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "ELineEnd::rn", (int64)ELineEnd::rn },
				{ "ELineEnd::n", (int64)ELineEnd::n },
				{ "ELineEnd::r", (int64)ELineEnd::r },
				{ "ELineEnd::nr", (int64)ELineEnd::nr },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "BlueprintType", "true" },
				{ "Category", "UE4Duino" },
				{ "ModuleRelativePath", "Public/Serial.h" },
				{ "n.DisplayName", "\\n" },
				{ "n.Name", "ELineEnd::n" },
				{ "nr.DisplayName", "\\n\\r" },
				{ "nr.Name", "ELineEnd::nr" },
				{ "r.DisplayName", "\\r" },
				{ "r.Name", "ELineEnd::r" },
				{ "rn.DisplayName", "\\r\\n" },
				{ "rn.Name", "ELineEnd::rn" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_UE4Duino,
				nullptr,
				"ELineEnd",
				"ELineEnd",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	DEFINE_FUNCTION(USerial::execLineEndToStr)
	{
		P_GET_ENUM(ELineEnd,Z_Param_LineEnd);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(FString*)Z_Param__Result=P_THIS->LineEndToStr(ELineEnd(Z_Param_LineEnd));
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execGetBaud)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*(int32*)Z_Param__Result=P_THIS->GetBaud();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execGetPort)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*(int32*)Z_Param__Result=P_THIS->GetPort();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execIsOpened)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->IsOpened();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execFlush)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->Flush();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execWriteBytes)
	{
		P_GET_TARRAY(uint8,Z_Param_Buffer);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->WriteBytes(Z_Param_Buffer);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execWriteByte)
	{
		P_GET_PROPERTY(FByteProperty,Z_Param_Value);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->WriteByte(Z_Param_Value);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execWriteInt)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_Value);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->WriteInt(Z_Param_Value);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execWriteFloat)
	{
		P_GET_PROPERTY(FFloatProperty,Z_Param_Value);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->WriteFloat(Z_Param_Value);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execPrintln)
	{
		P_GET_PROPERTY(FStrProperty,Z_Param_String);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->Println(Z_Param_String);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execPrint)
	{
		P_GET_PROPERTY(FStrProperty,Z_Param_String);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->Print(Z_Param_String);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadBytes)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_Limit);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(TArray<uint8>*)Z_Param__Result=P_THIS->ReadBytes(Z_Param_Limit);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadByte)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bSuccess);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(uint8*)Z_Param__Result=P_THIS->ReadByte(Z_Param_Out_bSuccess);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadInt)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bSuccess);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(int32*)Z_Param__Result=P_THIS->ReadInt(Z_Param_Out_bSuccess);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadFloat)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bSuccess);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(float*)Z_Param__Result=P_THIS->ReadFloat(Z_Param_Out_bSuccess);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadln)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bSuccess);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(FString*)Z_Param__Result=P_THIS->Readln(Z_Param_Out_bSuccess);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execReadString)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bSuccess);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(FString*)Z_Param__Result=P_THIS->ReadString(Z_Param_Out_bSuccess);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execClose)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->Close();
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execOpen)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_Port);
		P_GET_PROPERTY(FIntProperty,Z_Param_BaudRate);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=P_THIS->Open(Z_Param_Port,Z_Param_BaudRate);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execFloatToBytes)
	{
		P_GET_PROPERTY_REF(FFloatProperty,Z_Param_Out_Float);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(TArray<uint8>*)Z_Param__Result=USerial::FloatToBytes(Z_Param_Out_Float);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execBytesToFloat)
	{
		P_GET_TARRAY(uint8,Z_Param_Bytes);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(float*)Z_Param__Result=USerial::BytesToFloat(Z_Param_Bytes);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execIntToBytes)
	{
		P_GET_PROPERTY_REF(FIntProperty,Z_Param_Out_Int);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(TArray<uint8>*)Z_Param__Result=USerial::IntToBytes(Z_Param_Out_Int);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execBytesToInt)
	{
		P_GET_TARRAY(uint8,Z_Param_Bytes);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(int32*)Z_Param__Result=USerial::BytesToInt(Z_Param_Bytes);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USerial::execOpenComPort)
	{
		P_GET_UBOOL_REF(Z_Param_Out_bOpened);
		P_GET_PROPERTY(FIntProperty,Z_Param_Port);
		P_GET_PROPERTY(FIntProperty,Z_Param_BaudRate);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(USerial**)Z_Param__Result=USerial::OpenComPort(Z_Param_Out_bOpened,Z_Param_Port,Z_Param_BaudRate);
		P_NATIVE_END;
	}
	void USerial::StaticRegisterNativesUSerial()
	{
		UClass* Class = USerial::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "BytesToFloat", &USerial::execBytesToFloat },
			{ "BytesToInt", &USerial::execBytesToInt },
			{ "Close", &USerial::execClose },
			{ "FloatToBytes", &USerial::execFloatToBytes },
			{ "Flush", &USerial::execFlush },
			{ "GetBaud", &USerial::execGetBaud },
			{ "GetPort", &USerial::execGetPort },
			{ "IntToBytes", &USerial::execIntToBytes },
			{ "IsOpened", &USerial::execIsOpened },
			{ "LineEndToStr", &USerial::execLineEndToStr },
			{ "Open", &USerial::execOpen },
			{ "OpenComPort", &USerial::execOpenComPort },
			{ "Print", &USerial::execPrint },
			{ "Println", &USerial::execPrintln },
			{ "ReadByte", &USerial::execReadByte },
			{ "ReadBytes", &USerial::execReadBytes },
			{ "ReadFloat", &USerial::execReadFloat },
			{ "ReadInt", &USerial::execReadInt },
			{ "Readln", &USerial::execReadln },
			{ "ReadString", &USerial::execReadString },
			{ "WriteByte", &USerial::execWriteByte },
			{ "WriteBytes", &USerial::execWriteBytes },
			{ "WriteFloat", &USerial::execWriteFloat },
			{ "WriteInt", &USerial::execWriteInt },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USerial_BytesToFloat_Statics
	{
		struct Serial_eventBytesToFloat_Parms
		{
			TArray<uint8> Bytes;
			float ReturnValue;
		};
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_Bytes_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Bytes;
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_Bytes_Inner = { "Bytes", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_Bytes = { "Bytes", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventBytesToFloat_Parms, Bytes), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventBytesToFloat_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_BytesToFloat_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_Bytes_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_Bytes,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToFloat_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_BytesToFloat_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Utility function to convert 4 bytes into a float. If the input array's length is not 4, returns 0.0.\n\x09 *\n\x09 * @param Bytes A byte array with 4 values representing the float in IEEE 754 standard format.\n\x09 * @return The final float value or 0.0 for an invalid array.\n\x09 */" },
		{ "DisplayName", "Bytes to Float" },
		{ "Keywords", "cast concatenate group bit bitwise" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Utility function to convert 4 bytes into a float. If the input array's length is not 4, returns 0.0.\n\n@param Bytes A byte array with 4 values representing the float in IEEE 754 standard format.\n@return The final float value or 0.0 for an invalid array." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_BytesToFloat_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "BytesToFloat", nullptr, nullptr, sizeof(Serial_eventBytesToFloat_Parms), Z_Construct_UFunction_USerial_BytesToFloat_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_BytesToFloat_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_BytesToFloat_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_BytesToFloat_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_BytesToFloat()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_BytesToFloat_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_BytesToInt_Statics
	{
		struct Serial_eventBytesToInt_Parms
		{
			TArray<uint8> Bytes;
			int32 ReturnValue;
		};
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_Bytes_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Bytes;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_Bytes_Inner = { "Bytes", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_Bytes = { "Bytes", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventBytesToInt_Parms, Bytes), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventBytesToInt_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_BytesToInt_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_Bytes_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_Bytes,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_BytesToInt_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_BytesToInt_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Utility function to convert 4 bytes into an Integer. If the input array's length is not 4, returns 0.\n\x09 * \n\x09 * @param Bytes A byte array with 4 values representing the integer in little-endian format.\n\x09 * @return The final integer value or 0 for an invalid array.\n\x09 */" },
		{ "DisplayName", "Bytes to Int" },
		{ "Keywords", "cast concatenate group bit bitwise" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Utility function to convert 4 bytes into an Integer. If the input array's length is not 4, returns 0.\n\n@param Bytes A byte array with 4 values representing the integer in little-endian format.\n@return The final integer value or 0 for an invalid array." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_BytesToInt_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "BytesToInt", nullptr, nullptr, sizeof(Serial_eventBytesToInt_Parms), Z_Construct_UFunction_USerial_BytesToInt_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_BytesToInt_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14022401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_BytesToInt_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_BytesToInt_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_BytesToInt()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_BytesToInt_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Close_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Close_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Close and end the communication with the serial port. If not open, do nothing.\n\x09 */" },
		{ "DisplayName", "Close Port" },
		{ "Keywords", "com end finish release" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Close and end the communication with the serial port. If not open, do nothing." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Close_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Close", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Close_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Close_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Close()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Close_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_FloatToBytes_Statics
	{
		struct Serial_eventFloatToBytes_Parms
		{
			float Float;
			TArray<uint8> ReturnValue;
		};
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Float_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Float;
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_ReturnValue_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_Float_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_Float = { "Float", nullptr, (EPropertyFlags)0x0010000008000182, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventFloatToBytes_Parms, Float), METADATA_PARAMS(Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_Float_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_Float_MetaData)) };
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_ReturnValue_Inner = { "ReturnValue", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventFloatToBytes_Parms, ReturnValue), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_FloatToBytes_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_Float,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_ReturnValue_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_FloatToBytes_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_FloatToBytes_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Utility function to get the 4 bytes that make a float.\n\x09 *\n\x09 * @param Float The float value to be converted.\n\x09 * @return A byte array containing the 4 bytes that make the float, in IEEE 754 standard format.\n\x09 */" },
		{ "DisplayName", "Float to Bytes" },
		{ "Keywords", "cast separate bit bitwise" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Utility function to get the 4 bytes that make a float.\n\n@param Float The float value to be converted.\n@return A byte array containing the 4 bytes that make the float, in IEEE 754 standard format." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_FloatToBytes_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "FloatToBytes", nullptr, nullptr, sizeof(Serial_eventFloatToBytes_Parms), Z_Construct_UFunction_USerial_FloatToBytes_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_FloatToBytes_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14422401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_FloatToBytes_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_FloatToBytes_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_FloatToBytes()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_FloatToBytes_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Flush_Statics
	{
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Flush_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/** Clean the serial port by reading everything left to be read. */" },
		{ "DisplayName", "Flush Port" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Clean the serial port by reading everything left to be read." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Flush_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Flush", nullptr, nullptr, 0, nullptr, 0, RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Flush_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Flush_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Flush()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Flush_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_GetBaud_Statics
	{
		struct Serial_eventGetBaud_Parms
		{
			int32 ReturnValue;
		};
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_GetBaud_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventGetBaud_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_GetBaud_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_GetBaud_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_GetBaud_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Read the selected BaudRate for this Serial instance.\n\x09 * @return The baud rate.\n\x09 */" },
		{ "DisplayName", "Get Baud Rate" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Read the selected BaudRate for this Serial instance.\n@return The baud rate." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_GetBaud_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "GetBaud", nullptr, nullptr, sizeof(Serial_eventGetBaud_Parms), Z_Construct_UFunction_USerial_GetBaud_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_GetBaud_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_GetBaud_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_GetBaud_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_GetBaud()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_GetBaud_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_GetPort_Statics
	{
		struct Serial_eventGetPort_Parms
		{
			int32 ReturnValue;
		};
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_GetPort_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventGetPort_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_GetPort_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_GetPort_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_GetPort_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Read the number of the serial port selected for this Serial instance.\n\x09 * @return The number of the serial port.\n\x09 */" },
		{ "DisplayName", "Get Port Number" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Read the number of the serial port selected for this Serial instance.\n@return The number of the serial port." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_GetPort_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "GetPort", nullptr, nullptr, sizeof(Serial_eventGetPort_Parms), Z_Construct_UFunction_USerial_GetPort_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_GetPort_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_GetPort_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_GetPort_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_GetPort()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_GetPort_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_IntToBytes_Statics
	{
		struct Serial_eventIntToBytes_Parms
		{
			int32 Int;
			TArray<uint8> ReturnValue;
		};
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Int_MetaData[];
#endif
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Int;
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_ReturnValue_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_Int_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_Int = { "Int", nullptr, (EPropertyFlags)0x0010000008000182, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventIntToBytes_Parms, Int), METADATA_PARAMS(Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_Int_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_Int_MetaData)) };
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_ReturnValue_Inner = { "ReturnValue", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventIntToBytes_Parms, ReturnValue), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_IntToBytes_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_Int,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_ReturnValue_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_IntToBytes_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_IntToBytes_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Utility function to get the 4 bytes that make an integer.\n\x09 *\n\x09 * @param Int The integer value to be converted.\n\x09 * @return A byte array containing the 4 bytes that make the integer, starting from the least significant one (little endian).\n\x09 */" },
		{ "DisplayName", "Int to Bytes" },
		{ "Keywords", "cast separate bit bitwise" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Utility function to get the 4 bytes that make an integer.\n\n@param Int The integer value to be converted.\n@return A byte array containing the 4 bytes that make the integer, starting from the least significant one (little endian)." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_IntToBytes_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "IntToBytes", nullptr, nullptr, sizeof(Serial_eventIntToBytes_Parms), Z_Construct_UFunction_USerial_IntToBytes_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_IntToBytes_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14422401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_IntToBytes_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_IntToBytes_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_IntToBytes()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_IntToBytes_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_IsOpened_Statics
	{
		struct Serial_eventIsOpened_Parms
		{
			bool ReturnValue;
		};
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_IsOpened_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventIsOpened_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_IsOpened_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventIsOpened_Parms), &Z_Construct_UFunction_USerial_IsOpened_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_IsOpened_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_IsOpened_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_IsOpened_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Check if the serial port is open.\n\x09 * @return True if the serial port is open.\n\x09 */" },
		{ "DisplayName", "Is Port Open" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Check if the serial port is open.\n@return True if the serial port is open." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_IsOpened_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "IsOpened", nullptr, nullptr, sizeof(Serial_eventIsOpened_Parms), Z_Construct_UFunction_USerial_IsOpened_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_IsOpened_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_IsOpened_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_IsOpened_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_IsOpened()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_IsOpened_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_LineEndToStr_Statics
	{
		struct Serial_eventLineEndToStr_Parms
		{
			ELineEnd LineEnd;
			FString ReturnValue;
		};
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_LineEnd_Underlying;
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_LineEnd;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_LineEnd_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_LineEnd = { "LineEnd", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventLineEndToStr_Parms, LineEnd), Z_Construct_UEnum_UE4Duino_ELineEnd, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventLineEndToStr_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_LineEndToStr_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_LineEnd_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_LineEnd,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_LineEndToStr_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_LineEndToStr_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Converts a LineEnd enum value to String.\n\x09 * @param LineEnd LineEnd enum value.\n\x09 * @return The LineEnd value in string format.\n\x09 */" },
		{ "DisplayName", "Line End to String" },
		{ "keywords", "cast convert" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Converts a LineEnd enum value to String.\n@param LineEnd LineEnd enum value.\n@return The LineEnd value in string format." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_LineEndToStr_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "LineEndToStr", nullptr, nullptr, sizeof(Serial_eventLineEndToStr_Parms), Z_Construct_UFunction_USerial_LineEndToStr_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_LineEndToStr_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_LineEndToStr_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_LineEndToStr_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_LineEndToStr()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_LineEndToStr_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Open_Statics
	{
		struct Serial_eventOpen_Parms
		{
			int32 Port;
			int32 BaudRate;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Port;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_BaudRate;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_Open_Statics::NewProp_Port = { "Port", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventOpen_Parms, Port), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_Open_Statics::NewProp_BaudRate = { "BaudRate", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventOpen_Parms, BaudRate), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_Open_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventOpen_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_Open_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventOpen_Parms), &Z_Construct_UFunction_USerial_Open_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_Open_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Open_Statics::NewProp_Port,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Open_Statics::NewProp_BaudRate,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Open_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Open_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Open a serial port. Don't forget to close the port before exiting the game.\n\x09 * If this Serial instance has already an opened port,\n\x09 * return false and doesn't change the opened port number.\n\x09 *\n\x09 * @param Port The serial port to open.\n\x09 * @param BaudRate BaudRate to open the serial port with.\n\x09 * @return If the serial port was successfully opened.\n\x09 */" },
		{ "CPP_Default_BaudRate", "9600" },
		{ "CPP_Default_Port", "2" },
		{ "DisplayName", "Open Port" },
		{ "Keywords", "com start init" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Open a serial port. Don't forget to close the port before exiting the game.\nIf this Serial instance has already an opened port,\nreturn false and doesn't change the opened port number.\n\n@param Port The serial port to open.\n@param BaudRate BaudRate to open the serial port with.\n@return If the serial port was successfully opened." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Open_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Open", nullptr, nullptr, sizeof(Serial_eventOpen_Parms), Z_Construct_UFunction_USerial_Open_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Open_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Open_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Open_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Open()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Open_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_OpenComPort_Statics
	{
		struct Serial_eventOpenComPort_Parms
		{
			bool bOpened;
			int32 Port;
			int32 BaudRate;
			USerial* ReturnValue;
		};
		static void NewProp_bOpened_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bOpened;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Port;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_BaudRate;
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_bOpened_SetBit(void* Obj)
	{
		((Serial_eventOpenComPort_Parms*)Obj)->bOpened = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_bOpened = { "bOpened", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventOpenComPort_Parms), &Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_bOpened_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_Port = { "Port", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventOpenComPort_Parms, Port), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_BaudRate = { "BaudRate", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventOpenComPort_Parms, BaudRate), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventOpenComPort_Parms, ReturnValue), Z_Construct_UClass_USerial_NoRegister, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_OpenComPort_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_bOpened,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_Port,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_BaudRate,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_OpenComPort_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_OpenComPort_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Open a serial port and return the created Serial instance.\n\x09 * Don't forget to close the port before exiting the game.\n\x09 *\n\x09 * @param bOpened If the serial port was successfully opened.\n\x09 * @param Port The serial port to open.\n\x09 * @param BaudRate BaudRate to open the serial port with.\n\x09 * @return A Serial instance to work with the opened port.\n\x09 */" },
		{ "CPP_Default_BaudRate", "9600" },
		{ "CPP_Default_Port", "1" },
		{ "DisplayName", "Open Serial Port" },
		{ "Keywords", "com arduino serial start" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Open a serial port and return the created Serial instance.\nDon't forget to close the port before exiting the game.\n\n@param bOpened If the serial port was successfully opened.\n@param Port The serial port to open.\n@param BaudRate BaudRate to open the serial port with.\n@return A Serial instance to work with the opened port." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_OpenComPort_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "OpenComPort", nullptr, nullptr, sizeof(Serial_eventOpenComPort_Parms), Z_Construct_UFunction_USerial_OpenComPort_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_OpenComPort_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04422401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_OpenComPort_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_OpenComPort_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_OpenComPort()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_OpenComPort_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Print_Statics
	{
		struct Serial_eventPrint_Parms
		{
			FString String;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_String;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USerial_Print_Statics::NewProp_String = { "String", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventPrint_Parms, String), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_Print_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventPrint_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_Print_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventPrint_Parms), &Z_Construct_UFunction_USerial_Print_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_Print_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Print_Statics::NewProp_String,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Print_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Print_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes a string without newline to the serial port.\n\x09 * @param String The string to be sent to the serial port.\n\x09 * @return True if the string was sent.\n\x09 */" },
		{ "DisplayName", "Print" },
		{ "keywords", "send write string words text characters" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes a string without newline to the serial port.\n@param String The string to be sent to the serial port.\n@return True if the string was sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Print_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Print", nullptr, nullptr, sizeof(Serial_eventPrint_Parms), Z_Construct_UFunction_USerial_Print_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Print_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Print_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Print_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Print()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Print_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Println_Statics
	{
		struct Serial_eventPrintln_Parms
		{
			FString String;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_String;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USerial_Println_Statics::NewProp_String = { "String", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventPrintln_Parms, String), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_Println_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventPrintln_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_Println_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventPrintln_Parms), &Z_Construct_UFunction_USerial_Println_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_Println_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Println_Statics::NewProp_String,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Println_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Println_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes a string with newline (\\n) appended at the end to the serial port.\n\x09 * @param String The string to be sent to the serial port.\n\x09 * @return True if the string was sent.\n\x09 */" },
		{ "DisplayName", "Print Line" },
		{ "keywords", "send write string words text characters" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes a string with newline (\\n) appended at the end to the serial port.\n@param String The string to be sent to the serial port.\n@return True if the string was sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Println_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Println", nullptr, nullptr, sizeof(Serial_eventPrintln_Parms), Z_Construct_UFunction_USerial_Println_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Println_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Println_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Println_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Println()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Println_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_ReadByte_Statics
	{
		struct Serial_eventReadByte_Parms
		{
			bool bSuccess;
			uint8 ReturnValue;
		};
		static void NewProp_bSuccess_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSuccess;
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_bSuccess_SetBit(void* Obj)
	{
		((Serial_eventReadByte_Parms*)Obj)->bSuccess = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_bSuccess = { "bSuccess", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventReadByte_Parms), &Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_bSuccess_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadByte_Parms, ReturnValue), nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_ReadByte_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_bSuccess,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadByte_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_ReadByte_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Reads a byte from the serial port.\n\x09 * @param bSuccess True if there were 4 bytes to read.\n\x09 * @return The read value\n\x09 */" },
		{ "DisplayName", "Read a Byte" },
		{ "keywords", "get read receive" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Reads a byte from the serial port.\n@param bSuccess True if there were 4 bytes to read.\n@return The read value" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_ReadByte_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "ReadByte", nullptr, nullptr, sizeof(Serial_eventReadByte_Parms), Z_Construct_UFunction_USerial_ReadByte_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadByte_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_ReadByte_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadByte_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_ReadByte()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_ReadByte_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_ReadBytes_Statics
	{
		struct Serial_eventReadBytes_Parms
		{
			int32 Limit;
			TArray<uint8> ReturnValue;
		};
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Limit;
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_ReturnValue_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_Limit = { "Limit", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadBytes_Parms, Limit), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_ReturnValue_Inner = { "ReturnValue", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadBytes_Parms, ReturnValue), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_ReadBytes_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_Limit,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_ReturnValue_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadBytes_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_ReadBytes_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Reads up to Limit bytes from the serial port. If there are less than Limit,\n\x09 * reads all of them and return True.\n\x09 * @param bSuccess True if there was at least 1 byte to read.\n\x09 * @return An array containing the read bytes\n\x09 */" },
		{ "CPP_Default_Limit", "256" },
		{ "DisplayName", "Read Bytes" },
		{ "keywords", "get read receive" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Reads up to Limit bytes from the serial port. If there are less than Limit,\nreads all of them and return True.\n@param bSuccess True if there was at least 1 byte to read.\n@return An array containing the read bytes" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_ReadBytes_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "ReadBytes", nullptr, nullptr, sizeof(Serial_eventReadBytes_Parms), Z_Construct_UFunction_USerial_ReadBytes_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadBytes_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_ReadBytes_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadBytes_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_ReadBytes()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_ReadBytes_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_ReadFloat_Statics
	{
		struct Serial_eventReadFloat_Parms
		{
			bool bSuccess;
			float ReturnValue;
		};
		static void NewProp_bSuccess_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSuccess;
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_bSuccess_SetBit(void* Obj)
	{
		((Serial_eventReadFloat_Parms*)Obj)->bSuccess = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_bSuccess = { "bSuccess", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventReadFloat_Parms), &Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_bSuccess_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadFloat_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_ReadFloat_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_bSuccess,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadFloat_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_ReadFloat_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Reads a float from the serial port (sent as 4 bytes).\n\x09 * @param bSuccess True if there were 4 bytes to read.\n\x09 * @return The read value\n\x09 */" },
		{ "DisplayName", "Read a Float" },
		{ "keywords", "get read receive" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Reads a float from the serial port (sent as 4 bytes).\n@param bSuccess True if there were 4 bytes to read.\n@return The read value" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_ReadFloat_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "ReadFloat", nullptr, nullptr, sizeof(Serial_eventReadFloat_Parms), Z_Construct_UFunction_USerial_ReadFloat_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadFloat_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_ReadFloat_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadFloat_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_ReadFloat()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_ReadFloat_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_ReadInt_Statics
	{
		struct Serial_eventReadInt_Parms
		{
			bool bSuccess;
			int32 ReturnValue;
		};
		static void NewProp_bSuccess_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSuccess;
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_bSuccess_SetBit(void* Obj)
	{
		((Serial_eventReadInt_Parms*)Obj)->bSuccess = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_bSuccess = { "bSuccess", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventReadInt_Parms), &Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_bSuccess_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadInt_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_ReadInt_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_bSuccess,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadInt_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_ReadInt_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Reads an integer from the serial port (sent as 4 bytes).\n\x09 * @param bSuccess True if there were 4 bytes to read.\n\x09 * @return The read value\n\x09 */" },
		{ "DisplayName", "Read an Int" },
		{ "keywords", "get read receive integer" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Reads an integer from the serial port (sent as 4 bytes).\n@param bSuccess True if there were 4 bytes to read.\n@return The read value" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_ReadInt_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "ReadInt", nullptr, nullptr, sizeof(Serial_eventReadInt_Parms), Z_Construct_UFunction_USerial_ReadInt_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadInt_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_ReadInt_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadInt_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_ReadInt()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_ReadInt_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_Readln_Statics
	{
		struct Serial_eventReadln_Parms
		{
			bool bSuccess;
			FString ReturnValue;
		};
		static void NewProp_bSuccess_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSuccess;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_Readln_Statics::NewProp_bSuccess_SetBit(void* Obj)
	{
		((Serial_eventReadln_Parms*)Obj)->bSuccess = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_Readln_Statics::NewProp_bSuccess = { "bSuccess", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventReadln_Parms), &Z_Construct_UFunction_USerial_Readln_Statics::NewProp_bSuccess_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USerial_Readln_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadln_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_Readln_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Readln_Statics::NewProp_bSuccess,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_Readln_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_Readln_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Will read characters from Serial port until \\r\\n (Arduino println line end) is found.\n\x09 * \n\x09 * @param bSuccess If there was anything to read.\n\x09 * @return The read string\n\x09 */" },
		{ "DisplayName", "Read Line" },
		{ "keywords", "get read receive string words text characters" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Will read characters from Serial port until \\r\\n (Arduino println line end) is found.\n\n@param bSuccess If there was anything to read.\n@return The read string" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_Readln_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "Readln", nullptr, nullptr, sizeof(Serial_eventReadln_Parms), Z_Construct_UFunction_USerial_Readln_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Readln_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_Readln_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_Readln_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_Readln()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_Readln_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_ReadString_Statics
	{
		struct Serial_eventReadString_Parms
		{
			bool bSuccess;
			FString ReturnValue;
		};
		static void NewProp_bSuccess_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_bSuccess;
		static const UE4CodeGen_Private::FStrPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	void Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_bSuccess_SetBit(void* Obj)
	{
		((Serial_eventReadString_Parms*)Obj)->bSuccess = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_bSuccess = { "bSuccess", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventReadString_Parms), &Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_bSuccess_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStrPropertyParams Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventReadString_Parms, ReturnValue), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_ReadString_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_bSuccess,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_ReadString_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_ReadString_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Will read characters from Serial port until \\0 (null char) is found or there are no \n\x09 * characters left to read.\n\x09 *\n\x09 * @param bSuccess If there was anything to read.\n\x09 * @return The read string\n\x09 */" },
		{ "DisplayName", "Read String" },
		{ "keywords", "get read receive string words text characters" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Will read characters from Serial port until \\0 (null char) is found or there are no\ncharacters left to read.\n\n@param bSuccess If there was anything to read.\n@return The read string" },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_ReadString_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "ReadString", nullptr, nullptr, sizeof(Serial_eventReadString_Parms), Z_Construct_UFunction_USerial_ReadString_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadString_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_ReadString_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_ReadString_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_ReadString()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_ReadString_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_WriteByte_Statics
	{
		struct Serial_eventWriteByte_Parms
		{
			uint8 Value;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_Value;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_Value = { "Value", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventWriteByte_Parms, Value), nullptr, METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventWriteByte_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventWriteByte_Parms), &Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_WriteByte_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_Value,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteByte_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_WriteByte_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes a byte value to the serial port.\n\x09 * @param Value The value to be sent to the serial port.\n\x09 * @return True if the byte was sent.\n\x09 */" },
		{ "DisplayName", "Write a Byte" },
		{ "keywords", "send" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes a byte value to the serial port.\n@param Value The value to be sent to the serial port.\n@return True if the byte was sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_WriteByte_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "WriteByte", nullptr, nullptr, sizeof(Serial_eventWriteByte_Parms), Z_Construct_UFunction_USerial_WriteByte_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteByte_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_WriteByte_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteByte_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_WriteByte()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_WriteByte_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_WriteBytes_Statics
	{
		struct Serial_eventWriteBytes_Parms
		{
			TArray<uint8> Buffer;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_Buffer_Inner;
		static const UE4CodeGen_Private::FArrayPropertyParams NewProp_Buffer;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_Buffer_Inner = { "Buffer", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_Buffer = { "Buffer", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventWriteBytes_Parms, Buffer), EArrayPropertyFlags::None, METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventWriteBytes_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventWriteBytes_Parms), &Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_WriteBytes_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_Buffer_Inner,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_Buffer,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteBytes_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_WriteBytes_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes a byte array as a sequence of bytes to the serial port.\n\x09 * @param Buffer The byte array to be sent to the serial port.\n\x09 * @return True if the bytes were sent.\n\x09 */" },
		{ "DisplayName", "Write Bytes" },
		{ "keywords", "send" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes a byte array as a sequence of bytes to the serial port.\n@param Buffer The byte array to be sent to the serial port.\n@return True if the bytes were sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_WriteBytes_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "WriteBytes", nullptr, nullptr, sizeof(Serial_eventWriteBytes_Parms), Z_Construct_UFunction_USerial_WriteBytes_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteBytes_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_WriteBytes_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteBytes_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_WriteBytes()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_WriteBytes_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_WriteFloat_Statics
	{
		struct Serial_eventWriteFloat_Parms
		{
			float Value;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_Value;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_Value = { "Value", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventWriteFloat_Parms, Value), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventWriteFloat_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventWriteFloat_Parms), &Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_WriteFloat_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_Value,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteFloat_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_WriteFloat_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes a float value to the serial port as 4 bytes.\n\x09 * @param Value The value to be sent to the serial port.\n\x09 * @return True if the bytes were sent.\n\x09 */" },
		{ "DisplayName", "Write a Float" },
		{ "keywords", "send" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes a float value to the serial port as 4 bytes.\n@param Value The value to be sent to the serial port.\n@return True if the bytes were sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_WriteFloat_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "WriteFloat", nullptr, nullptr, sizeof(Serial_eventWriteFloat_Parms), Z_Construct_UFunction_USerial_WriteFloat_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteFloat_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_WriteFloat_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteFloat_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_WriteFloat()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_WriteFloat_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USerial_WriteInt_Statics
	{
		struct Serial_eventWriteInt_Parms
		{
			int32 Value;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FIntPropertyParams NewProp_Value;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FIntPropertyParams Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_Value = { "Value", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(Serial_eventWriteInt_Parms, Value), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((Serial_eventWriteInt_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(Serial_eventWriteInt_Parms), &Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USerial_WriteInt_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_Value,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USerial_WriteInt_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USerial_WriteInt_Statics::Function_MetaDataParams[] = {
		{ "Category", "UE4Duino" },
		{ "Comment", "/**\n\x09 * Writes an integer value to the serial port as 4 bytes.\n\x09 * @param Value The value to be sent to the serial port.\n\x09 * @return True if the bytes were sent.\n\x09 */" },
		{ "DisplayName", "Write an Int" },
		{ "keywords", "integer send" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Writes an integer value to the serial port as 4 bytes.\n@param Value The value to be sent to the serial port.\n@return True if the bytes were sent." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USerial_WriteInt_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USerial, nullptr, "WriteInt", nullptr, nullptr, sizeof(Serial_eventWriteInt_Parms), Z_Construct_UFunction_USerial_WriteInt_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteInt_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USerial_WriteInt_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USerial_WriteInt_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USerial_WriteInt()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USerial_WriteInt_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USerial_NoRegister()
	{
		return USerial::StaticClass();
	}
	struct Z_Construct_UClass_USerial_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FBytePropertyParams NewProp_WriteLineEnd_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_WriteLineEnd_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_WriteLineEnd;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USerial_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UObject,
		(UObject* (*)())Z_Construct_UPackage__Script_UE4Duino,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USerial_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USerial_BytesToFloat, "BytesToFloat" }, // 2129967569
		{ &Z_Construct_UFunction_USerial_BytesToInt, "BytesToInt" }, // 2969873403
		{ &Z_Construct_UFunction_USerial_Close, "Close" }, // 3436734167
		{ &Z_Construct_UFunction_USerial_FloatToBytes, "FloatToBytes" }, // 707516258
		{ &Z_Construct_UFunction_USerial_Flush, "Flush" }, // 4159348829
		{ &Z_Construct_UFunction_USerial_GetBaud, "GetBaud" }, // 2818308664
		{ &Z_Construct_UFunction_USerial_GetPort, "GetPort" }, // 2609609758
		{ &Z_Construct_UFunction_USerial_IntToBytes, "IntToBytes" }, // 1192112113
		{ &Z_Construct_UFunction_USerial_IsOpened, "IsOpened" }, // 3591230975
		{ &Z_Construct_UFunction_USerial_LineEndToStr, "LineEndToStr" }, // 2668830949
		{ &Z_Construct_UFunction_USerial_Open, "Open" }, // 4269348613
		{ &Z_Construct_UFunction_USerial_OpenComPort, "OpenComPort" }, // 3142672355
		{ &Z_Construct_UFunction_USerial_Print, "Print" }, // 125437888
		{ &Z_Construct_UFunction_USerial_Println, "Println" }, // 357238754
		{ &Z_Construct_UFunction_USerial_ReadByte, "ReadByte" }, // 2826527536
		{ &Z_Construct_UFunction_USerial_ReadBytes, "ReadBytes" }, // 3314763299
		{ &Z_Construct_UFunction_USerial_ReadFloat, "ReadFloat" }, // 306051459
		{ &Z_Construct_UFunction_USerial_ReadInt, "ReadInt" }, // 4290069671
		{ &Z_Construct_UFunction_USerial_Readln, "Readln" }, // 2835149199
		{ &Z_Construct_UFunction_USerial_ReadString, "ReadString" }, // 1513917651
		{ &Z_Construct_UFunction_USerial_WriteByte, "WriteByte" }, // 3302828677
		{ &Z_Construct_UFunction_USerial_WriteBytes, "WriteBytes" }, // 1636710533
		{ &Z_Construct_UFunction_USerial_WriteFloat, "WriteFloat" }, // 2355645373
		{ &Z_Construct_UFunction_USerial_WriteInt, "WriteInt" }, // 1744618447
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USerial_Statics::Class_MetaDataParams[] = {
		{ "BlueprintType", "true" },
		{ "Category", "UE4Duino" },
		{ "IncludePath", "Serial.h" },
		{ "Keywords", "com arduino serial" },
		{ "ModuleRelativePath", "Public/Serial.h" },
	};
#endif
	const UE4CodeGen_Private::FBytePropertyParams Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd_MetaData[] = {
		{ "Category", "UE4Duino | String" },
		{ "Comment", "/** Determines the line ending used when writing lines to serial port with PrintLine. */" },
		{ "ModuleRelativePath", "Public/Serial.h" },
		{ "ToolTip", "Determines the line ending used when writing lines to serial port with PrintLine." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd = { "WriteLineEnd", nullptr, (EPropertyFlags)0x0010000000000004, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(USerial, WriteLineEnd), Z_Construct_UEnum_UE4Duino_ELineEnd, METADATA_PARAMS(Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USerial_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USerial_Statics::NewProp_WriteLineEnd,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USerial_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USerial>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USerial_Statics::ClassParams = {
		&USerial::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_USerial_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		UE_ARRAY_COUNT(Z_Construct_UClass_USerial_Statics::PropPointers),
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_USerial_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_USerial_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USerial()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USerial_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USerial, 3398703119);
	template<> UE4DUINO_API UClass* StaticClass<USerial>()
	{
		return USerial::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USerial(Z_Construct_UClass_USerial, &USerial::StaticClass, TEXT("/Script/UE4Duino"), TEXT("USerial"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USerial);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
