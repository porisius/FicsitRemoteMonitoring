#include "NotificationLoader.h"

#include "FicsitRemoteMonitoringModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

bool UNotificationLoader::FileLoadString(FString LoadFile, FString& LoadString)
{
	return FFileHelper::LoadFileToString(LoadString, *(LoadFile));
}

bool UNotificationLoader::FileSaveString(FString SaveString, FString SaveFile)
{
	return FFileHelper::SaveStringToFile(SaveString, *(SaveFile));
}