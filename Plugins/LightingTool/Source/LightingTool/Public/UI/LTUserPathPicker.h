// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ContentBrowserItemPath.h"
#include "Widgets/SCompoundWidget.h"
#include "LTUserPathPicker.generated.h"

class SLTUserPathPicker : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLTUserPathPicker)
	{}

	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
		SLATE_ARGUMENT(FContentBrowserItemPath, AssetPath)
		SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Callback when the selected asset path has changed. */
	void OnSelectAssetPath(const FString& InVirtualPath) { AssetPath.SetPathFromString(InVirtualPath, EContentBrowserPathType::Virtual); }

	/** Callback when the "ok" button is clicked. */
	FReply OnClickOk();

	/** Destroys the window when the operation is cancelled. */
	FReply OnClickCancel();

	/** A pointer to the window that is asking the user to select a parent class */
	TWeakPtr<SWindow> WeakParentWindow;

	FContentBrowserItemPath AssetPath;

	bool bPressedOk;
};


UCLASS()
class LIGHTINGTOOL_API ULTUserPathPicker : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static FString CreateAFolderPicker(const FName& InTitle);
};
