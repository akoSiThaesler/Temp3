// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "Data/LTToolMenuCommands.h"
#include "Style/LTToolStyle.h"
#include "Framework/Commands/Commands.h"

#define LOCTEXT_NAMESPACE "FLightingToolModule"

void FLTToolMenuCommands::RegisterCommands()
{
	{
		const FText ALLabel = FText::FromString(TEXT("Light Map Tool"));
		const FText ALDesc = FText::FromString(TEXT("Light Map Tool"));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			OpenAutoLightMapTool,
			"LightMapTool",
			ALLabel,
			ALDesc,
			FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightMapIcon"),
			EUserInterfaceActionType::Button,
			FInputChord());
		
	}
	/*******************************************************************************/

	{
		const FText LTLabel = FText::FromString(TEXT("Lights Tool"));
		const FText LTDesc = FText::FromString(TEXT("Lights Tool"));
	
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			OpenLightsTool,
			"LightsTool",
			LTLabel,
			LTDesc,
			FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightsToolIcon"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}

	/*******************************************************************************/
	
	{
		const FText CameraLabel = FText::FromString(TEXT("Camera Manager"));
		const FText CameraDesc = FText::FromString(TEXT("Camera Manager Tool"));
	
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			OpenCameraManagerTool,
			"CameraManagerTool",
			CameraLabel,
			CameraDesc,
			FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.CameraManagerIcon"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}

	/*******************************************************************************/
	
	{
		const FText HDRILabel = FText::FromString(TEXT("HDRI Manager"));
		const FText HDRIDesc = FText::FromString(TEXT("HDRI Manager Tool"));
	
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			OpenHDRIManagerTool,
			"HDRIManagerTool",
			HDRILabel,
			HDRIDesc,
			FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.HDRIIcon"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}
	
	/*******************************************************************************/

	{
		const FText LRLabel = FText::FromString(TEXT("Light Render Settings"));
		const FText LRDesc = FText::FromString(TEXT("Light Render Settings Tool"));
	
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			OpenLightRenderSettingsTool,
			"LightRenderSettingsTool",
			LRLabel,
			LRDesc,
			FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightRenderSettingsIcon"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}
	
	/*******************************************************************************/
	
	const FText HelpLabel = FText::FromString(TEXT("Help..."));
	const FText HelpDesc = FText::FromString(TEXT("Go to Documentation"));
	
	FUICommandInfo::MakeCommandInfo(
		this->AsShared(),
		LaunchHelp,
		"LaunchHelp",
		HelpLabel,
		HelpDesc,
		FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.HelpIcon"),
		EUserInterfaceActionType::Button,
		FInputChord());
	
}

#undef LOCTEXT_NAMESPACE
