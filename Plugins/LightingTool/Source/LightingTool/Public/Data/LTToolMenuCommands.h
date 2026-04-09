// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "Framework/Commands/Commands.h"

class FLTToolMenuCommands : public TCommands<FLTToolMenuCommands>
{
public:
	FLTToolMenuCommands() : TCommands<FLTToolMenuCommands>(
	TEXT("Lighting Tool"),
	FText::FromString(TEXT("Leartes Studios Lighting Tool Commands")),
	NAME_None,
	TEXT("Lighting Tool")
	) {}

	virtual void RegisterCommands() override;
	
	TSharedPtr<FUICommandInfo> OpenAutoLightMapTool;
	TSharedPtr<FUICommandInfo> OpenLightsTool;
	TSharedPtr<FUICommandInfo> OpenLightRenderSettingsTool;
	TSharedPtr<FUICommandInfo> OpenCameraManagerTool;
	TSharedPtr<FUICommandInfo> OpenHDRIManagerTool;
	TSharedPtr<FUICommandInfo> LaunchHelp;
	
	TArray<TSharedPtr<FUICommandInfo>> ToolCommands;
};

