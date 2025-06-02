#include "getPipelineJunction.h"

#include "FGBuildableSubsystem.h"
#include "FGBuildablePipelineJunction.h"
#include "../../../Public/FRM_Library.h"
#include "Containers/Array.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"

class AFGBuildablePipelineJunction;
class AFGBuildableSubsystem;

TArray<TSharedPtr<FJsonValue>> UGetPipelineJunction::getPipeJunctions(UObject* WorldContext)
{
	TArray<TSharedPtr<FJsonValue>> JPipeJunctions;
	TArray<AFGBuildablePipelineJunction*> PipeJunctions;
	AFGBuildableSubsystem* BuildableSubsystem = AFGBuildableSubsystem::Get(WorldContext->GetWorld());
	BuildableSubsystem->GetTypedBuildable<AFGBuildablePipelineJunction>(PipeJunctions);

	for (AFGBuildablePipelineJunction* Junction : PipeJunctions)
	{
		JPipeJunctions.Add(MakeShared<FJsonValueObject>(UFRM_Library::CreateBuildableBaseJsonObject(Junction)));		
	}	
	return JPipeJunctions;
}
