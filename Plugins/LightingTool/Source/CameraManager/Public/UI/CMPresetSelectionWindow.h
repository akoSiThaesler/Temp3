// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "CMMisc.h"
#include "Widgets/SWindow.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

class SEditableTextBox;
class SPrimaryButton;
class FPresetViewItemModel;


// Delegate declaration for preset selection event
DECLARE_DELEGATE_OneParam(FOnPresetSelectedForLoad, FName);

// Delegate declaration for preset save request event
DECLARE_DELEGATE_TwoParams(FOnPresetSaveRequested, FName LastValidPresetName, FName Description); // Preset Save Window
DECLARE_DELEGATE_OneParam(FOnPresetDeleteRequested, FName PresetName); // Preset Save Window

#define LOCTEXT_NAMESPACE "PresetSelectionWindow"

// Struct to hold information about a preset view
struct FPresetViewInfo
{
	FName PresetName;
	FName Description;

	// Default constructor
	// Initializes PresetName to NAME_None
	FPresetViewInfo()
	:PresetName(NAME_None),Description(NAME_None)
	{
	}

	// Full parameter constructor
	FPresetViewInfo(FName InPresetName, FName InDescription)
	:PresetName(InPresetName), Description(InDescription)
	{
	}
};

/** Type definition for shared pointers to instances of FEventGraphSample. */
typedef TSharedPtr<FPresetViewInfo> FPresetViewItemInfoPtr;

// Class to model a preset view item
class FPresetViewItemModel : public TSharedFromThis<FPresetViewItemModel>
{
 
public:
 FPresetViewItemInfoPtr PresetViewItemInfo;

	// Parameterized constructor
	// Initializes PresetViewItemInfo to the provided value
	FPresetViewItemModel(const FPresetViewItemInfoPtr& InPresetViewItemInfo)
	: PresetViewItemInfo(InPresetViewItemInfo)
 {
 }
};

typedef TSharedPtr<FPresetViewItemModel> FPresetViewItemModelPtr;

namespace PresetTreeTreeColumns
{
	/** IDs for list columns */
	static const FName ColumnID_Preset("Preset");
	static const FName ColumnID_Description("Description");
}

/** A tree row representing a foliage type in the palette */
class SSPresetViewItemRow : public SMultiColumnTableRow<FPresetViewItemInfoPtr>
{
public:
	SLATE_BEGIN_ARGS(SSPresetViewItemRow) {}
	SLATE_END_ARGS()

	// Constructs a new SSPresetViewItemRow
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FPresetViewItemModel>& InModel);

	// Generates a widget for a given column
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FPresetViewItemModel> Model;
};

/**************************************** Preset Windows *******************************************/

#pragma region PresetSaveWindow

// Class to represent a preset save window
class SCMPresetSaveWindow : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SCMPresetSaveWindow){}

    SLATE_ARGUMENT(FText, Title)

    SLATE_ARGUMENT(TArray<PresetDescPair>, Presets)

    SLATE_EVENT(FOnPresetSaveRequested, OnPresetSaveRequested)
     
    SLATE_EVENT(FOnPresetDeleteRequested, OnPresetDeleteRequested)

    SLATE_END_ARGS()

    // Default constructor
    SCMPresetSaveWindow()
    {
    }

    // Constructs a new SCMPresetSaveWindow
    void Construct(const FArguments& InArgs);


private:
    
    virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;

    FOnPresetSaveRequested OnPresetSaveRequested;
 
    FOnPresetDeleteRequested OnPresetDeleteRequested;

    bool bIsAnyPresetSelected = false;

    // Handles name change events
	void OnNameChanged(const FText& NewName);   
    void OnNameCommitted(const FText& NewName, ETextCommit::Type CommitInfo);
	void OnDescriptionChanged(const FText& NewDescription);
	void OnDescriptionCommitted(const FText& NewDescription, ETextCommit::Type CommitInfo);

    bool DoesFormerPresetsContainsTheNameAlsoNotSelected(const TArray<PresetDescPair>& InPresets, const FText& Name) const;

    // Handles button click events
    FReply OnButtonClick(EAppReturnType::Type ButtonID);

    TArray<FPresetViewItemModelPtr> PresetViewItemModels;

    TSharedPtr<STreeView<FPresetViewItemModelPtr>> TreeViewWidget;

    TSharedPtr<SEditableTextBox> SaveTextBox;
    TSharedPtr<SEditableTextBox> DescriptionTextBox;

    TSharedPtr<SPrimaryButton> SaveButton;

    TSharedPtr<SButton> DeleteButton;

    TArray<PresetDescPair> Presets;

    FName LastValidPresetName;
    FName LastValidDescription;

    FName SaveType = TEXT("Save");

    TSharedPtr<class FUICommandList> ActionCommandList;

    void InitializeCommands();

    void DeleteSelectedPreset();

    FName GenerateUniquePresetName() const;

    // Creates view item models
    void CreateViewItemModels();

    // Handles preset selection change events
    void OnPresetSelectionChanged(TSharedPtr<FPresetViewItemModel> SelectedItem, ESelectInfo::Type SelectInfo);

    // Handles preset item double click events
    void OnPresetItemDoubleClicked(FPresetViewItemModelPtr InItem);
};

#pragma endregion PresetSaveWindow

#pragma region PresetLoadWindow

// Class to represent a preset load window
class SCMPresetLoadWindow : public SWindow
{
public:
    SLATE_BEGIN_ARGS(SCMPresetLoadWindow){}

    SLATE_ARGUMENT(FText, Title)

    SLATE_ARGUMENT(TArray<PresetDescPair>, Presets)

    SLATE_EVENT(FOnPresetSelectedForLoad,OnPresetSelectedForLoad)
     
    SLATE_EVENT(FOnPresetDeleteRequested,OnPresetDeleteRequested)

    SLATE_END_ARGS()

 // Default constructor
 // Initializes UserResponse to EAppReturnType::Cancel
SCMPresetLoadWindow()
:	UserResponse(EAppReturnType::Cancel)
{
}

 // Constructs a new SCMPresetLoadWindow
    void Construct(const FArguments& InArgs);

private:
    virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;

    bool bIsAnyPresetSelected = false;

    FOnPresetSelectedForLoad OnPresetSelectedForLoad;
 
    FOnPresetDeleteRequested OnPresetDeleteRequested;

    TSharedPtr<class FUICommandList> ActionCommandList;

    TSharedPtr<STreeView<FPresetViewItemModelPtr>> TreeViewWidget;

    void InitializeCommands();

    void DeleteSelectedPreset();

protected:
	// Handles button click events
	FReply OnButtonClick(EAppReturnType::Type ButtonID);

	EAppReturnType::Type UserResponse;

	TArray<PresetDescPair> Presets;

	FName SelectedPreset;

	TArray<FPresetViewItemModelPtr> PresetViewItemModels;

private:
    // Creates view item models
    void CreateViewItemModels();

    // Handles preset selection change events
    void OnPresetSelectionChanged(TSharedPtr<FPresetViewItemModel> SelectedItem, ESelectInfo::Type SelectInfo);

    // Handles preset item double click events
    void OnPresetItemDoubleClicked(FPresetViewItemModelPtr InItem);
};

#pragma endregion PresetLoadWindow

#undef LOCTEXT_NAMESPACE