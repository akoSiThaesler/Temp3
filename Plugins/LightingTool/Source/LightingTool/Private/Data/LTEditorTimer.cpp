// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTEditorTimer.h"


bool ULTEditorTimer::StartTimer(const float InTime)
{
	if(InTime <= 0)
	{
		return false;
	}
	
	Time = InTime;
	bIsTimerActive = true;
	return true;
}

void ULTEditorTimer::StopTimer()
{
	TotalElapsedTime = 0.0f;
	bIsTimerActive = false;
}

void ULTEditorTimer::Tick(float DeltaTime)
{
	if (LastFrameNumberWeTicked == GFrameCounter){return;}

	if(bIsTimerActive)
	{
		TotalElapsedTime += DeltaTime;
		
		if(TotalElapsedTime >= Time)
		{
			bIsTimerActive = false;
			OnEditorTimerFinished.Broadcast();
		}
	}
	LastFrameNumberWeTicked = GFrameCounter;
}
