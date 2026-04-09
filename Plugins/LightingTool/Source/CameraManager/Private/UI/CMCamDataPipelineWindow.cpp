// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMCamDataPipelineWindow.h"
#include <algorithm>
#include "Editor.h"
#include "EditorStyleSet.h"
#include "CMLogChannels.h"
#include "CMPresetSelectionWindow.h"
#include "CMSubsystem.h"
#include "CMToolStyle.h"
#include "SPrimaryButton.h"
#include "SWarningOrErrorBox.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Utilities/CMCameraManagerUtils.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#pragma region Camera Data Pipeline Preset Row

void SSPresetPipelineViewItemRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FPresetPipelineViewItemModel>& InModel)
{
	Model = InModel;
	OnSelectionStateChanged = InArgs._OnSelectionStateChanged;
	
	ensure(Model->PresetPipelineViewItemInfoPtr.IsValid());
	SMultiColumnTableRow<FPresetPipelineViewItemInfoPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SSPresetPipelineViewItemRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedPtr<SWidget> TableRowContent = SNullWidget::NullWidget;
	
	if(ColumnName == TEXT("Select"))
	{
		TableRowContent =
		SNew(SBox)
		.HeightOverride(25)
		.VAlign(VAlign_Center)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([&]()
			{
				if(Model->PresetPipelineViewItemInfoPtr.IsValid())
				{				
					return Model->PresetPipelineViewItemInfoPtr.Get()->bIsSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				}
				return ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([&](const ECheckBoxState NewState)
			{
				if(Model->PresetPipelineViewItemInfoPtr.IsValid())
				{
					Model->PresetPipelineViewItemInfoPtr->bIsSelected = NewState == ECheckBoxState::Checked;
					OnSelectionStateChanged.ExecuteIfBound();
				}
			})
		];

	}
	else if(ColumnName == TEXT("Preset"))
	{
		TableRowContent =

		SNew(SOverlay)
		+SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Left)
			.Padding(1.0f)
		[
			SNew(SImage)
			//.ColorAndOpacity(FSlateColor::UseForeground())
			.Visibility(Model->PresetPipelineViewItemInfoPtr->bIsAlreadyAvailable ? EVisibility::Visible : EVisibility::Hidden)
			.Image(FCMToolStyle::GetCreatedToolSlateStyleSet()->GetBrush("CameraManager.ReplaceIcon"))
			.DesiredSizeOverride(FVector2D(8,8))
			.ToolTipText(FText::FromName("This preset will overwrite the existing preset with the same name, and the cameras using this preset will be updated."))
		]
			
		+SOverlay::Slot()
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(FMargin(11,0,0,0))
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				.HeightOverride(20)
				[
					SNew(STextBlock)
					.Text(FText::FromName(Model->PresetPipelineViewItemInfoPtr->PresetName))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]
		];
	}
	else if(ColumnName == TEXT("Description"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
        			
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(11,0,0,0))
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HeightOverride(20)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Model->PresetPipelineViewItemInfoPtr->Description))
				.ToolTipText(FText::FromName(Model->PresetPipelineViewItemInfoPtr->Description))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
			]
		];
	}

	return TableRowContent.ToSharedRef();
	
}

#pragma endregion Camera Data Pipeline Preset Row

#pragma region Camera Data Pipeline Export Window

void SCMPresetPipelineExportWindow::Construct(const FArguments& InArgs)
{
	Presets = InArgs._Presets;
	OnPresetExportRequested = InArgs._OnPresetExportRequested;
	ExportPath = FCMFileUtils::GetDefaultExportPath();

	CreatePipelineViewItemModels();
	
	SWindow::Construct(SWindow::FArguments()
	.Title(InArgs._Title)
	.HasCloseButton(false)
	.IsTopmostWindow(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.IsPopupWindow(false)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot() // Add user input block
		.Padding(2,2,2,4)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(3)
				[
					SAssignNew(TreeViewWidget,STreeView<FPresetPipelineViewItemModelPtr>)
					.TreeItemsSource(&PresetPipelineViewItemModels)
					.SelectionMode(ESelectionMode::Single)
					.ClearSelectionOnClick(true)
					.EnableAnimatedScrolling(true)
					.OnGenerateRow(this,&SCMPresetPipelineExportWindow::TreeViewGenerateRow)
					.OnGetChildren_Lambda([](FPresetPipelineViewItemModelPtr Item, TArray<FPresetPipelineViewItemModelPtr>& OutChildren)
					{
						//No Children
					})
					.HeaderRow
					(
						// Toggle Active 
						SNew(SHeaderRow)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Select)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Select))
						.HeaderContentPadding(FMargin(2, 2, 2, 2))
						.FixedWidth(50)
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Center)
						.VAlignCell(VAlign_Center)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Preset)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Preset))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(1.0f)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Description)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Description))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(3.0f)
					)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					.AutoWidth()
					.Padding(FMargin(3.0f, 3.0f, 3.0f, 0))
					[
						SNew(STextBlock)
						.Text(FText::FromName(TEXT("Target:")))
						.ToolTipText(FText::FromName(TEXT("Target Directory")))
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.FillWidth(1.0f)
					[
						SAssignNew(DirectoryTextBox,SEditableTextBox)
						.Text(FText::FromString(ExportPath))
						.ToolTipText(FText::FromString(ExportPath))
						.OnTextChanged(this,&SCMPresetPipelineExportWindow::OnDirChanged)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Top)
					.Padding(5.0f, 2.0f, 3.0f, 0.0f)
					.AutoWidth()
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.OnClicked(this,&SCMPresetPipelineExportWindow::OnBrowse)
						.ToolTipText(FText::FromName(TEXT("Browse for a folder")))
						.ContentPadding(0)
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush("Icons.FolderClosed"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(3.f, 16.f)
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(FMargin(10.0f,0.0f))
			[
				SAssignNew(ActionBtn,SPrimaryButton)
				.Text_Lambda([&]()
				{
					return FText::FromName(TEXT("Export All"));
				})
				.OnClicked(this, &SCMPresetPipelineExportWindow::OnButtonClick, EAppReturnType::Ok)
			]

			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.FillWidth(1.0f)
			.Padding(FMargin(5.0f,0.0f))
			[
				SAssignNew(ErrorWidget,SWarningOrErrorBox)
				.Visibility(EVisibility::Hidden)
				//.IconSize(FVector2D(16,16))
				.Padding(FMargin(1.0f))
				.Message_Lambda([&]()
				{
					return FText::FromName(TEXT("Invalid Path!"));
				})
				.MessageStyle(EMessageStyle::Error)
				
			]
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(FMargin(10.0f,0.0f))
			[
				SNew(SButton)
				.Text(FText::FromName(TEXT("Cancel")))
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCMPresetPipelineExportWindow::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
}

void SCMPresetPipelineExportWindow::InitialSetup()
{
	DirectoryTextBox->ScrollTo(ETextLocation::EndOfLine);
}


void SCMPresetPipelineExportWindow::ExportDirectorySelected(const FString& InDirectory)
{
	UE_LOG(LogCameraManager,Warning,TEXT("Directory Selected: %s"),*InDirectory);	
}

void SCMPresetPipelineExportWindow::CreatePipelineViewItemModels()
{
	if(Presets.IsEmpty()){return;}
	
	for(const PresetDescPair& CurrentPreset : Presets)
	{
		FPresetPipelineViewItemModelPtr NewItemPtr = MakeShareable(new FPresetPipelineViewItemModel(MakeShareable(new FPresetPipelineViewItemInfo(CurrentPreset.Key,CurrentPreset.Value))));
			
		PresetPipelineViewItemModels.Emplace(NewItemPtr);
	}
	
}

TSharedRef<ITableRow> SCMPresetPipelineExportWindow::TreeViewGenerateRow(FPresetPipelineViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SSPresetPipelineViewItemRow, OwnerTable, Item)
		   .OnSelectionStateChanged(this,&SCMPresetPipelineExportWindow::SelectionChanged);
}

void SCMPresetPipelineExportWindow::SelectionChanged()
{
	CheckExportState();
}

void SCMPresetPipelineExportWindow::OnDirChanged(const FText& NewName)
{
	ExportPath = NewName.ToString();
	CheckExportState();
}

/*
void SCMPresetPipelineExportWindow::OnDirCommitted(const FText& NewName, ETextCommit::Type CommitInfo)
{
}
*/

FReply SCMPresetPipelineExportWindow::OnButtonClick(EAppReturnType::Type ButtonID)
{
	if(ButtonID == EAppReturnType::Cancel)
	{
		OnPresetExportRequested.ExecuteIfBound({},TEXT(""));
	}
	else
	{
		OnPresetExportRequested.ExecuteIfBound(GetSelectedPresets(),ExportPath);
	}
	
	return FReply::Handled();
}

FReply SCMPresetPipelineExportWindow::OnBrowse()
{
	FCMFileUtils::OpenExportPresetDialog(ExportPath, ExportPath);

	DirectoryTextBox->SetText(FText::FromString(ExportPath));
	DirectoryTextBox->SetToolTipText(FText::FromString(ExportPath));
	DirectoryTextBox->ScrollTo(ETextLocation::EndOfLine);

	CheckExportState();
	
	return FReply::Handled();
}

void SCMPresetPipelineExportWindow::CheckExportState()
{
	// Check if there is any selected item in the model list
	bool bIsAnySelectedItemAvailable = std::any_of(PresetPipelineViewItemModels.begin(), PresetPipelineViewItemModels.end(),
		[](const auto& CurrentModel) { 
			return CurrentModel->PresetPipelineViewItemInfoPtr->bIsSelected;
		});
	
	// Check if the export path is valid
	if (FCMFileUtils::IsValidPathForSaving(ExportPath))
	{
		ActionBtn->SetEnabled(bIsAnySelectedItemAvailable);
		ErrorWidget->SetVisibility(EVisibility::Hidden);
	}
	else
	{
		ErrorWidget->SetVisibility(EVisibility::Visible);
		ActionBtn->SetEnabled(false);
	}
}

TArray<FName> SCMPresetPipelineExportWindow::GetSelectedPresets() const
{
	TArray<FName> LocalSelectedPresets;
	
	for(auto& CurrentModel : PresetPipelineViewItemModels)
	{
		if(CurrentModel->PresetPipelineViewItemInfoPtr->bIsSelected)
		{
			LocalSelectedPresets.Emplace(CurrentModel->PresetPipelineViewItemInfoPtr->PresetName);
		}
	}
	return LocalSelectedPresets;
}

#pragma endregion Camera Data Pipeline Export Window

#pragma region Camera Data Pipeline Import Window

void SCMPresetPipelineImportWindow::Construct(const FArguments& InArgs)
{
	Presets = InArgs._Presets;
	ImportPath = FCMFileUtils::GetDefaultExportPath();
	OnPresetImportRequested = InArgs._OnPresetImportRequested;

	CreatePipelineViewItemModels();
	
	SWindow::Construct(SWindow::FArguments()
	.Title(InArgs._Title)
	.HasCloseButton(false)
	.IsTopmostWindow(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.IsPopupWindow(false)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot() // Add user input block
		.Padding(2,2,2,4)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(3)
				[
					SAssignNew(TreeViewWidget,STreeView<FPresetPipelineViewItemModelPtr>)
					.TreeItemsSource(&PresetPipelineViewItemModels)
					.SelectionMode(ESelectionMode::Single)
					.ClearSelectionOnClick(true)
					.EnableAnimatedScrolling(true)
					.OnGenerateRow(this,&SCMPresetPipelineImportWindow::TreeViewGenerateRow)
					.OnGetChildren_Lambda([](FPresetPipelineViewItemModelPtr Item, TArray<FPresetPipelineViewItemModelPtr>& OutChildren)
					{
						//No Children
					})
					.HeaderRow
					(
						// Toggle Active 
						SNew(SHeaderRow)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Select)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Select))
						.HeaderContentPadding(FMargin(2, 2, 2, 2))
						.FixedWidth(50)
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Center)
						.VAlignCell(VAlign_Center)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Preset)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Preset))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(1.0f)

						+ SHeaderRow::Column(PresetPipelineTreeColumns::ColumnID_Description)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetPipelineTreeColumns::ColumnID_Description))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(3.0f)
					)
				]
			]
		]

		+SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		.Padding(3.f, 16.f)
		[
			SNew(SHorizontalBox)
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			//.AutoWidth()
			.Padding(FMargin(10.0f,0.0f))
			[
				SAssignNew(ActionBtn,SPrimaryButton)
				.Text_Lambda([&]()
				{
					return FText::FromName(TEXT("Import All"));
				})
				.OnClicked(this, &SCMPresetPipelineImportWindow::OnButtonClick, EAppReturnType::Ok)
			]
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			//.AutoWidth()
			.Padding(FMargin(10.0f,0.0f))
			[
				SNew(SButton)
				.Text(FText::FromName(TEXT("Cancel")))
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCMPresetPipelineImportWindow::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
}

void SCMPresetPipelineImportWindow::CreatePipelineViewItemModels()
{
	if(Presets.IsEmpty()){return;}
	
	for(const FPresetImportData& CurrentPreset : Presets)
	{
		FPresetPipelineViewItemModelPtr NewItemPtr = MakeShareable(new FPresetPipelineViewItemModel(MakeShareable(new FPresetPipelineViewItemInfo(CurrentPreset.NameAndDesc.Key,CurrentPreset.NameAndDesc.Value,CurrentPreset.bIsAlreadyAvailable))));
			
		PresetPipelineViewItemModels.Emplace(NewItemPtr);
	}
}

TSharedRef<ITableRow> SCMPresetPipelineImportWindow::TreeViewGenerateRow(FPresetPipelineViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SSPresetPipelineViewItemRow, OwnerTable, Item)
	    .OnSelectionStateChanged(this,&SCMPresetPipelineImportWindow::SelectionChanged);
}

TArray<FName> SCMPresetPipelineImportWindow::GetSelectedPresets() const
{
	TArray<FName> LocalSelectedPresets;
	
	for(auto& CurrentModel : PresetPipelineViewItemModels)
	{
		if(CurrentModel->PresetPipelineViewItemInfoPtr->bIsSelected)
		{
			LocalSelectedPresets.Emplace(CurrentModel->PresetPipelineViewItemInfoPtr->PresetName);
		}
	}
	return LocalSelectedPresets;
}

bool SCMPresetPipelineImportWindow::DoesPresetAvailable(const FName& PresetName)
{
	if(!GEditor){return false;}
	
	if(UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
	{
		return CameraManager->IsPresetAvailable(PresetName);
	}
	return false;
}

void SCMPresetPipelineImportWindow::SelectionChanged()
{
	CheckImportState();
}

void SCMPresetPipelineImportWindow::CheckImportState()
{
	// Check if there is any selected item in the model list
	bool bIsAnySelectedItemAvailable = std::any_of(PresetPipelineViewItemModels.begin(), PresetPipelineViewItemModels.end(),
		[](const auto& CurrentModel) { 
			return CurrentModel->PresetPipelineViewItemInfoPtr->bIsSelected;
		});
	
	ActionBtn->SetEnabled(bIsAnySelectedItemAvailable);	
}

FReply SCMPresetPipelineImportWindow::OnButtonClick(EAppReturnType::Type ButtonID) const
{
	if(ButtonID == EAppReturnType::Cancel)
	{
		OnPresetImportRequested.ExecuteIfBound({});
	}
	else
	{
		OnPresetImportRequested.ExecuteIfBound(GetSelectedPresets());
	}
	
	return FReply::Handled();
}


#pragma endregion Camera Data Pipeline Import Window
