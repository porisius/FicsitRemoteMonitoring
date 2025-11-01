#include "RemoteMonitoringLibrary.h"

#include "Config_FactoryStruct.h"
#include "Drones.h"
#include "FGBuildableCircuitSwitch.h"
#include "FGBuildableDroneStation.h"
#include "FGBuildableRailroadStation.h"
#include "FGBuildableSubsystem.h"
#include "FGCharacterPlayer.h"
#include "FGFactoryConnectionComponent.h"
#include "FGInventoryLibrary.h"
#include "FGPipeConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "FGPowerInfoComponent.h"
#include "FGPowerShardDescriptor.h"
#include "FGResourceNode.h"
#include "FGSchematicManager.h"
#include "FGTrainStationIdentifier.h"
#include "FicsitRemoteMonitoring.h"
#include "FicsitRemoteMonitoringModule.h"
#include "Research.h"
#include "StructuredLog.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getActorJSON(AActor* Actor) {

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

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getActorFactoryCompXYZ(AFGBuildable* BeltPipe, UFGFactoryConnectionComponent* ConnectionComponent) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	const long double actorX = BeltPipe->GetActorLocation().X;
	const long double actorY = BeltPipe->GetActorLocation().Y;
	const long double actorZ = BeltPipe->GetActorLocation().Z;
	
	const long double connectionX = ConnectionComponent->GetRelativeTransform().GetTranslation().X;
	const long double connectionY = ConnectionComponent->GetRelativeTransform().GetTranslation().Y;
	const long double connectionZ = ConnectionComponent->GetRelativeTransform().GetTranslation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(actorX + connectionX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(actorY + connectionY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(actorZ + connectionZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::ConvertVectorToFJsonObject(FVector Vector) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double primaryX = Vector.X;
	long double primaryY = Vector.Y;
	long double primaryZ = Vector.Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::ConvertVectorToFJsonObject(FIntVector Vector) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	long double primaryX = Vector.X;
	long double primaryY = Vector.Y;
	long double primaryZ = Vector.Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(primaryX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(primaryY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(primaryZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::ColorSlotToJson(AFGBuildable* Buildable)
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();

	TArray<float> FactoryCustomizationData = Buildable->GetCustomizationData_Implementation().Data;

	FLinearColor PrimaryColor = FLinearColor( FactoryCustomizationData[0],
											  FactoryCustomizationData[1],
											  FactoryCustomizationData[2],
											  0.0);

	FLinearColor SecondaryColor = FLinearColor(FactoryCustomizationData[3],
											   FactoryCustomizationData[4],
											   FactoryCustomizationData[5],
											   0.0);
	
	Json->SetStringField("PrimaryColor", LinearColorToHex(PrimaryColor));
	Json->SetStringField("SecondaryColor", LinearColorToHex(SecondaryColor));

	return Json;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::FBoxToJson(AFGBuildable* Buildable, const FBox& Box)
{
	TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
	
	const long double actorX = Buildable->GetActorLocation().X;
	const long double actorY = Buildable->GetActorLocation().Y;
	const long double actorZ = Buildable->GetActorLocation().Z;
	
	TSharedPtr<FJsonObject> MinObj = MakeShared<FJsonObject>();
	MinObj->SetNumberField("x", actorX + Box.Min.X);
	MinObj->SetNumberField("y", actorY + Box.Min.Y);
	MinObj->SetNumberField("z", actorZ + Box.Min.Z);
	Json->SetObjectField("min", MinObj);

	TSharedPtr<FJsonObject> MaxObj = MakeShared<FJsonObject>();
	MaxObj->SetNumberField("x", actorX + Box.Max.X);
	MaxObj->SetNumberField("y", actorY + Box.Max.Y);
	MaxObj->SetNumberField("z", actorZ + Box.Max.Z);
	Json->SetObjectField("max", MaxObj);

	return Json;
}

TArray<TSharedPtr<FJsonValue>> URemoteMonitoringLibrary::SplineToJSON(USplineComponent* SplineComp, float SampleDistance)
{
	TArray<TSharedPtr<FJsonValue>> JSpineArray;
	
	const float SplineLength = SplineComp->GetSplineLength();
	
	for (float Distance = 0.f; Distance <= SplineLength; Distance += SampleDistance)
	{
		FVector Position = SplineComp->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		TSharedPtr<FJsonObject> PointJson = MakeShareable(new FJsonObject);
		PointJson->SetNumberField(TEXT("x"), Position.X);
		PointJson->SetNumberField(TEXT("y"), Position.Y);
		PointJson->SetNumberField(TEXT("z"), Position.Z);

		JSpineArray.Add(MakeShareable(new FJsonValueObject(PointJson)));
	}

	return JSpineArray;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getActorPipeXYZ(AFGBuildable* BeltPipe, UFGPipeConnectionComponent* ConnectionComponent) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	const long double actorX = BeltPipe->GetActorLocation().X;
	const long double actorY = BeltPipe->GetActorLocation().Y;
	const long double actorZ = BeltPipe->GetActorLocation().Z;
	
	const long double connectionX = ConnectionComponent->GetRelativeTransform().GetTranslation().X;
	const long double connectionY = ConnectionComponent->GetRelativeTransform().GetTranslation().Y;
	const long double connectionZ = ConnectionComponent->GetRelativeTransform().GetTranslation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(actorX + connectionX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(actorY + connectionY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(actorZ + connectionZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getActorHyperXYZ(AFGBuildable* HyperTube, UFGPipeConnectionComponentBase* ConnectionComponent) {

	TSharedPtr<FJsonObject> JLibrary = MakeShared<FJsonObject>();

	const long double actorX = HyperTube->GetActorLocation().X;
	const long double actorY = HyperTube->GetActorLocation().Y;
	const long double actorZ = HyperTube->GetActorLocation().Z;
	
	const long double connectionX = ConnectionComponent->GetRelativeTransform().GetTranslation().X;
	const long double connectionY = ConnectionComponent->GetRelativeTransform().GetTranslation().Y;
	const long double connectionZ = ConnectionComponent->GetRelativeTransform().GetTranslation().Z;

	JLibrary->Values.Add("x", MakeShared<FJsonValueNumber>(actorX + connectionX));
	JLibrary->Values.Add("y", MakeShared<FJsonValueNumber>(actorY + connectionY));
	JLibrary->Values.Add("z", MakeShared<FJsonValueNumber>(actorZ + connectionZ));

	return JLibrary;

};

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getActorFeaturesJSON(AActor* Actor, FString DisplayName, FString TypeName) {

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

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetActorLineFeaturesJSON(FVector PointOne, FVector PointTwo, FString DisplayName, FString TypeName) {

	TSharedRef<FJsonObject> JProperties = MakeShareable(new FJsonObject());

	JProperties->SetStringField("name", DisplayName);
	JProperties->SetStringField("type", TypeName);

	// Coordinates array with X, Y, Z (longitude, latitude, altitude)
	TArray<TSharedPtr<FJsonValue>> CoordinatesArray;
	TArray<TSharedPtr<FJsonValue>> PointOneArray;
	PointOneArray.Add(MakeShareable(new FJsonValueNumber(PointOne.X)));
	PointOneArray.Add(MakeShareable(new FJsonValueNumber(PointOne.Y)));
	PointOneArray.Add(MakeShareable(new FJsonValueNumber(PointOne.Z)));

	TArray<TSharedPtr<FJsonValue>> PointTwoArray;
	PointTwoArray.Add(MakeShareable(new FJsonValueNumber(PointTwo.X)));
	PointTwoArray.Add(MakeShareable(new FJsonValueNumber(PointTwo.Y)));
	PointTwoArray.Add(MakeShareable(new FJsonValueNumber(PointTwo.Z)));

	CoordinatesArray.Add(MakeShareable(new FJsonValueArray(PointOneArray)));
	CoordinatesArray.Add(MakeShareable(new FJsonValueArray(PointTwoArray)));

	TSharedRef<FJsonObject> JGeometry = MakeShareable(new FJsonObject());

	JGeometry->SetArrayField("coordinates", CoordinatesArray);
	JGeometry->SetStringField("type", "LineString");

	TSharedRef<FJsonObject> JFeatures = MakeShareable(new FJsonObject());

	JFeatures->SetObjectField("properties", JProperties);
	JFeatures->SetObjectField("geometry", JGeometry);

	return JFeatures;

}

void URemoteMonitoringLibrary::GetOverclockingItemsFromInventory(const UFGInventoryComponent* Inventory, int32& Somersloops, int32& PowerShards)
{
	if (!Inventory) return;

	TArray<FInventoryStack> Stacks;
	Inventory->GetInventoryStacks(Stacks);
	for (const FInventoryStack& Stack : Stacks)
	{
		TSubclassOf<UFGItemDescriptor> ItemClass = Stack.Item.GetItemClass();
		UFGPowerShardDescriptor* PowerShardDescriptor = ItemClass->GetDefaultObject<UFGPowerShardDescriptor>();
		if (!PowerShardDescriptor) continue;

		EPowerShardType ShardType = UFGPowerShardDescriptor::GetPowerShardType(PowerShardDescriptor->GetClass());
		if (ShardType == EPowerShardType::PST_ProductionBoost)
		{
			Somersloops += Stack.NumItems;
		}
		else if (ShardType == EPowerShardType::PST_Overclock)
		{
			PowerShards += Stack.NumItems;
		}
	}
}

FString URemoteMonitoringLibrary::APItoJSON(TArray<TSharedPtr<FJsonValue>> JSONArray, UObject* WorldContext) {

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

bool URemoteMonitoringLibrary::IsIntInRange(int32 Number, int32 LowerBound, int32 UpperBound)
{
	bool bIsMoreThanLowerBound = Number > LowerBound;
	bool bIsLessThanUpperBound = Number < UpperBound;

	return bIsMoreThanLowerBound && bIsLessThanUpperBound;
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> URemoteMonitoringLibrary::GetGroupedInventoryItems(const UFGInventoryComponent* Inventory)
{
	TArray<FInventoryStack> InventoryStacks;

	Inventory->GetInventoryStacks(InventoryStacks);

	return GetGroupedInventoryItems(InventoryStacks);
}

void URemoteMonitoringLibrary::GetGroupedInventoryItems(const UFGInventoryComponent* Inventory, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems)
{
	TArray<FInventoryStack> InventoryStacks;

	Inventory->GetInventoryStacks(InventoryStacks);

	GetGroupedInventoryItems(InventoryStacks, InventoryItems);
}

TMap<TSubclassOf<UFGItemDescriptor>, int32> URemoteMonitoringLibrary::GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks)
{
	TMap<TSubclassOf<UFGItemDescriptor>, int32> InventoryItems;

	GetGroupedInventoryItems(InventoryStacks, InventoryItems);

	return InventoryItems;
}

void URemoteMonitoringLibrary::GetGroupedInventoryItems(const TArray<FInventoryStack>& InventoryStacks, TMap<TSubclassOf<UFGItemDescriptor>, int32>& InventoryItems)
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

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetItemValueObject(const FItemAmount Item, float StackSizeMuliplier)
{
	return GetItemValueObject(Item.ItemClass, Item.Amount, StackSizeMuliplier);
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetItemValueObject(const FInventoryStack& Item, float StackSizeMuliplier)
{
	return GetItemValueObject(Item.Item.GetItemClass(), Item.NumItems, StackSizeMuliplier);
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetItemValueObject(const TSubclassOf<UFGItemDescriptor>& Item, const int Amount, float StackSizeMuliplier)
{
	TSharedPtr<FJsonObject> JItem = MakeShared<FJsonObject>();

	JItem->Values.Add("Name", MakeShared<FJsonValueString>(UFGItemDescriptor::GetItemName(Item).ToString()));
	JItem->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Item)));
	JItem->Values.Add("Amount", MakeShared<FJsonValueNumber>(
		UFGInventoryLibrary::GetAmountConvertedByForm(Amount, UFGItemDescriptor::GetForm(Item))
	));
	JItem->Values.Add("MaxAmount", MakeShared<FJsonValueNumber>(
		UFGInventoryLibrary::GetAmountConvertedByForm(UFGItemDescriptor::GetStackSize(Item) * StackSizeMuliplier, UFGItemDescriptor::GetForm(Item) )
	));

	return JItem;
}

TArray<TSharedPtr<FJsonValue>> URemoteMonitoringLibrary::GetInventoryJSON(const TArray<FItemAmount>& Items, float StackSizeMuliplier)
{
	TArray<TSharedPtr<FJsonValue>> JInventoryArray;

	for (const auto Item : Items) {
		JInventoryArray.Add(MakeShared<FJsonValueObject>(GetItemValueObject(Item.ItemClass, Item.Amount, StackSizeMuliplier)));
	}

	return JInventoryArray;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetSchematicJson(AFicsitRemoteMonitoring* ModSubsystem, UObject* WorldContext, TSubclassOf<UFGSchematic> Schematic)
{
	TSharedPtr<FJsonObject> JSchematic = CreateBaseJsonObject(Schematic);
	TArray<TSharedPtr<FJsonValue>> JRecipeArray;
	TArray<TSubclassOf<UFGRecipe>> Recipes;

	bool Purchased = false;
	bool HasUnlocks = false;
	bool LockedAny = false;
	bool LockedTutorial = false;
	bool LockedDependent = false;
	bool LockedPhase = false;
	bool Tutorial = false;
			
	ModSubsystem->SchematicToRecipes_BIE(WorldContext, Schematic, Recipes, Purchased, HasUnlocks, LockedAny, LockedTutorial, LockedDependent, LockedPhase, Tutorial);

	for (const TSubclassOf<UFGRecipe> Recipe : Recipes) {
		TSharedPtr<FJsonObject> JRecipe = UResearch::getRecipe(WorldContext, Recipe);
		JRecipeArray.Add(MakeShared<FJsonValueObject>(JRecipe));
	}

	FString SchematicType;
	switch (UFGSchematic::GetType(Schematic)) {
		case ESchematicType::EST_Alternate: SchematicType = TEXT("Alternate");
			break;
		case ESchematicType::EST_Cheat: SchematicType = TEXT("Cheat");
			break;
		case ESchematicType::EST_Custom: SchematicType = TEXT("Custom");
			break;
		case ESchematicType::EST_HardDrive: SchematicType = TEXT("Hard Drive");
			break;
		case ESchematicType::EST_MAM: SchematicType = TEXT("M.A.M.");
			break;
		case ESchematicType::EST_Milestone: SchematicType = TEXT("Milestone");
			break;
		case ESchematicType::EST_Prototype: SchematicType = TEXT("Prototype");
			break;
		case ESchematicType::EST_ResourceSink: SchematicType = TEXT("Resource Sink");
			break;
		case ESchematicType::EST_Story: SchematicType = TEXT("Story");
			break;
		case ESchematicType::EST_Tutorial: SchematicType = TEXT("Tutorial");
	}

	JSchematic->Values.Add("Name", MakeShared<FJsonValueString>(UFGSchematic::GetSchematicDisplayName(Schematic).ToString()));
	JSchematic->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Schematic)));
	JSchematic->Values.Add("TechTier", MakeShared<FJsonValueNumber>(UFGSchematic::GetTechTier(Schematic)));
	JSchematic->Values.Add("Type", MakeShared<FJsonValueString>(SchematicType));
	JSchematic->Values.Add("Recipes", MakeShared<FJsonValueArray>(JRecipeArray));
	JSchematic->Values.Add("HasUnlocks", MakeShared<FJsonValueBoolean>(HasUnlocks));
	JSchematic->Values.Add("Locked", MakeShared<FJsonValueBoolean>(LockedAny));
	JSchematic->Values.Add("Purchased", MakeShared<FJsonValueBoolean>(Purchased));
	JSchematic->Values.Add("DepLocked", MakeShared<FJsonValueBoolean>(LockedDependent));
	JSchematic->Values.Add("LockedTutorial", MakeShared<FJsonValueBoolean>(LockedTutorial));
	JSchematic->Values.Add("LockedPhase", MakeShared<FJsonValueBoolean>(LockedPhase));
	JSchematic->Values.Add("Tutorial", MakeShared<FJsonValueBoolean>(Tutorial));

	TArray<TSharedPtr<FJsonValue>> JCosts;
	TArray<FItemAmount> ItemsPaid = AFGSchematicManager::Get(WorldContext)->GetPaidOffCostFor(Schematic);
	TArray<FItemAmount> ItemsCost = UFGSchematic::GetCost(Schematic);

	for (FItemAmount ItemCost : ItemsCost)
	{
		TSharedPtr<FJsonObject> JCost = GetItemValueObject(ItemCost.ItemClass, ItemCost.Amount);
	
		// Probably an easier way of doing this...
		int32 MilestonePaid = 0;
		for (FItemAmount ItemPaid : ItemsPaid)
		{
			if (ItemCost.ItemClass == ItemPaid.ItemClass)
			{
				MilestonePaid = ItemPaid.Amount;
				break;
			}
		}
		
		JCost->Values.Add("RemainingCost", MakeShared<FJsonValueNumber>(ItemCost.Amount - MilestonePaid));
		JCost->Values.Add("TotalCost", MakeShared<FJsonValueNumber>(ItemCost.Amount));
	
		JCosts.Add(MakeShared<FJsonValueObject>(JCost));
	}

	JSchematic->Values.Add("Cost", MakeShared<FJsonValueArray>(JCosts));

	return JSchematic;
}

TArray<TSharedPtr<FJsonValue>> URemoteMonitoringLibrary::GetInventoryJSON(const TMap<TSubclassOf<UFGItemDescriptor>, int32>& Items, float StackSizeMuliplier)
{
	TArray<TSharedPtr<FJsonValue>> JInventoryArray;

	for (const TPair<TSubclassOf<UFGItemDescriptor>, int32> Item : Items) {
		JInventoryArray.Add(MakeShared<FJsonValueObject>(GetItemValueObject(Item.Key, Item.Value, StackSizeMuliplier)));
	}

	return JInventoryArray;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::CreateBaseJsonObject(const UObject* Actor)
{
	TSharedPtr<FJsonObject> JObject = MakeShared<FJsonObject>();
	JObject->Values.Add("ID", MakeShared<FJsonValueString>(Actor->GetName()));

	return JObject;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::CreateBuildableBaseJsonObject(AFGBuildable* Buildable)
{
	TSharedPtr<FJsonObject> JObject = MakeShared<FJsonObject>();
	JObject->Values.Add("ID", MakeShared<FJsonValueString>(Buildable->GetName()));
	
	if (AFGBuildableDroneStation* DroneStation = Cast<AFGBuildableDroneStation>(Buildable))
	{
		JObject->Values.Add("Name", MakeShared<FJsonValueString>(UDrones::getDronePortName(DroneStation)));
	} else if (AFGBuildableRailroadStation* RailroadStation = Cast<AFGBuildableRailroadStation>(Buildable))
	{
		if (AFGTrainStationIdentifier* StationIdentifier = RailroadStation->GetStationIdentifier())
		{
			JObject->Values.Add("Name", MakeShared<FJsonValueString>(StationIdentifier->GetStationName().ToString()));
		}
	} else 	if (AFGBuildableCircuitSwitch* PowerSwitch = Cast<AFGBuildableCircuitSwitch>(Buildable))
	{
		JObject->Values.Add("Name", MakeShared<FJsonValueString>(PowerSwitch->GetBuildingTag_Implementation()));	
	} else
	{
		JObject->Values.Add("Name", MakeShared<FJsonValueString>(Buildable->mDisplayName.ToString()));
	}	
	
	JObject->Values.Add("ClassName", MakeShared<FJsonValueString>(UKismetSystemLibrary::GetClassDisplayName(Buildable->GetClass())));
	JObject->Values.Add("location", MakeShared<FJsonValueObject>(URemoteMonitoringLibrary::getActorJSON(Buildable)));
	JObject->Values.Add("BoundingBox", MakeShared<FJsonValueObject>(FBoxToJson(Buildable, Buildable->GetCombinedClearanceBox())));
	JObject->Values.Add("ColorSlot", MakeShared<FJsonValueObject>(ColorSlotToJson(Buildable)));

	return JObject;
}

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::GetResourceNodeJSON(AActor* Actor, const bool bIncludeFeatures)
{
	AFGResourceNode* ResourceNode = Cast<AFGResourceNode>(Actor);
	if (!ResourceNode) {
		return nullptr;
	}

	TSharedPtr<FJsonObject> JResourceNode = CreateBaseJsonObject(Actor);

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

TSharedPtr<FJsonObject> URemoteMonitoringLibrary::getPowerConsumptionJSON(UFGPowerInfoComponent* PowerInfo) {
	TSharedPtr<FJsonObject> JCircuit = MakeShared<FJsonObject>();
	int32 CircuitGroupID = -1;
	int32 CircuitID = -1;
	bool FuseTriggered = false;
	float PowerConsumed = 0;
	float MaxPowerConsumed = 0;

	if (IsValid(PowerInfo)) {
		UFGPowerCircuit* PowerCircuit = PowerInfo->GetPowerCircuit();
		if (IsValid(PowerCircuit)) {
			CircuitGroupID = PowerCircuit->GetCircuitGroupID();
			CircuitID = PowerCircuit->GetCircuitID();
			FuseTriggered = PowerCircuit->IsFuseTriggered();
			PowerConsumed = PowerInfo->GetActualConsumption();
			MaxPowerConsumed = PowerInfo->GetMaximumTargetConsumption();
		}
	}
	
	JCircuit->Values.Add("CircuitGroupID", MakeShared<FJsonValueNumber>(CircuitGroupID));
	JCircuit->Values.Add("CircuitID", MakeShared<FJsonValueNumber>(CircuitID));
	JCircuit->Values.Add("FuseTriggered", MakeShared<FJsonValueBoolean>(FuseTriggered));
	JCircuit->Values.Add("PowerConsumed", MakeShared<FJsonValueNumber>(PowerConsumed));
	JCircuit->Values.Add("MaxPowerConsumed", MakeShared<FJsonValueNumber>(MaxPowerConsumed));

	return JCircuit;
};

FString URemoteMonitoringLibrary::GetPlayerName(AFGCharacterPlayer* Character)
{
	FString CachedPlayerName = Character->GetCachedPlayerName();
	if (!CachedPlayerName.IsEmpty() && CachedPlayerName != " ")
	{
		return CachedPlayerName;
	}

	if (Character->mPlayerNames.Num())
	{
		return Character->mPlayerNames[0].PlayerName;
	}

	const APlayerState* PlayerState = Character->GetPlayerState();
	if (!IsValid(PlayerState))
	{
		return "";
	}

	FString PlayerName = PlayerState->GetPlayerName();

	if (!PlayerName.IsEmpty() || PlayerName != " ")
	{
		return PlayerName;
	}
	
	return "";
}

TSharedPtr<FJsonValueArray> URemoteMonitoringLibrary::GetSplineVector(TArray<FSplinePointData> SplinePointDatas)
{
	TArray<TSharedPtr<FJsonValue>> SplineVectors;

	for (FSplinePointData SplinePointData : SplinePointDatas)
	{
		SplineVectors.Add(MakeShared<FJsonValueObject>(ConvertVectorToFJsonObject(SplinePointData.Location)));
	}
	
	return MakeShared<FJsonValueArray>(SplineVectors);
}