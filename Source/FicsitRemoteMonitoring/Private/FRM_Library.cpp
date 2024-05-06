
#include "FRM_Library.h"
#include "FRM_Factory.h"

TSharedPtr<FJsonObject> UFRM_Library::getActorJSON(AActor* Actor) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double ZActor = Actor->GetActorRotation().Quaternion().Z;

	long double primaryX = Actor->GetActorLocation().X;
	long double primaryY = Actor->GetActorLocation().Y;
	long double primaryZ = Actor->GetActorLocation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	//Because East is what Epic decided to call heading 0, with a -180 to 180 spread.	
	ZActor = ZActor + 450;
	if (ZActor > 360) {
		ZActor = ZActor - 360;
	}

	JLibrary->Values.Add("rotation", MakeShared<FJsonValueNumber>(ZActor));
		
	return JLibrary;

};

TSharedPtr<FJsonObject> UFRM_Library::getActorFactoryCompXYZ(UFGFactoryConnectionComponent* BeltPipe) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double primaryX = BeltPipe->GetRelativeTransform().GetTranslation().X;
	long double primaryY = BeltPipe->GetRelativeTransform().GetTranslation().Y;
	long double primaryZ = BeltPipe->GetRelativeTransform().GetTranslation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> UFRM_Library::getActorPipeXYZ(UFGPipeConnectionComponent* BeltPipe) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double primaryX = BeltPipe->GetRelativeTransform().GetTranslation().X;
	long double primaryY = BeltPipe->GetRelativeTransform().GetTranslation().Y;
	long double primaryZ = BeltPipe->GetRelativeTransform().GetTranslation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> UFRM_Library::getActorFeaturesJSON(AActor* Actor, FString DisplayName, FString TypeName) {

	TSharedPtr<FJsonObject> JProperties = MakeShared<FJsonObject>();

	JProperties->Values.Add("name", MakeShared<FJsonValueString>(DisplayName));
	JProperties->Values.Add("type", MakeShared<FJsonValueString>(TypeName));

	long double primaryX = Actor->GetActorLocation().X;
	long double primaryY = Actor->GetActorLocation().Y;
	long double primaryZ = Actor->GetActorLocation().Z;

	TSharedPtr<FJsonObject> JCoordinates = MakeShared<FJsonObject>();

	JCoordinates->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JCoordinates->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JCoordinates->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	TSharedPtr<FJsonObject> JGeometry = MakeShared<FJsonObject>();

	JGeometry->Values.Add("coordinates", MakeShared<FJsonValueObject>(JCoordinates));
	JGeometry->Values.Add("type", MakeShared<FJsonValueString>("Point"));

	TSharedPtr<FJsonObject> JFeatures = MakeShared<FJsonObject>();

	JFeatures->Values.Add("properties", MakeShared<FJsonValueObject>(JProperties));
	JFeatures->Values.Add("geometry", MakeShared<FJsonValueObject>(JGeometry));

	return JFeatures;

};

float UFRM_Library::SafeDivide(float NumberA, float NumberB) {

	if (NumberB == 0) {
		return 0;
	};

	float divide = NumberA / NumberB;

	return divide;

};

FString UFRM_Library::APItoJSON(TArray<TSharedPtr<FJsonValue>> JSONArray, UObject* WorldContext) {

	FString Write;
	auto config = FConfig_FactoryStruct::GetActiveConfig(WorldContext);

	if (config.JSONDebugMode) {
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Write);
		FJsonSerializer::Serialize(JSONArray, JsonWriter);
	} else {
		const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Write); //Our Writer Factory
		FJsonSerializer::Serialize(JSONArray, JsonWriter);
	}

	return Write;

}