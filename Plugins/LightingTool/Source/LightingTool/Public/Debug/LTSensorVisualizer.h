// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class LIGHTINGTOOL_API FLTSensorVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
};
