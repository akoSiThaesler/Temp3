// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTUserPathPicker.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "Components/VerticalBox.h"
#include "Widgets/Input/SButton.h"


void SLTUserPathPicker::Construct(const FArguments& InArgs)
{
	WeakParentWindow = InArgs._ParentWindow;

	bPressedOk = false;
	AssetPath = InArgs._AssetPath;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	FPathPickerConfig PathPickerConfig;
	PathPickerConfig.DefaultPath = AssetPath.GetVirtualPathString();
	PathPickerConfig.OnPathSelected = FOnPathSelected::CreateRaw(this, &SLTUserPathPicker::OnSelectAssetPath);
	PathPickerConfig.bAllowReadOnlyFolders = false;
	PathPickerConfig.bOnPathSelectedPassesVirtualPaths = true;
	PathPickerConfig.bAllowClassesFolder = false;
	PathPickerConfig.bAllowContextMenu = false;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			ContentBrowserModule.Get().CreatePathPicker(PathPickerConfig)
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.Padding(0, 20, 0, 0)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(0, 2, 6, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Bottom)
				.ContentPadding(FMargin(8, 2, 8, 2))
				.OnClicked(this, &SLTUserPathPicker::OnClickOk)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
				.Text(FText::FromName(TEXT("OK")))
			]
			+ SHorizontalBox::Slot()
			.Padding(0, 2, 0, 0)
			.AutoWidth()
			[
				SNew(SButton)
				.VAlign(VAlign_Bottom)
				.ContentPadding(FMargin(8, 2, 8, 2))
				.OnClicked(this, &SLTUserPathPicker::OnClickCancel)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.TextStyle(FAppStyle::Get(), "FlatButton.DefaultTextStyle")
				.Text(FText::FromName(TEXT("Cancel")))
			]
		]
	];
};

FReply SLTUserPathPicker::OnClickOk()
{
	bPressedOk = true;

	if (WeakParentWindow.IsValid())
	{
		WeakParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SLTUserPathPicker::OnClickCancel()
{
	if (WeakParentWindow.IsValid())
	{
		WeakParentWindow.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FString ULTUserPathPicker::CreateAFolderPicker(const FName& InTitle)
{
	FContentBrowserItemPath AssetPath;
	
	// Create the window to pick the class
	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(FText::FromName(InTitle))
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(300.f, 400.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SLTUserPathPicker> PathPickerDialog = SNew(SLTUserPathPicker)
		.ParentWindow(PickerWindow)
		.AssetPath(AssetPath);

	PickerWindow->SetContent(PathPickerDialog);

	GEditor->EditorAddModalWindow(PickerWindow);
	
	if (PathPickerDialog->bPressedOk)
	{
		AssetPath.SetPathFromString(PathPickerDialog->AssetPath.GetVirtualPathString(), EContentBrowserPathType::Virtual);

		if(AssetPath.HasInternalPath())
		{
			return PathPickerDialog->AssetPath.GetInternalPathString();
		}
		
		return FString(TEXT("NotValid"));
	}
	
	return {};
}