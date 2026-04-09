// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "CMDelayedDelegateExecutor.h"
#include "Async/Async.h"


FThreadSafeBool FAsyncDelayExecutor::bIsTaskInProgress = false;

FCriticalSection FAsyncDelayExecutor::Mutex;

void FAsyncDelayExecutor::ExecuteFunctionAfterDelay(TFunction<void()> FunctionToExecute, float DelayInSeconds)
{
	Mutex.Lock();
	if (bIsTaskInProgress)
	{
		Mutex.Unlock();
		return;
	}
	bIsTaskInProgress = true;
	Mutex.Unlock();

	Async(EAsyncExecution::ThreadPool, [FunctionToExecute, DelayInSeconds]()
	{
		FPlatformProcess::Sleep(DelayInSeconds);

		Async(EAsyncExecution::TaskGraphMainThread, [FunctionToExecute]()
		{
			FunctionToExecute();
			bIsTaskInProgress = false;
		});
	});
}

bool FAsyncDelayExecutor::IsTaskInProgress()
{
	return bIsTaskInProgress;
}

void FAsyncDelayExecutor::ResetAndRestart(const TFunction<void()>& FunctionToExecute, float DelayInSeconds)
{
	Mutex.Lock();

	bIsTaskInProgress = false;
	Mutex.Unlock();

	// Immediately restart the task
	ExecuteFunctionAfterDelay(FunctionToExecute, DelayInSeconds);
}
