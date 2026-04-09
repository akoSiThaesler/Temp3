// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTSensorVisualizer.h"
#include "LTSensorVisualizerComponent.h"
#include "SceneManagement.h"

void FLTSensorVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    if (!PDI || !Component) return;

    const auto SensorVisComp = Cast<ULTSensorVisualizerComponent>(Component);
    if (!SensorVisComp) { return; }

    // Actor's location
    const FVector ActorLocation = SensorVisComp->GetOwner()->GetActorLocation();

    // Up, forward, and right vectors (negative up vector for downward direction)
    const FVector UpVector = SensorVisComp->GetOwner()->GetActorUpVector();
    const FVector ForwardVector = SensorVisComp->GetOwner()->GetActorForwardVector();
    const FVector RightVector = SensorVisComp->GetOwner()->GetActorRightVector();
    
    // Calculate radius from height and angle
    const float Radius = FMath::Tan(FMath::DegreesToRadians(SensorVisComp->AngleWidth/*SensorVisComp->AngleWidth / 2.0f*/)) * SensorVisComp->Height;

    // The tip of the cone is at the actor's location
    const FVector Tip = ActorLocation;

    // Base center of the cone, in the downward direction
    const FVector BaseCenter = ActorLocation - (UpVector * SensorVisComp->Height);

    // Angle between each segment in radians
    const float AngleStep = 2.0f * PI / SensorVisComp->NumSides;

    // Calculate initial base point
    FVector PrevPoint = BaseCenter + Radius * (FMath::Cos(0.0f) * RightVector + FMath::Sin(0.0f) * ForwardVector);

    for (int32 i = 1; i <= SensorVisComp->NumSides; ++i)
    {
        const float CurrAngle = i * AngleStep;
        FVector CurrPoint = BaseCenter + Radius * (FMath::Cos(CurrAngle) * RightVector + FMath::Sin(CurrAngle) * ForwardVector);

        // Draw the base
        PDI->DrawLine(PrevPoint, CurrPoint, SensorVisComp->FovColor, SDPG_World, 0.0f, 0.0f, true);

        // Draw lines from the tip to the base
        PDI->DrawLine(Tip, CurrPoint, SensorVisComp->FovColor, SDPG_World, 0.0f, 0.0f, true);

        PrevPoint = CurrPoint;
    }
}


