// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/SWidget.h"
#include "LTNumericEntry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueChanged,float,OutValue);

class STextBlock;

UCLASS()
class LIGHTINGTOOL_API ULTNumericEntry : public UWidget
{
    GENERATED_BODY()

public:
    ULTNumericEntry();

    // Override to synchronize properties from UMG to Slate
    virtual void SynchronizeProperties() override;

    // Property to be set in the UMG editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
    FText HeaderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
    FText HeaderToolTipText;
    
    // Property to be set in the UMG editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
    float MinValue;

    // Property to be set in the UMG editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
    float MaxValue;

    UPROPERTY(BlueprintAssignable,BlueprintCallable, Category = "Lighting Tool")
    FOnValueChanged OnValueChangedSignature;

    UFUNCTION(BlueprintCallable, Category = "Lighting Tool")
    void SetInitialValue(float InValue);

protected:
    // UWidget interface
    virtual TSharedRef<SWidget> RebuildWidget() override;
    
    void OnValueChanged(float InNewValue);

private:
    TSharedPtr<STextBlock> HeaderTextBlock;
    TSharedPtr<SNumericEntryBox<float>> NumericEntryBox;
    TOptional<float> CurrentValue;

    TOptional<float> GetCurrentValue() const;
};
