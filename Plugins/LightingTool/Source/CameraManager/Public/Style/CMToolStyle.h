// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "Styling/SlateStyle.h"

class FCMToolStyle
{
public:
	static void InitializeToolStyle();
	static void ShutDownStyle();
	static const ISlateStyle& Get();
	
private:
	static FName CMToolStyleName;
	
	static TSharedRef<FSlateStyleSet> CreateToolSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedToolSlateStyleSet;

public:
	static FName GetToolStyleName(){return CMToolStyleName;}

	static TSharedRef<FSlateStyleSet> GetCreatedToolSlateStyleSet() {return CreatedToolSlateStyleSet.ToSharedRef();}
};