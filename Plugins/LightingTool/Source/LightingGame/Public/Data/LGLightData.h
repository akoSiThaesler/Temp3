// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "LGLightData.generated.h"

UENUM(BlueprintType, Category = "Lighting Tool")
enum class ELGLightTypes : uint8
{
	PointLight		UMETA(DisplayName = "Point Light"),
	RectLight		UMETA(DisplayName = "Rect Light"),
	SpotLight		UMETA(DisplayName = "Spot Light"),
};