// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once


#include "CoreMinimal.h"


namespace ECMImportFailReason
{
	enum Type
	{
		None,
		UserCancelled,
		NoValidData,
		UnknownError,
	};
}