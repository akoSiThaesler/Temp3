// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTListManager.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "LTFunctions.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "LTLightItemController"

void ULTListManager::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

TSharedRef<SWidget> ULTListManager::RebuildWidget()
{
	return
	SNew(SBox)
	.MinDesiredWidth(150)
	[
		SAssignNew(ItemComboBox, STextComboBox)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OptionsSource(&ItemNames)
		 .InitiallySelectedItem(nullptr) 
		.OnSelectionChanged_Lambda([this](const TSharedPtr<FString>& String, ESelectInfo::Type)
		 {
			SelectedItemIndex = ItemNames.Find(String);
	   		if(SelectedItemIndex >= 0)
	   		{
	   			OnItemSelected.Broadcast(**String);
	   		}
		 })
    ];
	
}

void ULTListManager::ChangeSelectedItemName(const FName& InItemName)
{
	if(ItemNames.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("ItemNames is empty."));
		return;
	}
	
	SelectedItemIndex = 0;
	
	if (InItemName.IsNone())
	{
		ItemComboBox->SetSelectedItem(ItemNames[SelectedItemIndex]);
	}

	FString LocalName = ULTFunctions::GetRidOfTextureSuffix(InItemName).ToString();
	LocalName = ULTFunctions::GetRidOfHDRISuffix(*LocalName).ToString();

	const int32 Num = ItemNames.Num();
	for (int32 Index = 0; Index < Num; Index++)
	{
		if (ItemNames[Index]->Equals(LocalName))
		{
			SelectedItemIndex = Index;
			break;
		}
		//UE_LOG(LogTemp, Error, TEXT("Did not find the Item in the list."));
	}

	ItemComboBox->SetSelectedItem(ItemNames[SelectedItemIndex]);
}

FName ULTListManager::GetSelectedItemName()
{
	if (ItemNames.IsValidIndex(SelectedItemIndex))
	{
		return **ItemNames[SelectedItemIndex];
	}

	//UE_LOG(LogLighting, Error, TEXT("ItemNames has not valid index."));
	return {};
}

void ULTListManager::ReGenerateItemList(const TArray<FName>& InItemList)
{
	ItemNames.Empty();

	ItemNames.Add(MakeShareable(new FString(TEXT("None"))));

	for (int32 Index = 0; Index < InItemList.Num(); ++Index)
	{
		if (InItemList[Index].IsEqual(TEXT("None"))) { continue; }

		ItemNames.Add(MakeShareable(new FString(InItemList[Index].ToString())));
	}

	if (!ItemNames.IsValidIndex(SelectedItemIndex))
	{
		SelectedItemIndex = 0;
	}

	ItemComboBox->SetSelectedItem(ItemNames[SelectedItemIndex]);
}

void ULTListManager::SwitchToNextItem(bool bSwitchToNext)
{
	if (ItemNames.IsEmpty() || ItemNames.Num() == 0) { return; }

	SelectedItemIndex = SelectedItemIndex + (bSwitchToNext ? 1 : -1);

	if (SelectedItemIndex >= ItemNames.Num())
	{
		SelectedItemIndex = 0;
	}
	else if (SelectedItemIndex < 0)
	{
		SelectedItemIndex = ItemNames.Num() - 1;
	}

	const TSharedPtr<FString> SelectedItemName = ItemNames[SelectedItemIndex];
	ItemComboBox->SetSelectedItem(SelectedItemName);

	OnItemSelected.Broadcast(**SelectedItemName);
}

#undef LOCTEXT_NAMESPACE
