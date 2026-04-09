// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UEditorUtilityWidget;

class FLightingToolModule : public IModuleInterface
{
public:
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void InitToolMenuCommands();
	void SetupPluginToolbarEntry();
	TSharedRef<class SWidget> CreateToolMenu() const;
	void AddToolbarExtension(class FToolBarBuilder& ToolBarBuilder);
	
	TSharedPtr<class FUICommandList> ToolMenuCommands;

public:
	void ToggleAutoLightMapWindow(); 
	void ToggleLightsToolWindow();
	void ToggleHDRIManagerWindow();
	void ToggleCameraManager();
	void ToggleLightRenderSettingsTolWindow();
	static void LaunchHelp();

public:
	static FName GetALToolTabID() { return ALToolTabID; }
	static FName GetLTToolTabID() { return LTToolTabID; }
	static FName GetLRToolTabID() { return LRToolTabID; }
	static FName getCMToolTabID() { return CMToolTabID; }
	static FName GetHDRIToolTabID() { return HDRIToolTabID; }
	
private:
	static FName ALToolTabID;
	static FName LTToolTabID;
	static FName LRToolTabID;
	static FName CMToolTabID;
	static FName HDRIToolTabID;
};