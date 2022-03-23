
#include "FRMBPLibrary.h"
#include "FicsitRemoteMonitoringModule.h"

void UFRMBPLibrary::SortIntArray(UPARAM(ref) TArray<int32>& IntArray, TArray<int32>& IntArraySorted)
{
	IntArray.Sort();
	IntArraySorted = IntArray;
}
void UFRMBPLibrary::SortFloatArray(UPARAM(ref) TArray<float>& FloatArray, TArray<float>& FloatArraySorted)
{
	FloatArray.Sort();
	FloatArraySorted = FloatArray;
}