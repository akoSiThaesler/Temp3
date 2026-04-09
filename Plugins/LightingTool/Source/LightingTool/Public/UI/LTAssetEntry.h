// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "LTData.h"
#include "Blueprint/UserWidget.h"
#include "LTAssetEntry.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAssetViewDataChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoordIndexSubmittedSignature, FName, InObjectPath,int32,InIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResolutionSubmittedSignature, FName, InObjectPath,int32,InResolution);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewAssetParamSubmittedSignature, bool,IsItForResolution,int32,InResolution);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEntryClickedSignature,int32,EntryIndex,bool,IsControlDown,bool,IsShiftDown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnListViewAssetSelectionChangedSignature,bool,NewSelection);

UCLASS()
class LIGHTINGTOOL_API ULTAssetEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,meta=(ExposeOnSpawn = true),Category = "Lighting Tool")
	FLTAssetViewData AssetViewData;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnAssetViewDataChangedSignature OnAssetViewDataChanged;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnCoordIndexSubmittedSignature OnCoordIndexSubmitted;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnResolutionSubmittedSignature OnResolutionSubmitted;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnEntryClickedSignature OnEntryClicked;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnListViewAssetSelectionChangedSignature OnListViewAssetSelectionChanged;

	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnNewAssetParamSubmittedSignature OnNewAssetParamSubmitted;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
