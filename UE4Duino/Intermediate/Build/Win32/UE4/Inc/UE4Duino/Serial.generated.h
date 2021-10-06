// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
enum class ELineEnd : uint8;
class USerial;
#ifdef UE4DUINO_Serial_generated_h
#error "Serial.generated.h already included, missing '#pragma once' in Serial.h"
#endif
#define UE4DUINO_Serial_generated_h

#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_SPARSE_DATA
#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execLineEndToStr); \
	DECLARE_FUNCTION(execGetBaud); \
	DECLARE_FUNCTION(execGetPort); \
	DECLARE_FUNCTION(execIsOpened); \
	DECLARE_FUNCTION(execFlush); \
	DECLARE_FUNCTION(execWriteBytes); \
	DECLARE_FUNCTION(execWriteByte); \
	DECLARE_FUNCTION(execWriteInt); \
	DECLARE_FUNCTION(execWriteFloat); \
	DECLARE_FUNCTION(execPrintln); \
	DECLARE_FUNCTION(execPrint); \
	DECLARE_FUNCTION(execReadBytes); \
	DECLARE_FUNCTION(execReadByte); \
	DECLARE_FUNCTION(execReadInt); \
	DECLARE_FUNCTION(execReadFloat); \
	DECLARE_FUNCTION(execReadln); \
	DECLARE_FUNCTION(execReadString); \
	DECLARE_FUNCTION(execClose); \
	DECLARE_FUNCTION(execOpen); \
	DECLARE_FUNCTION(execFloatToBytes); \
	DECLARE_FUNCTION(execBytesToFloat); \
	DECLARE_FUNCTION(execIntToBytes); \
	DECLARE_FUNCTION(execBytesToInt); \
	DECLARE_FUNCTION(execOpenComPort);


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execLineEndToStr); \
	DECLARE_FUNCTION(execGetBaud); \
	DECLARE_FUNCTION(execGetPort); \
	DECLARE_FUNCTION(execIsOpened); \
	DECLARE_FUNCTION(execFlush); \
	DECLARE_FUNCTION(execWriteBytes); \
	DECLARE_FUNCTION(execWriteByte); \
	DECLARE_FUNCTION(execWriteInt); \
	DECLARE_FUNCTION(execWriteFloat); \
	DECLARE_FUNCTION(execPrintln); \
	DECLARE_FUNCTION(execPrint); \
	DECLARE_FUNCTION(execReadBytes); \
	DECLARE_FUNCTION(execReadByte); \
	DECLARE_FUNCTION(execReadInt); \
	DECLARE_FUNCTION(execReadFloat); \
	DECLARE_FUNCTION(execReadln); \
	DECLARE_FUNCTION(execReadString); \
	DECLARE_FUNCTION(execClose); \
	DECLARE_FUNCTION(execOpen); \
	DECLARE_FUNCTION(execFloatToBytes); \
	DECLARE_FUNCTION(execBytesToFloat); \
	DECLARE_FUNCTION(execIntToBytes); \
	DECLARE_FUNCTION(execBytesToInt); \
	DECLARE_FUNCTION(execOpenComPort);


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUSerial(); \
	friend struct Z_Construct_UClass_USerial_Statics; \
public: \
	DECLARE_CLASS(USerial, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/UE4Duino"), NO_API) \
	DECLARE_SERIALIZER(USerial)


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_INCLASS \
private: \
	static void StaticRegisterNativesUSerial(); \
	friend struct Z_Construct_UClass_USerial_Statics; \
public: \
	DECLARE_CLASS(USerial, UObject, COMPILED_IN_FLAGS(0), CASTCLASS_None, TEXT("/Script/UE4Duino"), NO_API) \
	DECLARE_SERIALIZER(USerial)


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API USerial(const FObjectInitializer& ObjectInitializer); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(USerial) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USerial); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USerial); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USerial(USerial&&); \
	NO_API USerial(const USerial&); \
public:


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API USerial(USerial&&); \
	NO_API USerial(const USerial&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, USerial); \
DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(USerial); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(USerial)


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_PRIVATE_PROPERTY_OFFSET
#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_28_PROLOG
#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_PRIVATE_PROPERTY_OFFSET \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_SPARSE_DATA \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_RPC_WRAPPERS \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_INCLASS \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_PRIVATE_PROPERTY_OFFSET \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_SPARSE_DATA \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_RPC_WRAPPERS_NO_PURE_DECLS \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_INCLASS_NO_PURE_DECLS \
	HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h_31_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> UE4DUINO_API UClass* StaticClass<class USerial>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID HostProject_Plugins_UE4Duino_Source_UE4Duino_Public_Serial_h


#define FOREACH_ENUM_ELINEEND(op) \
	op(ELineEnd::rn) \
	op(ELineEnd::n) \
	op(ELineEnd::r) \
	op(ELineEnd::nr) 

enum class ELineEnd : uint8;
template<> UE4DUINO_API UEnum* StaticEnum<ELineEnd>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
