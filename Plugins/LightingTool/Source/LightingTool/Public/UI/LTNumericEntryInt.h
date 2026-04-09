// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Widgets/SWidget.h"
#include "LTNumericEntryInt.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIntValueChanged, int32, OutValue);

UCLASS()
class LIGHTINGTOOL_API ULTNumericEntryInt : public UWidget
{
	GENERATED_BODY()

public:
	ULTNumericEntryInt();

	// Override to synchronize properties from UMG to Slate
	virtual void SynchronizeProperties() override;

	// Property to be set in the UMG editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
	FText HeaderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
	FText HeaderToolTipText;

	// Property to be set in the UMG editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
	int32 MinValue;

	// Property to be set in the UMG editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
	int32 MaxValue;

	// Step size property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool")
	int32 StepSize;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Lighting Tool")
	FOnIntValueChanged OnValueChangedSignature;

	UFUNCTION(BlueprintCallable, Category = "Lighting Tool")
	void SetInitialValue(int32 InValue);

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

	void OnValueChanged(int32 InNewValue);

private:
	TSharedPtr<class STextBlock> HeaderTextBlock;
	TSharedPtr<SNumericEntryBox<int32>> NumericEntryBox;
	TOptional<int32> CurrentValue;

	TOptional<int32> GetCurrentValue() const;
};
