// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTToolData.h"
#include "Editor.h"
#include "LTFunctions.h"

void ULTToolData::DataParamChanged()
{
	SaveConfig();
}


bool ULTToolData::ToggleAndGetListViewExpandStatus()
{
	bIsListViewExpanded = !bIsListViewExpanded;
	return bIsListViewExpanded;
}

void ULTToolData::SetIdealDensity(const float& InIdealDensity)
{
	IdealLightMapDensity = InIdealDensity;
	
	if(GEditor->MaxLightMapDensity <= IdealLightMapDensity)
	{
		GEditor->MaxLightMapDensity , MaxLightMapDensity = IdealLightMapDensity;
	}
}

void ULTToolData::SetMaxAllowedDensity(const float& InMaxAllowedDensity)
{
	MaxLightMapDensity = InMaxAllowedDensity;
	
	if(MaxLightMapDensity <= IdealLightMapDensity)
	{
		MaxLightMapDensity = IdealLightMapDensity;
	}
	
	ULTFunctions::SetMaxLightmapDensity(MaxLightMapDensity);
}

void ULTToolData::SetMaxAllowedResolution(const int32& InMaxAllowedResolution)
{
	MaxAllowedResolution = InMaxAllowedResolution;
	
	if (MaxAllowedResolution % 4 != 0)
	{
		MaxAllowedResolution += 4 - (MaxAllowedResolution % 4);
	}
}

void ULTToolData::CheckForMaximumDensity(const float& InIdealDensity)
{
	if(GEditor->MaxLightMapDensity <= InIdealDensity)
	{
		GEditor->MaxLightMapDensity = InIdealDensity;
	}
}

