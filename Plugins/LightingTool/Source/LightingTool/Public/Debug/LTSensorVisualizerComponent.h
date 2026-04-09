// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LTSensorVisualizerComponent.generated.h"

UCLASS(ClassGroup=(Custom),meta=(BlueprintSpawnableComponent)) 
class LIGHTINGTOOL_API ULTSensorVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

	ULTSensorVisualizerComponent();

public:
	UPROPERTY(BlueprintReadWrite,Category = "Lighting Tool|Sensor")
	bool  DebugBounds = true;

	UPROPERTY(BlueprintReadWrite,Category = "Lighting Tool|Sensor")
	float  Height = 1000.0f;

	UPROPERTY(BlueprintReadWrite,Category = "Lighting Tool|Sensor")
	float  AngleWidth = 30.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Lighting Tool|Sensor")
	int32  NumSides = 32;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,DisplayName= "FOV Color",Category = "Lighting Tool|Sensor")
	FLinearColor FovColor  = FLinearColor::Green;
};
