// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeBool.h"

/**
 * Utility class to execute a function after a delay asynchronously.
 * Includes thread-safe checks to see if the task is in progress.
 */
class FAsyncDelayExecutor
{
public:
	static void ExecuteFunctionAfterDelay(TFunction<void()> FunctionToExecute, float DelayInSeconds);
	static bool IsTaskInProgress();
	static void ResetAndRestart(const TFunction<void()>& FunctionToExecute, float DelayInSeconds);

private:
	static FThreadSafeBool bIsTaskInProgress;
	static FCriticalSection Mutex;  // Mutex to protect task reset and restart
};
