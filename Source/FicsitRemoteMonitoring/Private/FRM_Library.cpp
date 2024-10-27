
#include "FRM_Library.h"
#include "FRM_Factory.h"

TSharedPtr<FJsonObject> UFRM_Library::getActorJSON(AActor* Actor) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double primaryX = Actor->GetActorLocation().X;
	long double primaryY = Actor->GetActorLocation().Y;
	long double primaryZ = Actor->GetActorLocation().Z;

	// UE rotations range from -180 to 180 with zero as "forward" and negative values "left".
	// In Satisfactory this results in a rotation of zero being due east.
	// FRM will normalise this to a range of 0 <= x < 360 with zero as due north.
	long double rotation = Actor->GetActorRotation().Yaw;
	rotation += 450;
	rotation = fmod(rotation, 360);

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));
	JLibrary->Values.Add("rotation", MakeShared<FJsonValueNumber>(rotation));
		
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

bool UFRM_Library::IsIntInRange(int32 Number, int32 LowerBound, int32 UpperBound)
{
	bool bIsMoreThanLowerBound = Number > LowerBound;
	bool bIsLessThanUpperBound = Number < UpperBound;

	return bIsMoreThanLowerBound && bIsLessThanUpperBound;
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> UFRM_Library::GetGroupedInventoryItems(const UFGInventoryComponent* Inventory)
{
	TArray<FInventoryStack> InventoryStacks;

	Inventory->GetInventoryStacks(InventoryStacks);

	return GetGroupedInventoryItems(InventoryStacks);
}

void UFRM_Library::GetGroupedInventoryItems(const UFGInventoryComponent* Inventory, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems)
{
	TArray<FInventoryStack> InventoryStacks;

	Inventory->GetInventoryStacks(InventoryStacks);

	GetGroupedInventoryItems(InventoryStacks, InventoryItems);
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> UFRM_Library::GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks)
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> InventoryItems;

	GetGroupedInventoryItems(InventoryStacks, InventoryItems);

	return InventoryItems;
}

void UFRM_Library::GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems)
{
	for (FInventoryStack Inventory : InventoryStacks) {
		auto ItemClass = Inventory.Item.GetItemClass();
		const auto Amount = Inventory.NumItems;

		if (InventoryItems.Contains(ItemClass)) {
			InventoryItems.Add(ItemClass) = Amount + InventoryItems.FindRef(ItemClass);
		}
		else {
			InventoryItems.Add(ItemClass) = Amount;
		}
	}
}

TArray<TSharedPtr<FJsonValue>> UFRM_Library::GetInventoryJSON(const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items)
{
	TArray<TSharedPtr<FJsonValue>> JInventoryArray;

	for (const TPair<TSubclassOf<UFGItemDescriptor>, int32> Item : Items) {
		TSharedPtr<FJsonObject> JInventory = MakeShared<FJsonObject>();

		JInventory->Values.Add("Name", MakeShared<FJsonValueString>((Item.Key.GetDefaultObject()->mDisplayName).ToString()));
		JInventory->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Item.Key)));
		JInventory->Values.Add("Amount", MakeShared<FJsonValueNumber>(Item.Value));

		JInventoryArray.Add(MakeShared<FJsonValueObject>(JInventory));
	}

	return JInventoryArray;
}

TSharedPtr<FJsonObject> UFRM_Library::GetResourceNodeJSON(AActor* Actor, const bool bIncludeFeatures)
{
	AFGResourceNode* ResourceNode = Cast<AFGResourceNode>(Actor);
	if (!ResourceNode) {
		return nullptr;
	}

	TSharedPtr<FJsonObject> JResourceNode = MakeShared<FJsonObject>();

	// get purity
	const EResourcePurity ResourcePurity = ResourceNode->GetResoucePurity();
	FString Purity = TEXT("Unknown");
	switch(ResourcePurity)
	{
		case RP_Inpure: Purity = TEXT("Impure"); break;
		case RP_Pure: Purity = TEXT("Pure"); break;
		case RP_Normal: Purity = TEXT("Normal"); break;
	}

	// get resource form
	FString ResourceForm = TEXT("Unknown");
	switch(ResourceNode->GetResourceForm())
	{
		case EResourceForm::RF_INVALID: ResourceForm = TEXT("Invalid"); break;
		case EResourceForm::RF_SOLID: ResourceForm = TEXT("Solid"); break;
		case EResourceForm::RF_LIQUID: ResourceForm = TEXT("Liquid"); break;
		case EResourceForm::RF_GAS: ResourceForm = TEXT("Gas"); break;
		case EResourceForm::RF_HEAT: ResourceForm = TEXT("Heat"); break;
	}

	// get resource node type
	FString ResourceNodeType = TEXT("Unknown");
	switch (ResourceNode->GetResourceNodeType()) {
		case EResourceNodeType::Geyser: ResourceNodeType = TEXT("Geyser"); break;
		case EResourceNodeType::FrackingCore: ResourceNodeType = TEXT("Fracking Core"); break;
		case EResourceNodeType::FrackingSatellite: ResourceNodeType = TEXT("Fracking Satellite"); break;
		case EResourceNodeType::Node: ResourceNodeType = TEXT("Node"); break;
	}
	
	FString ResourceName = ResourceNode->GetResourceName().ToString();
		
	JResourceNode->Values.Add("Name", MakeShared<FJsonValueString>(ResourceName));
	JResourceNode->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(ResourceNode->GetResourceClass())));
	JResourceNode->Values.Add("Purity", MakeShared<FJsonValueString>(Purity));
	JResourceNode->Values.Add("EnumPurity", MakeShared<FJsonValueString>(UEnum::GetValueAsString(ResourcePurity)));
	JResourceNode->Values.Add("ResourceForm", MakeShared<FJsonValueString>(ResourceForm));
	JResourceNode->Values.Add("NodeType", MakeShared<FJsonValueString>(ResourceNodeType));
	JResourceNode->Values.Add("Exploited", MakeShared<FJsonValueBoolean>(ResourceNode->IsOccupied()));
	JResourceNode->Values.Add("location", MakeShared<FJsonValueObject>(getActorJSON(Actor)));

	if (bIncludeFeatures) {
		JResourceNode->Values.Add("features", MakeShared<FJsonValueObject>(getActorFeaturesJSON(ResourceNode, ResourceName, "Resource Node")));
	}

	return JResourceNode;
}

TSharedPtr<FJsonValue> ConvertStringToFJsonValue(const FString& JsonString)
{
	TSharedPtr<FJsonValue> FJsonValue;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (FJsonSerializer::Deserialize(Reader, FJsonValue) && FJsonValue.IsValid())
	{
		return FJsonValue;
	}

	UE_LOGFMT(LogFRMDebug, Log, "Invalid JSON Detected: {0}", JsonString);

	// Handle error: return nullptr or some default value
	return nullptr;
}

TSharedPtr<FJsonObject> UFRM_Library::getPowerConsumptionJSON(UFGPowerInfoComponent* PowerInfo) {
	return UFRM_Library::getPowerConsumptionJSON(PowerInfo, 0);
}

TSharedPtr<FJsonObject> UFRM_Library::getPowerConsumptionJSON(UFGPowerInfoComponent* PowerInfo, float additionalPowerConsumed) {
	TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();
	int32 CircuitGroupID = -1;
	int32 CircuitID = -1;
	float PowerConsumed = additionalPowerConsumed;

	if (IsValid(PowerInfo)) {
		UFGPowerCircuit* PowerCircuit = PowerInfo->GetPowerCircuit();
		if (IsValid(PowerCircuit)) {
			CircuitGroupID = PowerCircuit->GetCircuitGroupID();
			CircuitID = PowerCircuit->GetCircuitID();
			PowerConsumed = PowerInfo->GetActualConsumption() + additionalPowerConsumed;
		}
	}
	JCircuit->Values.Add("CircuitGroupID", MakeShared<FJsonValueNumber>(CircuitGroupID));
	JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(CircuitID));
	JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(PowerConsumed));
	return JCircuit;
};