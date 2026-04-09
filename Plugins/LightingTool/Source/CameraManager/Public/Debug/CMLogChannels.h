// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCameraManager, Log, All);

#define CHECK_AND_RETURN_WITH_ERROR(Condition, LogMessage) \
if (Condition) \
{ \
UE_LOG(LogCameraManager,Error, LogMessage); \
return; \
}