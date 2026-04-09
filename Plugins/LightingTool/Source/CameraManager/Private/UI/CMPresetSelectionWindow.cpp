// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "CMPresetSelectionWindow.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "EditorStyleSet.h"
#include "Editor.h"
#include "CMLogChannels.h"
#include "CMToolStyle.h"
#include "SPrimaryButton.h"
#include "Framework/Commands/GenericCommands.h"
#include "Utilities/CMCameraManagerUtils.h"
#include "Widgets/Images/SImage.h"

#define LOCTEXT_NAMESPACE "PresetSelectionWindow"

void SCMPresetSaveWindow::Construct(const FArguments& InArgs)
{
	Presets = InArgs._Presets;
	OnPresetSaveRequested = InArgs._OnPresetSaveRequested;
	OnPresetDeleteRequested = InArgs._OnPresetDeleteRequested;

	CreateViewItemModels();

	InitializeCommands();
		
	SWindow::Construct(SWindow::FArguments()
	.Title(InArgs._Title)
	.HasCloseButton(false)
	.IsTopmostWindow(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.IsPopupWindow(false)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	//.ClientSize(FVector2D(200.f, 300.f))
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
					SAssignNew(TreeViewWidget,STreeView<FPresetViewItemModelPtr>)
					.ItemHeight(120.0f)
					.TreeItemsSource(&PresetViewItemModels)
					.SelectionMode(ESelectionMode::Single)
					.ClearSelectionOnClick(true)
					.EnableAnimatedScrolling(true)
					.OnSelectionChanged(this,&SCMPresetSaveWindow::OnPresetSelectionChanged)
					.OnMouseButtonDoubleClick(this,&SCMPresetSaveWindow::OnPresetItemDoubleClicked)
					.OnGenerateRow_Lambda([](FPresetViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable)
					{
						return SNew(SSPresetViewItemRow, OwnerTable, Item);
					})
					.OnGetChildren_Lambda([](FPresetViewItemModelPtr Item, TArray<FPresetViewItemModelPtr>& OutChildren)
					{
						//No Children
					})
					.HeaderRow
					(
						// Toggle Active //TODO Add ShortMode
						SNew(SHeaderRow)

						+ SHeaderRow::Column(PresetTreeTreeColumns::ColumnID_Preset)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetTreeTreeColumns::ColumnID_Preset))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(2.0f)

						+ SHeaderRow::Column(PresetTreeTreeColumns::ColumnID_Description)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetTreeTreeColumns::ColumnID_Description))
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

					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 10, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.MinDesiredWidth(75)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Preset Name", "Preset Name"))
						]
					]

					+SHorizontalBox::Slot()
					[
						SAssignNew(SaveTextBox,SEditableTextBox)
						.OnTextChanged(this,&SCMPresetSaveWindow::OnNameChanged)
						.OnTextCommitted(this, &SCMPresetSaveWindow::OnNameCommitted)
						.MinDesiredWidth(250)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3)
				[
					SNew(SHorizontalBox)

					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(0, 0, 10, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.MinDesiredWidth(75)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("Description", "Description"))
						]
					]

					+SHorizontalBox::Slot()
					[
						SAssignNew(DescriptionTextBox,SEditableTextBox)
						.OnTextChanged(this,&SCMPresetSaveWindow::OnDescriptionChanged)
						.OnTextCommitted(this, &SCMPresetSaveWindow::OnDescriptionCommitted)
						.MinDesiredWidth(250)
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
			SNew(SUniformGridPanel)
			.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))

			+SUniformGridPanel::Slot(0,0)
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(30)
				.HeightOverride(24)
				.Visibility_Lambda([&]()
				{
					return bIsAnyPresetSelected ? EVisibility::Visible : EVisibility::Hidden;
				})
				[
					SNew(SButton)
					.ButtonStyle( FCMToolStyle::Get(), "CameraManager.ImageWithButtonStyle")
					.OnClicked_Lambda([&]()
					{
						DeleteSelectedPreset();
						
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(FGenericCommands::Get().Delete->GetIcon().GetIcon())
					]
				]
			]
			
			+SUniformGridPanel::Slot(1,0)
			.HAlign(HAlign_Right)
			[
				SAssignNew(SaveButton,SPrimaryButton)
				.Text_Lambda([&]()
				{
					return FText::FromName(SaveType);
				})
				.OnClicked(this, &SCMPresetSaveWindow::OnButtonClick, EAppReturnType::Ok)
			]
			+SUniformGridPanel::Slot(2,0)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(LOCTEXT("Cancel", "Cancel"))
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCMPresetSaveWindow::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
	
	LastValidPresetName = GenerateUniquePresetName();
	SaveTextBox->SetText(FText::FromName(LastValidPresetName));
}

FReply SCMPresetSaveWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if(SaveButton.IsValid() && SaveButton->IsEnabled())
		{
			OnPresetSaveRequested.Execute(FCMPresetUtils::CleanPresetName(LastValidPresetName),LastValidDescription);
		}
	}
	return ActionCommandList->ProcessCommandBindings( InKeyEvent ) ? FReply::Handled() : FReply::Unhandled();
}


void SCMPresetSaveWindow::OnNameChanged(const FText& NewName)
{
	if(FCMPresetUtils::IsValidNameConvention(NewName))
	{
		LastValidPresetName = *NewName.ToString();
		
		SaveButton->SetEnabled(!NewName.IsEmpty());
	}
	else
	{
		if(FCMPresetUtils::IsValidNameConventionFromName(LastValidPresetName))
		{
			SaveButton->SetEnabled(!LastValidPresetName.IsNone());
		}
		else
		{
			SaveButton->SetEnabled(false);
		}
		
		if(!NewName.IsEmpty() && !LastValidPresetName.IsNone())
		{
			SaveTextBox->SetText(FText::FromName(LastValidPresetName));
		}
		else
		{
			SaveTextBox->SetText(FText::GetEmpty());
		}
	}

	//Check if the name is already in the list and not selected
	if(!NewName.IsEmpty() && DoesFormerPresetsContainsTheNameAlsoNotSelected(Presets,NewName))
	{
		SaveButton->SetEnabled(false);
	}
}

bool SCMPresetSaveWindow::DoesFormerPresetsContainsTheNameAlsoNotSelected(const TArray<PresetDescPair>& InPresets, const FText& Name) const
{
	const FName LocalGivenName = *Name.ToString();
	TArray<TSharedPtr<FPresetViewItemModel>> SelectedItems = TreeViewWidget->GetSelectedItems();

	for(const PresetDescPair& CurrentPreset : InPresets)
	{
		if(CurrentPreset.Key.IsEqual(LocalGivenName))
		{
			if(SelectedItems.IsEmpty())
			{
				return true;
			}

			return !SelectedItems[0]->PresetViewItemInfo->PresetName.IsEqual(LocalGivenName);
		}
	}
	return false;
}


void SCMPresetSaveWindow::OnNameCommitted(const FText& NewName, ETextCommit::Type CommitInfo)
{
	LastValidPresetName = *NewName.ToString();
}


void SCMPresetSaveWindow::OnDescriptionChanged(const FText& NewDescription)
{
	LastValidDescription = *NewDescription.ToString();
}

void SCMPresetSaveWindow::OnDescriptionCommitted(const FText& NewDescription, ETextCommit::Type CommitInfo)
{
	//LastValidDescription = *NewDescription.ToString();	
}

void SCMPresetSaveWindow::OnPresetSelectionChanged(TSharedPtr<FPresetViewItemModel> SelectedItem, ESelectInfo::Type SelectInfo)
{
	bIsAnyPresetSelected = SelectedItem.IsValid();

	if(bIsAnyPresetSelected)
	{
		SaveType = TEXT("Overwrite");
		
		LastValidPresetName = SelectedItem->PresetViewItemInfo->PresetName;
		LastValidDescription = SelectedItem->PresetViewItemInfo->Description;
	}
	else
	{
		SaveType = TEXT("Save");
		
		LastValidPresetName = GenerateUniquePresetName();
		LastValidDescription = TEXT("");
	}
	
	SaveTextBox->SetText(FText::FromName(LastValidPresetName));
	DescriptionTextBox->SetText(FText::FromName(LastValidDescription));
	DescriptionTextBox->SetToolTipText(FText::FromName(LastValidDescription));

	SaveButton->SetEnabled(true); 
}

FReply SCMPresetSaveWindow::OnButtonClick(EAppReturnType::Type ButtonID)
{
	if(ButtonID == EAppReturnType::Cancel)
	{
		LastValidPresetName = NAME_None;
	}
	OnPresetSaveRequested.Execute(FCMPresetUtils::CleanPresetName(LastValidPresetName),LastValidDescription);

	return FReply::Handled();
}

void SCMPresetSaveWindow::OnPresetItemDoubleClicked(FPresetViewItemModelPtr InItem)
{
	OnPresetSaveRequested.Execute(InItem->PresetViewItemInfo->PresetName,LastValidDescription);
}


void SCMPresetSaveWindow::InitializeCommands()
{
	ActionCommandList = MakeShareable(new FUICommandList());
	ActionCommandList->MapAction(FGenericCommands::Get().Delete,FExecuteAction::CreateRaw(this,&SCMPresetSaveWindow::DeleteSelectedPreset));
}

void SCMPresetSaveWindow::DeleteSelectedPreset()
{
	if(TreeViewWidget.IsValid() && TreeViewWidget->GetNumItemsSelected() == 1)
	{
		FPresetViewItemModelPtr FoundModel =  TreeViewWidget->GetSelectedItems()[0];
	
		PresetViewItemModels.Remove(FoundModel);
		TreeViewWidget->RequestListRefresh();
	
		OnPresetDeleteRequested.ExecuteIfBound(FoundModel->PresetViewItemInfo->PresetName);
	}
}

FName SCMPresetSaveWindow::GenerateUniquePresetName() const
{
	int32 PresetSuffix = 1;
	FString GeneratedName;
	bool bNameExists;

	do
	{
		bNameExists = false;
		GeneratedName = FString::Printf(TEXT("New Preset %d"), PresetSuffix++);
        
		// Check if the GeneratedName already exists in the Presets array
		for (const PresetDescPair& Preset : Presets)
		{
			if (Preset.Key.ToString() == GeneratedName)
			{
				bNameExists = true;
				break;
			}
		}
	} 
	while (bNameExists);
    
	return FName(*GeneratedName);
}


void SCMPresetSaveWindow::CreateViewItemModels()
{
	if(Presets.IsEmpty()){return;}
	
	for(const PresetDescPair& CurrentPreset : Presets)
	{
		FPresetViewItemModelPtr NewItemPtr = MakeShareable(new FPresetViewItemModel(MakeShareable(new FPresetViewInfo(CurrentPreset.Key,CurrentPreset.Value))));
			
		PresetViewItemModels.Emplace(NewItemPtr);
	}
}

/**************************************************************************************/

void SCMPresetLoadWindow::Construct(const FArguments& InArgs)
{
	Presets = InArgs._Presets;
	OnPresetSelectedForLoad = InArgs._OnPresetSelectedForLoad;
	OnPresetDeleteRequested = InArgs._OnPresetDeleteRequested;

	CreateViewItemModels();

	InitializeCommands();
	
	const FText BlankText = FText::GetEmpty();
	
	SWindow::Construct(SWindow::FArguments()
	.Title(InArgs._Title)
	.HasCloseButton(false)
	.IsTopmostWindow(true)
	.SupportsMaximize(false)
	.SupportsMinimize(false)
	.IsPopupWindow(false)
	.AutoCenter(EAutoCenter::PreferredWorkArea)
	//.ClientSize(FVector2D(350.f, 200.f))
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
					SAssignNew(TreeViewWidget, STreeView<FPresetViewItemModelPtr>)
					.ItemHeight(120.0f)
					.TreeItemsSource(&PresetViewItemModels)
					.SelectionMode(ESelectionMode::Single)
					.ClearSelectionOnClick(true)
					.EnableAnimatedScrolling(true)
					.OnSelectionChanged(this,&SCMPresetLoadWindow::OnPresetSelectionChanged)
					.OnMouseButtonDoubleClick(this,&SCMPresetLoadWindow::OnPresetItemDoubleClicked)
					.OnGenerateRow_Lambda([](FPresetViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable)
					{
						return SNew(SSPresetViewItemRow, OwnerTable, Item);
					})
					.OnGetChildren_Lambda([](FPresetViewItemModelPtr Item, TArray<FPresetViewItemModelPtr>& OutChildren)
					{
						//No Children
					})
					.HeaderRow
					(
						// Toggle Active //TODO Add ShortMode
						SNew(SHeaderRow)

						+ SHeaderRow::Column(PresetTreeTreeColumns::ColumnID_Preset)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetTreeTreeColumns::ColumnID_Preset))
						.HAlignHeader(HAlign_Center)
						.HAlignCell(HAlign_Left)
						.VAlignCell(VAlign_Center)
						.FillWidth(2.0f)

						+ SHeaderRow::Column(PresetTreeTreeColumns::ColumnID_Description)
						.HeaderContentPadding(FMargin(2,2, 2, 2))
						.DefaultLabel(FText::FromName(PresetTreeTreeColumns::ColumnID_Description))
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
			SNew(SUniformGridPanel)
			.MinDesiredSlotWidth(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FAppStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			.SlotPadding(FAppStyle::GetMargin("StandardDialog.SlotPadding"))

			+SUniformGridPanel::Slot(0,0)
			.HAlign(HAlign_Left)
			[
				SNew(SBox)
				.WidthOverride(30)
				.HeightOverride(24)
				.Visibility_Lambda([&]()
				{
					return bIsAnyPresetSelected ? EVisibility::Visible : EVisibility::Hidden;
				})
				[
					SNew(SButton)
					.ButtonStyle( FCMToolStyle::Get(), "CameraManager.ImageWithButtonStyle")
					.OnClicked_Lambda([&]()
					{
						DeleteSelectedPreset();
									
						return FReply::Handled();
					})
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(FGenericCommands::Get().Delete->GetIcon().GetIcon())
					]
				]
			]
			+SUniformGridPanel::Slot(1,0)
			.HAlign(HAlign_Right)
			[
				SNew(SPrimaryButton)
				.Text(LOCTEXT("Load", "Load"))
				.OnClicked(this, &SCMPresetLoadWindow::OnButtonClick, EAppReturnType::Ok)
			]
			+SUniformGridPanel::Slot(2,0)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(LOCTEXT("Cancel", "Cancel"))
				.ContentPadding(FAppStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SCMPresetLoadWindow::OnButtonClick, EAppReturnType::Cancel)
			]
		]
	]);
}

FReply SCMPresetLoadWindow::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		if(TreeViewWidget->GetNumItemsSelected() == 1)
		{
			SelectedPreset = TreeViewWidget->GetSelectedItems()[0]->PresetViewItemInfo->PresetName;
	
			OnPresetSelectedForLoad.Execute(SelectedPreset);

			return FReply::Handled();
		}
	}
	
	return ActionCommandList->ProcessCommandBindings( InKeyEvent ) ? FReply::Handled() : FReply::Unhandled();
}


void SCMPresetLoadWindow::InitializeCommands()
{
	ActionCommandList = MakeShareable(new FUICommandList());
	ActionCommandList->MapAction(FGenericCommands::Get().Delete,FExecuteAction::CreateRaw(this,&SCMPresetLoadWindow::DeleteSelectedPreset));
}

void SCMPresetLoadWindow::DeleteSelectedPreset()
{
	if(TreeViewWidget.IsValid() && TreeViewWidget->GetNumItemsSelected() == 1)
	{
		FPresetViewItemModelPtr FoundModel =  TreeViewWidget->GetSelectedItems()[0];
	
		PresetViewItemModels.Remove(FoundModel);
		TreeViewWidget->RequestListRefresh();
		
		OnPresetDeleteRequested.ExecuteIfBound(FoundModel->PresetViewItemInfo->PresetName);
	}
}

FReply SCMPresetLoadWindow::OnButtonClick(EAppReturnType::Type ButtonID)
{
	if(ButtonID == EAppReturnType::Cancel)
	{
		SelectedPreset = NAME_None;
	}
	
	OnPresetSelectedForLoad.Execute(SelectedPreset);
	
	return FReply::Handled();
}

void SCMPresetLoadWindow::CreateViewItemModels()
{
	if(Presets.IsEmpty()){return;}
	
	for(const PresetDescPair& CurrentPreset : Presets)
	{
		FPresetViewItemModelPtr NewItemPtr = MakeShareable(new FPresetViewItemModel(MakeShareable(new FPresetViewInfo(CurrentPreset.Key,CurrentPreset.Value))));
			
		PresetViewItemModels.Emplace(NewItemPtr);
	}
}

void SCMPresetLoadWindow::OnPresetSelectionChanged(TSharedPtr<FPresetViewItemModel> SelectedItem, ESelectInfo::Type SelectInfo)
{
	bIsAnyPresetSelected = SelectedItem.IsValid();
	
	if(bIsAnyPresetSelected)
	{
		SelectedPreset = SelectedItem->PresetViewItemInfo->PresetName;
	}
}

void SCMPresetLoadWindow::OnPresetItemDoubleClicked(FPresetViewItemModelPtr InItem)
{
	if(InItem.IsValid())
	{
		SelectedPreset = InItem->PresetViewItemInfo->PresetName;
	}
	
	OnPresetSelectedForLoad.Execute(SelectedPreset);
}


// Preset Tree View
void SSPresetViewItemRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView,const TSharedPtr<FPresetViewItemModel>& InModel)
{
	Model = InModel;
	SMultiColumnTableRow<FPresetViewItemInfoPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SSPresetViewItemRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedPtr<SWidget> TableRowContent = SNullWidget::NullWidget;
	
	if(!Model->PresetViewItemInfo.IsValid())
	{
		UE_LOG(LogCameraManager,Error,TEXT("Preset Info is empty, terminating..."));
		return TableRowContent.ToSharedRef();
	}

	FText Desc = FText::FromName(Model->PresetViewItemInfo->Description);
	
	if(Model->PresetViewItemInfo->Description.IsNone())
	{
		Desc = FText::GetEmpty();
	}
	
	if(ColumnName == TEXT("Preset"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
        			
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SExpanderArrow, SharedThis(this))
		]
        					
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HeightOverride(20)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Model->PresetViewItemInfo.Get()->PresetName))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			]
		];
	}
	else if(ColumnName == TEXT("Description"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
        			
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SExpanderArrow, SharedThis(this))
		]
        					
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			.HeightOverride(20)
			[
				SNew(STextBlock)
				.Text(Desc)
				.ToolTipText(Desc)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
			]
		];
	}

	return TableRowContent.ToSharedRef();
}


#undef LOCTEXT_NAMESPACE
