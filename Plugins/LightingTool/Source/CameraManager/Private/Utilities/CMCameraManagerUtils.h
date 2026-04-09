// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


class AActor;
class ACineCameraActor;
class UDataTable;

namespace FCMCameraUtils
{
	void InsertNewPresetNameToCineCamera(ACineCameraActor* InCameraActor, const FName& InNewTag);
	
	FName GetPresetNameFromCineCamera(const TArray<FName>& InPresetNames, ACineCameraActor* InCineCameraActor);
	
	bool RemovePresetNameFromCineCamera(ACineCameraActor* InCineCameraActor);

	bool DoesContainsThePreset(const FName& InPresetName, ACineCameraActor* InCineCameraActor);

	TArray<ACineCameraActor*> FilterCineCameraActors(const TArray<UObject*>& InActors);

	TArray<ACineCameraActor*> GetSelectedCineCameraActors();

	TArray<AActor*> GetPilotedCameraActors();
	TArray<ACineCameraActor*> GetAllCineCameraActors();
	
	bool AlphanumericCompare(const FString& A, const FString& B);

	ACineCameraActor* CreateCineCameraOnView();
	
}

namespace FCMPresetUtils
{
	bool IsValidNameConvention(const FText& InText);
	
	bool IsValidNameConventionFromName(const FName& InName);

	FName CleanPresetName(const FName& Name);
}

namespace FCMFileUtils
{
	bool OpenImportPresetDialog(FString& OutFilePath);

	bool OpenExportPresetDialog(const FString& InFilePath, FString& OutFilePath);
	
	bool OpenDirectoryPicker(FString& OutDirectory, const FString& DefaultPath);

	FString GetLastDirectory();
	
	void SetLastDirectory(const FString& InLastDirectory);

	FString GetDefaultExportPath();

	bool IsValidPathForSaving(const FString& Path);
	
	UDataTable* CreateTempDataTableWithJsonFile(UScriptStruct* InParentTable, const FString& InFilePath);
	void ClearTempDataTable(UDataTable* InDataTable);

}
