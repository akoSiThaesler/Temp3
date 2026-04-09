// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/Views/STreeView.h"

class ACineCameraActor;

DECLARE_DELEGATE_OneParam(FOnPilotButtonClicked,TWeakObjectPtr<ACineCameraActor>)
DECLARE_DELEGATE_TwoParams(FOnLockStateChanged,ACineCameraActor* CineCameraActor,bool NewState)


DECLARE_DELEGATE(FOnRenameRequested)
DECLARE_DELEGATE(FOnPresetChanged)

struct FCameraViewItemInfo
{
	//Data
	TWeakObjectPtr<ACineCameraActor> CameraPtr;
	FName PresetName;
	bool bPilotState;
	
	//Widget To Outside
	FOnPilotButtonClicked OnPilotButtonClicked;
	FOnLockStateChanged OnLockStateChanged;

	//Outside To Widget
	FOnRenameRequested OnRenameRequested;
	FOnPresetChanged OnPresetChanged;
	
	// Default constructor
	FCameraViewItemInfo()
	: CameraPtr(nullptr), PresetName(NAME_None), bPilotState(false)
	{
	}

	// Full parameter constructor
	FCameraViewItemInfo(TWeakObjectPtr<ACineCameraActor> InCineCameraActor,FName InPresetName, bool InPilotState)
	: CameraPtr(InCineCameraActor), PresetName(InPresetName), bPilotState(InPilotState)
	{
	}
};

/** Type definition for shared pointers to instances of FEventGraphSample. */
typedef TSharedPtr<FCameraViewItemInfo> FCameraViewItemInfoPtr;

DECLARE_DELEGATE_OneParam(FOnPresetSelectedForLoadOnDropDown, FCameraViewItemInfoPtr);

class FCameraViewItemModel : public TSharedFromThis<FCameraViewItemModel>
{
public:
	FCameraViewItemInfoPtr CameraViewItemInfo;
	
	// Parameterized constructor
	FCameraViewItemModel(const FCameraViewItemInfoPtr& InCameraViewItemInfo)
		: CameraViewItemInfo(InCameraViewItemInfo)
	{
	}
};

typedef TSharedPtr<FCameraViewItemModel> FCameraViewItemModelPtr;

namespace CameraTreeTreeColumns
{
	/** IDs for list columns */
	static const FName ColumnID_Pilot("Pilot");
	static const FName ColumnID_Camera("Camera");
	static const FName ColumnID_Preset("Preset");
	static const FName ColumnID_LockState("Lock");
}


/** A tree row representing a foliage type in the palette */
class SSCameraViewItemRow : public SMultiColumnTableRow<FCameraViewItemInfoPtr>
{
public:
	SLATE_BEGIN_ARGS(SSCameraViewItemRow) {}
		
	SLATE_ARGUMENT(TArray<TSharedPtr<FString>>*, PresetData)

	SLATE_ARGUMENT(TSharedPtr<FString>, InFilterText)

	SLATE_EVENT(FOnPresetSelectedForLoadOnDropDown, OnPresetSelectedForLoadDropDown)

	SLATE_END_ARGS()
	
	void StartRenamingProcess() const;
	void RefreshSelectedPreset() const;
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FCameraViewItemModel>& InModel);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
	static TSharedPtr<FString> FindPresetName(const TArray<TSharedPtr<FString>>& InPresetNames, const FName& InPresetName);
	
	TSharedPtr<class SWidgetSwitcher> NameWidgetSwitcher;
	TSharedPtr<class SInlineEditableTextBlock> CameraNameEditableText;
	TArray<TSharedPtr<FString>>* PresetNames;

private:
	FOnPresetSelectedForLoadOnDropDown OnPresetSelectedForLoadDropDown;
	
	TSharedPtr<FCameraViewItemModel> Model;
	TSharedPtr<FString> FilterText;
	TSharedPtr<class STextComboBox> PresetComboBox;
};
