// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define CM_PRESET_TAG_TITLE TEXT("CM_Preset_Tag:")


class FCameraManagerModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static void RegisterCameraSettings();
	void UnregisterCameraSettings();
};
