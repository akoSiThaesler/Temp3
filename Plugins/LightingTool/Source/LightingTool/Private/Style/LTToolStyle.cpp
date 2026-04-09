// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "LTToolStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"


FName FLTToolStyle::ToolStyleName = FName("LightingToolStyle");
TSharedPtr<FSlateStyleSet> FLTToolStyle::CreatedToolSlateStyleSet = nullptr;

void FLTToolStyle::InitializeToolStyle()
{	
	if(!CreatedToolSlateStyleSet.IsValid())
	{
		CreatedToolSlateStyleSet = CreateToolSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedToolSlateStyleSet);
	}
}

TSharedRef<FSlateStyleSet> FLTToolStyle::CreateToolSlateStyleSet()
{	
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(ToolStyleName));

	const FString IconDirectory = 
	IPluginManager::Get().FindPlugin(TEXT("LightingTool"))->GetBaseDir() /"Resources";
	
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon32x32 (32.f,32.f);
	const FVector2D Icon64x64 (64.f,64.f);
	const FVector2D Icon128x128 (128.f,128.f);

	CustomStyleSet->Set("LightingTool.ToolbarIcon",
	new FSlateImageBrush(IconDirectory/"Toolbar.png",Icon32x32));
	
	CustomStyleSet->Set("LightingTool.LightMapIcon",
	new FSlateImageBrush(IconDirectory/"LightMap.png",Icon32x32));

	CustomStyleSet->Set("LightingTool.LightsToolIcon",
	new FSlateImageBrush(IconDirectory/"LightsTool.png",Icon32x32));

	CustomStyleSet->Set("LightingTool.HDRIIcon",
	new FSlateImageBrush(IconDirectory/"HDRI.png",Icon32x32));

	CustomStyleSet->Set("LightingTool.CameraManagerIcon",
	new FSlateImageBrush(IconDirectory/"CameraManager.png",Icon32x32));
	
	CustomStyleSet->Set("LightingTool.LightRenderSettingsIcon",
	new FSlateImageBrush(IconDirectory/"LightRenderSettings.png",Icon32x32));

	CustomStyleSet->Set("LightingTool.PointLightIcon",
	new FSlateImageBrush(IconDirectory/"PointLight.png",Icon128x128));

	CustomStyleSet->Set("LightingTool.SpotLightIcon",
	new FSlateImageBrush(IconDirectory/"SpotLight.png",Icon128x128));

	CustomStyleSet->Set("LightingTool.RectLightIcon",
	new FSlateImageBrush(IconDirectory/"RectLight.png",Icon128x128));
	
	CustomStyleSet->Set("LightingTool.LightmapVolumeIcon",
	new FSlateImageBrush(IconDirectory/"LightmapVolume.png",Icon128x128));

	CustomStyleSet->Set("LightingTool.HelpIcon",
	new FSlateImageBrush(IconDirectory/"Help.png",Icon32x32));
	
	const FButtonStyle LTButtonStyle = FButtonStyle()
	.SetNormal(FSlateNoResource())
	.SetHovered(FSlateNoResource())
	.SetPressed(FSlateNoResource())
	.SetNormalPadding(FMargin(0,0,0,0))
	.SetPressedPadding(FMargin(1,1,1,1));

	CustomStyleSet->Set("LightingTool.LTButtonStyle",LTButtonStyle);

	
	
	return CustomStyleSet;
}

FName FLTToolStyle::GetLightStyleNameByClassName(const FName& InClassName)
{
	if(InClassName.IsEqual(TEXT("AutoPointLight_C")))
	{
		return TEXT("LightingTool.PointLightIcon");
	}
	if(InClassName.IsEqual(TEXT("AutoSpotLight_C")))
	{
		return TEXT("LightingTool.SpotLightIcon");
	}
	if(InClassName.IsEqual(TEXT("AutoRectLight_C")))
	{
		return TEXT("LightingTool.RectLightIcon");
	}
	return TEXT("LightingTool.MotionSensorIcon");
}

void FLTToolStyle::ShutDownStyle()
{
	if(CreatedToolSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedToolSlateStyleSet);
		CreatedToolSlateStyleSet.Reset();
	}
}
