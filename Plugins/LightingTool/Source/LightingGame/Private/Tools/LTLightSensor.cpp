// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "Tools/LTLightSensor.h"
#include "Components/LightComponent.h"
#include "Engine/Light.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ALTLightSensor::ALTLightSensor(): SensorBehavior()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALTLightSensor::BeginPlay()
{
	Super::BeginPlay();
	
	for(const auto CurrentLight : LightsToControl)
	{
		CurrentLight->GetLightComponent()->SetVisibility(false);
	}
}

void ALTLightSensor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!bCanLightTick){return;}

	TotalElapsedTime += DeltaTime;
	
	if(TotalElapsedTime >= CheckFrequency)
	{
		LightTick();
		
		TotalElapsedTime = 0;
	}
}


bool ALTLightSensor::IsTargetInFOV(const FVector& InTargetLocation) const
{
	const FVector TargetInLocal = GetActorTransform().InverseTransformPosition(InTargetLocation);
	const float HeightAlongCone = FVector::DotProduct(TargetInLocal,GetActorUpVector() * -1.0f);

	if(HeightAlongCone >= 0 && HeightAlongCone <= Height)
	{
		const float RadiusAtHeight = FMath::Tan(FMath::DegreesToRadians(AngleWidth)) * HeightAlongCone;

		if(TargetInLocal.Size2D() < RadiusAtHeight)
		{
			return true;
		}
	}
	return false;
}

bool ALTLightSensor::IsPlayerMoving() const
{
	if (const auto PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
	{
		return PlayerPawn->GetVelocity().SizeSquared() > MotionDetectionSensitivity * MotionDetectionSensitivity;
	}
	return false;
}


