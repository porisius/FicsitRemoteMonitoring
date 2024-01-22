
#include "FRM_Library.h"

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

TSharedPtr<FJsonObject> UFRM_Library::getActorFeaturesJSON(AActor* Actor, FString DisplayName) {

	TSharedPtr<FJsonObject> JFeatures = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JCoordinates = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JProperties = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> JGeometry = MakeShared<FJsonObject>();

	JProperties->Values.Add("name", MakeShared<FJsonValueString>(DisplayName));
	JProperties->Values.Add("type", MakeShared<FJsonValueString>(DisplayName));

	long double primaryX = Actor->GetActorLocation().X;
	long double primaryY = Actor->GetActorLocation().Y;
	long double primaryZ = Actor->GetActorLocation().Z;

	JCoordinates->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JCoordinates->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JCoordinates->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	JGeometry->Values.Add("coordinates", MakeShared<FJsonValueObject>(JCoordinates));
	JGeometry->Values.Add("type", MakeShared<FJsonValueString>("Point"));

	JFeatures->Values.Add("properties", MakeShared<FJsonValueObject>(JProperties));
	JFeatures->Values.Add("geometry", MakeShared<FJsonValueObject>(JGeometry));

	return JFeatures;

};