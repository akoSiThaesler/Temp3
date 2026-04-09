// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CMMisc.h"
#include "CMCameraDefinitions.h"



DECLARE_DELEGATE_OneParam(FOnPresetImportProcessCompleted, ECMImportFailReason::Type)


class CAMERAMANAGER_API FPResetImportHandler
{
public:
	FPResetImportHandler();

	FPResetImportHandler(const FOnPresetImportProcessCompleted& InOnPresetImportProcessCompleted);

	void StartImportProcess();

	void RequestShutdown(ECMImportFailReason::Type InShutdownReason);

	FOnPresetImportProcessCompleted OnPresetImportProcessCompleted;

private:
	class UDataTable* TempTable = nullptr;

	TSharedPtr<class SWindow> PresetImportWindow;

	TArray<FName> PresetsToImport;

	TArray<FName> SpawnImportWindow(const TArray<FPresetImportData>& InPresets);

	void OnPresetImportRequested(const TArray<FName>& InSelectedPresetNames);
};