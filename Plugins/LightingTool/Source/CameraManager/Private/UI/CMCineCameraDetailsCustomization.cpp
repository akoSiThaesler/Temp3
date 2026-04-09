// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CameraManager/Public/UI/CMCineCameraDetailsCustomization.h"
#include "DetailLayoutBuilder.h"

// List of categories to hide
static const TArray<FName> HiddenCineCameraCategories = {
	"Transform", "Mobility", "Tags", "Cooking", "Replication", "HLOD",
	"Actor", "Physics", "AssetUserData", "ComponentReplication", "Collision",
	"Variable", "Activation", "LOD", "Tick", "WorldPartition", "DataLayers"
};

TSharedRef<IDetailCustomization> ICMCineCameraDetailsCustomization::MakeInstance()
{
	return MakeShareable(new ICMCineCameraDetailsCustomization());
}

void ICMCineCameraDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Loop through each category and hide it
	for (const FName& Category : HiddenCineCameraCategories)
	{
		DetailLayout.HideCategory(Category);
	}
}



