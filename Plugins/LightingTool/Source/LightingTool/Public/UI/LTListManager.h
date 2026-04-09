// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "LTListManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSelected,FName,OutItemName);


UCLASS()
class LIGHTINGTOOL_API ULTListManager : public UWidget
{
	GENERATED_BODY()

public:
	virtual void SynchronizeProperties() override;

	TSharedPtr<class STextComboBox> ItemComboBox;
private:
	TArray<TSharedPtr<FString>> ItemNames;

	int32 SelectedItemIndex = 0;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	UPROPERTY(BlueprintAssignable,Category = "Lighting Tool")
	FOnItemSelected OnItemSelected;
	
	UFUNCTION(BlueprintCallable,Category = "Lighting Tool")
	void SwitchToNextItem(bool bSwitchToNext);
	
	UFUNCTION(BlueprintCallable,Category = "Lighting Tool")
	void ReGenerateItemList(const TArray<FName>& InItemList);
	
	UFUNCTION(BlueprintCallable,Category = "Lighting Tool")
	void ChangeSelectedItemName(const FName& InItemName);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "Lighting Tool")
	FName GetSelectedItemName();
};
