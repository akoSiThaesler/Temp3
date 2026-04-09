// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTAssetEntry.h"

FReply ULTAssetEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if(InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnEntryClicked.Broadcast(AssetViewData.AssetIndex,InMouseEvent.IsControlDown(),InMouseEvent.IsShiftDown());
	}
	return FReply::Handled();
}
