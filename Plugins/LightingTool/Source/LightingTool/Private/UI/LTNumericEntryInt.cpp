// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTNumericEntryInt.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"

ULTNumericEntryInt::ULTNumericEntryInt()
{
    // Set default property values
    HeaderName = FText::FromString("Default Header");
    MinValue = 0;
    MaxValue = 4096;
    CurrentValue = 0;
    StepSize = 4; 
}

void ULTNumericEntryInt::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (HeaderTextBlock.IsValid())
    {
        HeaderTextBlock->SetText(HeaderName);
    }
}

void ULTNumericEntryInt::SetInitialValue(int32 InValue)
{
    CurrentValue = InValue;
}

TSharedRef<SWidget> ULTNumericEntryInt::RebuildWidget()
{
    return SNew(SBorder)
    .BorderImage(FAppStyle::GetNoBrush())
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .MinDesiredWidth(45)
            .VAlign(VAlign_Center)
            .ToolTipText(HeaderToolTipText)
            [
                SAssignNew(HeaderTextBlock, STextBlock)
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
                .Text(HeaderName)
            ]
        ]
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(SSpacer)
            .Size(FVector2D(5, 0))
        ]
        
        + SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(SBox)
            .MinDesiredWidth(50)
            [
                SAssignNew(NumericEntryBox, SNumericEntryBox<int32>)
                .Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
                .MinValue(TAttribute<TOptional<int32>>::CreateLambda([this]() { return MinValue; }))
                .MaxValue(TAttribute<TOptional<int32>>::CreateLambda([this]() { return MaxValue; }))
                .MinSliderValue(TAttribute<TOptional<int32>>::CreateLambda([this]() { return MinValue; }))
                .MaxSliderValue(TAttribute<TOptional<int32>>::CreateLambda([this]() { return MaxValue; }))
                .Value_UObject(this, &ULTNumericEntryInt::GetCurrentValue)
                .AllowSpin(true)
                .OnValueChanged_UObject(this, &ULTNumericEntryInt::OnValueChanged)
            ]
        ]
    ];
}

void ULTNumericEntryInt::OnValueChanged(int32 InNewValue)
{
    // Round the value to the nearest multiple of StepSize
    int32 RoundedValue = FMath::RoundToInt(InNewValue / static_cast<float>(StepSize)) * StepSize;
    CurrentValue = RoundedValue;
    OnValueChangedSignature.Broadcast(RoundedValue);
}

TOptional<int32> ULTNumericEntryInt::GetCurrentValue() const
{
    return CurrentValue;
}
