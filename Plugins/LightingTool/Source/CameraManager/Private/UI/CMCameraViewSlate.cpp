// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMCameraViewSlate.h"
#include "ActorEditorUtils.h"
#include "CineCameraActor.h"
#include "DetailLayoutBuilder.h"
#include "EditorStyleSet.h"
#include "ScopedTransaction.h"
#include "CMLogChannels.h"
#include "CMToolStyle.h"
#include "Editor/EditorEngine.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableText.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"



void SSCameraViewItemRow::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, const TSharedPtr<FCameraViewItemModel>& InModel)
{
	Model = InModel;
	PresetNames = InArgs._PresetData;
	FilterText = InArgs._InFilterText;
	OnPresetSelectedForLoadDropDown = InArgs._OnPresetSelectedForLoadDropDown;
	
	ensure(Model->CameraViewItemInfo.IsValid());

	Model->CameraViewItemInfo->OnRenameRequested.BindRaw(this,&SSCameraViewItemRow::StartRenamingProcess);
	Model->CameraViewItemInfo->OnPresetChanged.BindRaw(this,&SSCameraViewItemRow::RefreshSelectedPreset);
	
	SMultiColumnTableRow<FCameraViewItemInfoPtr>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SSCameraViewItemRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	TSharedPtr<SWidget> TableRowContent = SNullWidget::NullWidget;
	
	if(!Model->CameraViewItemInfo.Get()->CameraPtr.IsValid())
	{
		UE_LOG(LogCameraManager,Error,TEXT("CameraActor is empty, terminating..."));
		return TableRowContent.ToSharedRef();
	}
	
	if(ColumnName == TEXT("Pilot"))
	{
		TableRowContent =
		SNew(SBox)
		.Padding(FMargin(3))
		[
			SNew(SButton)
			.ButtonStyle(FCMToolStyle::Get(), "CameraManager.FloatingButtonStyle")
			.OnClicked_Lambda([&]()
			{
				Model->CameraViewItemInfo->OnPilotButtonClicked.ExecuteIfBound(Model->CameraViewItemInfo->CameraPtr.Get());
				
				return FReply::Handled();
			})
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.ContentPadding(0)
			.Content()
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image_Lambda([&]() -> const FSlateBrush*
				{
					if(Model->CameraViewItemInfo.IsValid() && Model->CameraViewItemInfo.Get()->CameraPtr.IsValid())
					{
						return Model->CameraViewItemInfo.Get()->bPilotState
						? FCMToolStyle::GetCreatedToolSlateStyleSet()->GetBrush("CameraManager.PilotIcon")
						: FCMToolStyle::GetCreatedToolSlateStyleSet()->GetBrush("CameraManager.NoPilotIcon");
					}
					return nullptr;
				})
			]
		];
	}
	else if (ColumnName == TEXT("Camera"))
	{
		TableRowContent =
		SNew(SHorizontalBox)
			
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(FVector2D(5,0))
		]
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SAssignNew(NameWidgetSwitcher,SWidgetSwitcher)
			+SWidgetSwitcher::Slot()
			[
				SNew(SBox)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.OnDoubleClicked_Lambda([this](const FGeometry&,const FPointerEvent&)
					{
						StartRenamingProcess();

						return FReply::Handled();	
					})
					.Text_Lambda([&]()
					{
						if(Model.IsValid() && Model->CameraViewItemInfo.Get()->CameraPtr.IsValid())
						{
							return FText::FromString(Model->CameraViewItemInfo.Get()->CameraPtr->GetActorLabel());
						}
						return FText::GetEmpty();
					})
					.HighlightText_Lambda([this]()
					{
						if(FilterText.IsValid())
						{
							return FilterText->IsEmpty() ? FText::GetEmpty() : FText::FromString(*FilterText);
						}
						return FText::GetEmpty();
					})
				]
			]

			+SWidgetSwitcher::Slot()
			[
				SAssignNew(CameraNameEditableText,SInlineEditableTextBlock)
				.ColorAndOpacity(FAppStyle::Get().GetSlateColor("Colors.AccentGreen"))
				.OnTextCommitted_Lambda([this](const FText& InLabel, ETextCommit::Type InCommitInfo)
				{
					auto* Actor = Model->CameraViewItemInfo->CameraPtr.Get();
					if (Actor && Actor->IsActorLabelEditable() && !InLabel.ToString().Equals(Actor->GetActorLabel(), ESearchCase::CaseSensitive))
					{
						const FScopedTransaction Transaction(FText::FromName(TEXT("Rename Actor")));
						FActorLabelUtilities::RenameExistingActor(Actor, InLabel.ToString());
					}
					NameWidgetSwitcher->SetActiveWidgetIndex(0);
				})
				.OnVerifyTextChanged_Lambda([](const FText& InLabel, FText& OutErrorMessage)
				{
					return FActorEditorUtils::ValidateActorName(InLabel, OutErrorMessage);
				})
			]
		];
	}
	else if(ColumnName == TEXT("Preset"))
	{
		TableRowContent =
		SNew(SHorizontalBox)        					
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Fill)
		.FillWidth(1)
		[
			SNew(SBox)
			.Padding(FMargin(3,6))
			[
				SAssignNew(PresetComboBox,STextComboBox)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OptionsSource(PresetNames)
				.InitiallySelectedItem(FindPresetName(*PresetNames,Model->CameraViewItemInfo.Get()->PresetName))
				.ComboBoxStyle(FAppStyle::Get(), "SimpleComboBox")
				.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& String, ESelectInfo::Type)
				 {
					Model->CameraViewItemInfo->PresetName = FName(**String.Get());
					OnPresetSelectedForLoadDropDown.ExecuteIfBound(Model->CameraViewItemInfo);
				 })
			]
		];
	}
	else if(ColumnName == TEXT("Lock"))
	{
		TableRowContent =
		SNew(SCheckBox)
		.IsChecked_Lambda([&]()
		{
			if(Model->CameraViewItemInfo.Get()->CameraPtr.IsValid())
			{				
				return Model->CameraViewItemInfo.Get()->CameraPtr->IsLockLocation() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			}
			
			UE_LOG(LogCameraManager,Error,TEXT("CameraActor is empty, returning default..."));
			return ECheckBoxState::Unchecked;
		})
		.OnCheckStateChanged_Lambda([&](const ECheckBoxState NewState)
		{
			Model->CameraViewItemInfo->OnLockStateChanged.ExecuteIfBound(Model->CameraViewItemInfo->CameraPtr.Get(),NewState == ECheckBoxState::Checked);
		});
	}
	return TableRowContent.ToSharedRef();
}

TSharedPtr<FString> SSCameraViewItemRow::FindPresetName(const TArray<TSharedPtr<FString>>& InPresetNames,const FName& InPresetName)
{
	for (const TSharedPtr<FString>& PresetName : InPresetNames)
	{
		if (PresetName.IsValid() && *PresetName == InPresetName.ToString())
		{
			return PresetName;
		}
	}
	return nullptr;
}

void SSCameraViewItemRow::StartRenamingProcess() const
{
	if (CameraNameEditableText.IsValid()							&&
		NameWidgetSwitcher.IsValid()								&&
		Model->CameraViewItemInfo.Get()->CameraPtr.IsValid()		&&
		!CameraNameEditableText->IsInEditMode()						&&
		NameWidgetSwitcher->GetNumWidgets() > 0)
	{
		NameWidgetSwitcher->SetActiveWidgetIndex(1);
		CameraNameEditableText->SetText(FText::FromString(Model->CameraViewItemInfo.Get()->CameraPtr->GetActorLabel()));
		CameraNameEditableText->EnterEditingMode();
	}
}

void SSCameraViewItemRow::RefreshSelectedPreset() const
{
	if(PresetComboBox.IsValid())
	{
		TSharedPtr<FString> FoundPresetName = FindPresetName(*PresetNames,Model->CameraViewItemInfo.Get()->PresetName);
		
		PresetComboBox->SetSelectedItem(FoundPresetName);
	}
}




