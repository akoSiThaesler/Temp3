// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTNumericEntry.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"

ULTNumericEntry::ULTNumericEntry()
{
    // Set default property values
    HeaderName = FText::FromString("Default Header");
    MinValue = 0.0f;
    MaxValue = 100.0f;
    CurrentValue = 0.0f;
}

void ULTNumericEntry::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (HeaderTextBlock.IsValid())
    {
        HeaderTextBlock->SetText(HeaderName);
    }
}

void ULTNumericEntry::SetInitialValue(float InValue)
{
    FString ValueString = FString::Printf(TEXT("%.4f"), InValue);
    float TruncatedValue = FCString::Atof(*ValueString);
    CurrentValue = TruncatedValue;
}

TSharedRef<SWidget> ULTNumericEntry::RebuildWidget()
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
            .ToolTipText(HeaderToolTipText)
            //.MinDesiredHeight(20)
            .VAlign(VAlign_Center)
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
            //.HeightOverride()
            [
                SAssignNew(NumericEntryBox, SNumericEntryBox<float>)
                .Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
                .MinValue(TAttribute<TOptional<float>>::CreateLambda([this]() { return MinValue; }))
                .MaxValue(TAttribute<TOptional<float>>::CreateLambda([this]() { return MaxValue; }))
                .MinSliderValue(TAttribute<TOptional<float>>::CreateLambda([this]() { return MinValue; }))
                .MaxSliderValue(TAttribute<TOptional<float>>::CreateLambda([this]() { return MaxValue; }))
                .Value_UObject(this, &ULTNumericEntry::GetCurrentValue)
                .AllowSpin(true)
                .OnValueChanged_UObject(this, &ULTNumericEntry::OnValueChanged)
            ]
        ]
    ];
}

void ULTNumericEntry::OnValueChanged(float InNewValue)
{
    float Multiplier = FMath::Pow(10.0f, 4);
    float TruncatedValue = FMath::FloorToFloat(InNewValue * Multiplier) / Multiplier;
    
    CurrentValue = TruncatedValue;
    OnValueChangedSignature.Broadcast(TruncatedValue);
}


TOptional<float> ULTNumericEntry::GetCurrentValue() const
{
    return CurrentValue;
}
