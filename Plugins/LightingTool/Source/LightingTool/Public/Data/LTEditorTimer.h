// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Tickable.h"
#include "LTEditorTimer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLTEditorTimerFinishSignature);
UCLASS(BlueprintType,DisplayName = "Editor Timer")
class LIGHTINGTOOL_API ULTEditorTimer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

#pragma region TickSetup

	bool bIsTimerActive = false;
	
	float TotalElapsedTime;

	float Time = 5.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Lighting Tool")
	bool StartTimer(const float InTime);

	UFUNCTION(BlueprintCallable, Category = "Lighting Tool")
	void StopTimer();
	
	virtual void Tick( float DeltaTime ) override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( FMyTickableThing, STATGROUP_Tickables );
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return true;
	}
	virtual bool IsTickableInEditor() const override
	{
		return true;
	}

private:
	uint32 LastFrameNumberWeTicked = INDEX_NONE;

#pragma endregion TickSetup

public:
	UPROPERTY(BlueprintAssignable)
	FLTEditorTimerFinishSignature OnEditorTimerFinished;
};
