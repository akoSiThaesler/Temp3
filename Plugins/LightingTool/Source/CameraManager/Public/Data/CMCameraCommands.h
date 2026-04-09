// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "Framework/Commands/Commands.h"

class FCMCameraContextMenuCommands : public TCommands<FCMCameraContextMenuCommands>
{
public:
	FCMCameraContextMenuCommands() : TCommands<FCMCameraContextMenuCommands>(
	TEXT("Camera Manager UI Commands"),
	FText::FromString(TEXT("Camera Manager UI")),
	NAME_None,
	TEXT("Camera Manager Style")
	) {}

	virtual void RegisterCommands() override;
	
	TSharedPtr<FUICommandInfo> SavePreset;
	TSharedPtr<FUICommandInfo> LoadPreset;
	TSharedPtr<FUICommandInfo> ImportPresets;
	TSharedPtr<FUICommandInfo> ExportPresets;
	TSharedPtr<FUICommandInfo> ShowDetailsPanel;
	TSharedPtr<FUICommandInfo> AutoPilotCamera;
};

class FCMCameraLevelCommands : public TCommands<FCMCameraLevelCommands>
{
public:
	FCMCameraLevelCommands() : TCommands<FCMCameraLevelCommands>(
	TEXT("Camera Manager Level Commands"),
	FText::FromString(TEXT("Camera Manager Level")),
	NAME_None,
	TEXT("Camera Manager Style")
	) {}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> CreateCamera;
	TSharedPtr<FUICommandInfo> NextCamera;
	TSharedPtr<FUICommandInfo> PreviousCamera;
	TSharedPtr<FUICommandInfo> EjectPilotMode;
	TSharedPtr<FUICommandInfo> LockToggle;
};