// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CameraManager.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "CMCameraSettings.h"
#include "CMCameraUIManager.h"
#include "CMToolStyle.h"

#define LOCTEXT_NAMESPACE "FCameraManagerModule"


void FCameraManagerModule::StartupModule()
{
	FCMToolStyle::InitializeToolStyle();

	RegisterCameraSettings();

	FCameraUIManager::Initialize();
}

void FCameraManagerModule::ShutdownModule()
{
	FCameraUIManager::Shutdown();
	
	UnregisterCameraSettings();
	
	FCMToolStyle::ShutDownStyle();
}

void FCameraManagerModule::RegisterCameraSettings()
{
	UCMCameraSettings* Config = GetMutableDefault<UCMCameraSettings>();

	if(IsValid(Config))
	{
		Config->LoadPluginConfig();
	}
	
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Editor", "Plugins", "Camera Manager",
			LOCTEXT("CameraManagerSettingsName", "Camera Manager"),
			LOCTEXT("CameraManagerSettingsDescription", "Configure the settings for camera manager."),
			GetMutableDefault<UCMCameraSettings>());
	}
}

void FCameraManagerModule::UnregisterCameraSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "Camera Manager");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCameraManagerModule, CameraManager)