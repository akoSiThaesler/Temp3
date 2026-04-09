// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "Styling/SlateStyle.h"

class FLTToolStyle
{
public:
	static void InitializeToolStyle();
	static void ShutDownStyle();

private:
	static FName ToolStyleName;

	static TSharedRef<FSlateStyleSet> CreateToolSlateStyleSet();
	static TSharedPtr<FSlateStyleSet> CreatedToolSlateStyleSet;

public:
	static FName GetToolStyleName(){return ToolStyleName;}

	static TSharedRef<FSlateStyleSet> GetCreatedToolSlateStyleSet() {return CreatedToolSlateStyleSet.ToSharedRef();}

	static FName GetLightStyleNameByClassName(const FName& InClassName);
};