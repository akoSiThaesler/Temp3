// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "CMToolStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Styling/StyleColors.h"


FName FCMToolStyle::CMToolStyleName = FName("CameraManagerStyle");
TSharedPtr<FSlateStyleSet> FCMToolStyle::CreatedToolSlateStyleSet = nullptr;

void FCMToolStyle::InitializeToolStyle()
{	
	if(!CreatedToolSlateStyleSet.IsValid())
	{
		CreatedToolSlateStyleSet = CreateToolSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedToolSlateStyleSet);
	}
}

const ISlateStyle& FCMToolStyle::Get()
{
	if (const ISlateStyle* AppStyle = FSlateStyleRegistry::FindSlateStyle(CMToolStyleName))
	{
		return *AppStyle;
	}

	return FCoreStyle::GetCoreStyle();
}


TSharedRef<FSlateStyleSet> FCMToolStyle::CreateToolSlateStyleSet()
{	
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(CMToolStyleName));

	const FString IconDirectory = IPluginManager::Get().FindPlugin(TEXT("LightingTool"))->GetBaseDir() /"Resources";
	
	CustomStyleSet->SetContentRoot(IconDirectory);

	const FVector2D Icon16x16 (16.f,16.f);
	const FVector2D Icon20x20 (20.f,20.f);

	CustomStyleSet->Set("CameraManager.CameraManagerIcon",new FSlateImageBrush(IconDirectory/"CameraManager.png",Icon20x20));
	CustomStyleSet->Set("CameraManager.PilotIcon",new FSlateImageBrush(IconDirectory/"Pilot.png",Icon20x20));
	CustomStyleSet->Set("CameraManager.NoPilotIcon",new FSlateImageBrush(IconDirectory/"NoPilot.png",Icon20x20));
	CustomStyleSet->Set("CameraManager.ReplaceIcon",new FSlateImageBrush(IconDirectory/"Replace.png",Icon16x16));
	CustomStyleSet->Set("CameraManager.AddCameraIcon",new FSlateImageBrush(IconDirectory/"AddCamera.png",Icon20x20));

	
	FLinearColor InputA = FStyleColors::Input.GetSpecifiedColor();
	FLinearColor InputB = FStyleColors::Input.GetSpecifiedColor();
	InputA .A = .70;

	CustomStyleSet->Set("CameraManager.FloatingButtonStyle", FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(InputA, 3))
		.SetHovered(FSlateRoundedBoxBrush(InputB, 3))
		.SetPressed(FSlateRoundedBoxBrush(InputB, 3))
		.SetNormalForeground(FStyleColors::Foreground)
		.SetHoveredForeground(FStyleColors::ForegroundHover)
		.SetPressedForeground(FStyleColors::ForegroundHover)
		.SetDisabledForeground(FStyleColors::White25)
		.SetNormalPadding(FMargin(4))
		.SetPressedPadding(FMargin(4)));

	const FMargin ButtonMargins(3.f, 1.5f, 3.f, 1.5f);
	const FMargin PressedButtonMargins(4.0f, 2.5f, 4.0f, 2.5f);
	
	CustomStyleSet->Set("CameraManager.ImageWithButtonStyle", FButtonStyle()
	.SetNormal(FSlateNoResource())
	.SetHovered(FSlateRoundedBoxBrush(FStyleColors::Dropdown, 4.0f))
	.SetPressed(FSlateRoundedBoxBrush(FStyleColors::Dropdown, 4.0f))
	.SetDisabled(FSlateNoResource())
	.SetNormalForeground(FStyleColors::Foreground)
	.SetHoveredForeground(FStyleColors::ForegroundHover)
	.SetPressedForeground(FStyleColors::ForegroundHover)
	.SetDisabledForeground(FStyleColors::Foreground)
	.SetNormalPadding(ButtonMargins)
	.SetPressedPadding(PressedButtonMargins));
	
	return CustomStyleSet;
}


void FCMToolStyle::ShutDownStyle()
{
	if(CreatedToolSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedToolSlateStyleSet);
		CreatedToolSlateStyleSet.Reset();
	}
}

