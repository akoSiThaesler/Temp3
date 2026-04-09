// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "Data/CMCameraCommands.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FCameraManagerModule"

void FCMCameraContextMenuCommands::RegisterCommands()
{
	{
		const FText Label = FText::FromString(TEXT("Save As Preset"));
		const FText Desc = FText::FromString(TEXT("Saves the selected camera as a new preset."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			SavePreset,
			"SavePreset",
			Label,
			Desc,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Level.SaveIcon16x"),
			EUserInterfaceActionType::Button,
			FInputChord(EKeys::S, EModifierKey::Control));
	}
	/*******************************************************************************/

	{
		const FText Label = FText::FromString(TEXT("Load Preset"));
		const FText Desc = FText::FromString(TEXT("Loads a new preset onto the selected cameras."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			LoadPreset,
			"LoadPreset",
			Label,
			Desc,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
			EUserInterfaceActionType::Button,
			FInputChord(EKeys::L, EModifierKey::Control));
	}
	
	/*******************************************************************************/

	{
		const FText Label = FText::FromString(TEXT("Import Presets"));
		const FText Desc = FText::FromString(TEXT("Enables importing presets from outside the project. Select a previously exported file to import presets."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			ImportPresets,
			"ImportPresets",
			Label,
			Desc,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Import"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}

	/*******************************************************************************/

	{
		const FText Label = FText::FromString(TEXT("Export Presets"));
		const FText Desc = FText::FromString(TEXT("Enables exporting and sharing of selected presets outside the project."));
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			ExportPresets,
			"ExportPresets",
			Label,
			Desc,
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.ExportAll"),
			EUserInterfaceActionType::Button,
			FInputChord());
	}

	/*******************************************************************************/
	
	{
		const FText Label = FText::FromString(TEXT("Show Details"));
		const FText Desc = FText::FromString(TEXT("Shows details panel when a camera selected."));
		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			ShowDetailsPanel,
			"ShowDetails",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::ToggleButton,
			FInputChord());
	}

	/*******************************************************************************/
	
	{
		const FText Label = FText::FromString(TEXT("Auto Pilot"));
		const FText Desc = FText::FromString(TEXT("Auto pilot when a camera created."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			AutoPilotCamera,
			"AutoPilotCamera",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::ToggleButton,
			FInputChord());
	}
}

void FCMCameraLevelCommands::RegisterCommands()
{
	{
		const FText Label = FText::FromString(TEXT("Create Camera"));
		const FText Desc = FText::FromString(TEXT("Create a camera at the current viewport's view"));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			CreateCamera,
			"CreateCamera",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
FInputChord(EKeys::Q, EModifierKey::Control));

	}
	
	{
		const FText Label = FText::FromString(TEXT("Next Camera"));
		const FText Desc = FText::FromString(TEXT("Switches to the next camera in the list."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			NextCamera,
			"NextCamera",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
FInputChord(EKeys::MouseScrollUp, EModifierKey::Control), FInputChord(EKeys::Tab, EModifierKey::Control));

	}
	{
		const FText Label = FText::FromString(TEXT("Previous Camera"));
		const FText Desc = FText::FromString(TEXT("Switches to the previous camera in the list."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			PreviousCamera,
			"PreviousCamera",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
			FInputChord(EKeys::MouseScrollDown, EModifierKey::Control),FInputChord(EKeys::Tab, EModifierKey::Control | EModifierKey::Shift));
	}
	{
		const FText Label = FText::FromString(TEXT("Eject Pilot Mode"));
		const FText Desc = FText::FromString(TEXT("Exits pilot mode for the selected camera."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			EjectPilotMode,
			"EjectPilotMode",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
			FInputChord(EKeys::E, EModifierKey::Control));
	}
	{
		const FText Label = FText::FromString(TEXT("Toggle Lock"));
		const FText Desc = FText::FromString(TEXT("Toggles the lock state on the selected camera."));

		FUICommandInfo::MakeCommandInfo(
			this->AsShared(),
			LockToggle,
			"LockToggle",
			Label,
			Desc,
			FSlateIcon(),
			EUserInterfaceActionType::Button,
			FInputChord(EKeys::D, EModifierKey::Control | EModifierKey::Shift));
	}
}

#undef LOCTEXT_NAMESPACE
