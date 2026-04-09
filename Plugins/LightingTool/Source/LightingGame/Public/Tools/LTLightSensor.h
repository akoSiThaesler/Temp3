// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTLightSensor.generated.h"

class ALight;

UENUM(BlueprintType, Category = "Lighting Tool")
enum class ELightSensorBehavior: uint8
{
	Permanent		UMETA(DisplayName = "Permanent",ToolTip = "Permanently turns the light on"),
	OnOff			UMETA(DisplayName = "On & Off",ToolTip = "Turns of the light when the objective leaves the detection area"),
	Timer			UMETA(DisplayName = "Timer",ToolTip = "Turns on the lights for the set time"),
};

UCLASS()
class LIGHTINGGAME_API ALTLightSensor : public AActor
{
	GENERATED_BODY()

public:
	ALTLightSensor();
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,DisplayName = "Follow Player", Category = "Sensor Settings|Detection")
	bool bFollowPlayer = true;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Sensor Settings|Detection")
	TArray<ALight*> LightsToControl;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Sensor Settings|Behavior")
	ELightSensorBehavior SensorBehavior = ELightSensorBehavior::Permanent;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = "SensorBehavior == ELightSensorBehavior::Timer",EditConditionHides), Category = "Sensor Settings|Behavior")
	float LightingTime = 10;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(EditCondition = "SensorBehavior == ELightSensorBehavior::Timer",EditConditionHides,ToolTip = "Minimum velocity required to detect motion"), Category = "Sensor Settings|Behavior")
	float MotionDetectionSensitivity = 100.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Sensor Settings|Sensor")
	float Height = 1000.0f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Sensor Settings|Sensor")
	float AngleWidth = 30.0f;

	UPROPERTY(EditAnywhere,Category = "Sensor Settings|Optimization")
	float CheckFrequency = 0.1;

private:
	float TotalElapsedTime = 0;
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BLueprintImplementableEvent,BlueprintCallable, Category = "Light Functions")
	void LightTick();
	
private:
	UFUNCTION(BlueprintCallable, Category = "Light Functions")
	bool IsTargetInFOV(const FVector& InTargetLocation) const;

protected:
	UPROPERTY(BlueprintReadWrite,Category = "LightingTool|Sensor Settings")
	bool bCanLightTick = true;
	
	UPROPERTY(BlueprintReadWrite,Category = "LightingTool|Sensor Settings")
	bool bIsDetected = false;

	UPROPERTY(BlueprintReadWrite,Category = "LightingTool|Sensor Settings")
	bool bIsLightsOn = false;

public:
	UFUNCTION(BlueprintPure,Category = "LightingTool|Lightmap")
	bool IsPlayerMoving() const;
	
};
