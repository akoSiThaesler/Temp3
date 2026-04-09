// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CMCameraSettings.generated.h"

UCLASS(config = CameraManagerConfig)
class CAMERAMANAGER_API UCMCameraSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, config, Category = "Camera Manager General",meta=(ToolTip = "Shows details panel when a camera selected."))
	bool bShowDetailsPanel = true;

	UPROPERTY(EditAnywhere, config, Category = "Camera Manager General",meta=(ToolTip = "Auto pilot when a camera created."))
	bool bAutoPilotCamera = false;

	void SavePluginConfig();
	void LoadPluginConfig();
};
