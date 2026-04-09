// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMCameraSettings.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"


void UCMCameraSettings::SavePluginConfig()
{
	// Save the configuration in the plugin's config directory
	const FString PluginConfigDir = IPluginManager::Get().FindPlugin(TEXT("LightingTool"))->GetBaseDir() /"Config";
	FString PluginConfigFile = FPaths::Combine(PluginConfigDir, TEXT("CameraManagerConfig.ini"));

	SaveConfig(CPF_Config, *PluginConfigFile);
}

void UCMCameraSettings::LoadPluginConfig()
{
	const FString PluginConfigDir = IPluginManager::Get().FindPlugin(TEXT("LightingTool"))->GetBaseDir() /"Config";
	FString PluginConfigFile = FPaths::Combine(PluginConfigDir, TEXT("CameraManagerConfig.ini"));
	LoadConfig(GetClass(), *PluginConfigFile);
}
