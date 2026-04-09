// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class ICMCineCameraDetailsCustomization : public IDetailCustomization
{
public:
	// Makes a new instance of this detail layout class
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

private:
	//We can add parameter and to constructor to pass the camera actor when we calling the FOnGetDetailCustomizationInstance delegate
	//ICMCineCameraDetailsCustomization();
};
