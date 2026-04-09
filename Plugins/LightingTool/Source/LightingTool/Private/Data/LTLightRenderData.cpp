// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTLightRenderData.h"

void ULTLightRenderData::SavePresetData()
{
	UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(ULTLightRenderData, Preset)),GetDefaultConfigFilename());
}

void ULTLightRenderData::LoadPresetData()
{
	LoadConfig();
}
