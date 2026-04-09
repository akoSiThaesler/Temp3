// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"
#include "CMMisc.h"
#include "Widgets/SWindow.h"

DECLARE_DELEGATE(FOnSelectionStateChanged); 

struct FPresetPipelineViewItemInfo
{
	bool bIsSelected;
	bool bIsAlreadyAvailable;
	FName PresetName;
	FName Description;
	
	// Default constructor
	// Initializes PresetName to NAME_None
	FPresetPipelineViewItemInfo()
		: bIsSelected(true), bIsAlreadyAvailable(false), PresetName(NAME_None), Description(NAME_None)
	{
	}

	// Full parameter constructor
	FPresetPipelineViewItemInfo(FName InPresetName, FName InDescription,bool bInIsAlreadyAvailable = false)
		: bIsSelected(true), bIsAlreadyAvailable(bInIsAlreadyAvailable), PresetName(InPresetName), Description(InDescription)
	{
	}
};

/** Type definition for shared pointers to instances of FEventGraphSample. */
typedef TSharedPtr<FPresetPipelineViewItemInfo> FPresetPipelineViewItemInfoPtr;

class FPresetPipelineViewItemModel : public TSharedFromThis<FPresetPipelineViewItemModel>
{
public:
	FPresetPipelineViewItemInfoPtr PresetPipelineViewItemInfoPtr;
	
	// Parameterized constructor
	FPresetPipelineViewItemModel(const FPresetPipelineViewItemInfoPtr& InPresetPipelineViewItemInfoPtr)
		: PresetPipelineViewItemInfoPtr(InPresetPipelineViewItemInfoPtr)
	{
	}
};

typedef TSharedPtr<FPresetPipelineViewItemModel> FPresetPipelineViewItemModelPtr;

namespace PresetPipelineTreeColumns
{
	/** IDs for list columns */
	static const FName ColumnID_Select("Select");
	static const FName ColumnID_Preset("Preset");
	static const FName ColumnID_Description("Description");
}


/** A tree row representing a foliage type in the palette */
class SSPresetPipelineViewItemRow : public SMultiColumnTableRow<FPresetPipelineViewItemInfoPtr>
{
public:
	SLATE_BEGIN_ARGS(SSPresetPipelineViewItemRow) {}
		
	SLATE_EVENT(FOnSelectionStateChanged, OnSelectionStateChanged)
		
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FPresetPipelineViewItemModel>& InModel);
	
	FOnSelectionStateChanged OnSelectionStateChanged;

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FPresetPipelineViewItemModel> Model;
};

// Class to represent a preset save window
class SCMPresetPipelineExportWindow : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SCMPresetPipelineExportWindow){}

    SLATE_ARGUMENT(FText, Title)
    	
    SLATE_ARGUMENT(TArray<PresetDescPair>, Presets)
	    
    SLATE_EVENT(FOnPresetExportRequested,OnPresetExportRequested)
    	
    SLATE_END_ARGS()

    // Default constructor
    SCMPresetPipelineExportWindow(){}

    // Constructs a new SCMPresetPipelineExportWindow
    void Construct(const FArguments& InArgs);

	void InitialSetup();

private:
	void CreatePipelineViewItemModels();

	TSharedRef<ITableRow> TreeViewGenerateRow(FPresetPipelineViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable);
	void SelectionChanged();
	
	FOnPresetExportRequested OnPresetExportRequested;
	
	void ExportDirectorySelected(const FString& InDirectory);

	// Handles name change events
	void OnDirChanged(const FText& NewName);   
	
    // Handles button click events
    FReply OnButtonClick(EAppReturnType::Type ButtonID);
	
    FReply OnBrowse();

	void CheckExportState();

	TArray<FName> GetSelectedPresets() const;
	
    TArray<FPresetPipelineViewItemModelPtr> PresetPipelineViewItemModels;

    TSharedPtr<STreeView<FPresetPipelineViewItemModelPtr>> TreeViewWidget;

    TSharedPtr<class SEditableTextBox> DirectoryTextBox;

	TSharedPtr<class SWarningOrErrorBox> ErrorWidget;

    TSharedPtr<class SPrimaryButton> ActionBtn;
	
	TArray<PresetDescPair> Presets;

	FString ExportPath;
};

// Class to represent a preset save window
class SCMPresetPipelineImportWindow : public SWindow
{
public:
	SLATE_BEGIN_ARGS(SCMPresetPipelineImportWindow){}

	SLATE_ARGUMENT(FText, Title)
    	
	SLATE_ARGUMENT(TArray<FPresetImportData>, Presets)

	SLATE_EVENT(FOnPresetImportRequested,OnPresetImportRequested)
		
	SLATE_END_ARGS()

	// Default constructor
	SCMPresetPipelineImportWindow(){}

	// Constructs a new SCMPresetPipelineImportWindow
	void Construct(const FArguments& InArgs);

private:
	void CreatePipelineViewItemModels();

	FOnPresetImportRequested OnPresetImportRequested;

	TSharedRef<ITableRow> TreeViewGenerateRow(FPresetPipelineViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable);

	void SelectionChanged();

	void CheckImportState();
	
	FReply OnButtonClick(EAppReturnType::Type ButtonID) const;
	
	TArray<FName> GetSelectedPresets() const;

	static bool DoesPresetAvailable(const FName&PresetName);
	
	TArray<FPresetPipelineViewItemModelPtr> PresetPipelineViewItemModels;

	TSharedPtr<STreeView<FPresetPipelineViewItemModelPtr>> TreeViewWidget;

	TSharedPtr<class SEditableTextBox> DirectoryTextBox;
	
	TSharedPtr<class SPrimaryButton> ActionBtn;
	
	TArray<FPresetImportData> Presets;

	FString ImportPath;
};