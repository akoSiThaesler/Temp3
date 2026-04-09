// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "CMMisc.h"
#include "UnrealEdMisc.h"
#include "CMSubsystem.generated.h"

struct FCineCameraPresetData;
struct FCMCineCameraData;
class ACineCameraActor;
class UUserWidget;
class FLevelEditorModule;
class UDataTable;

DECLARE_DELEGATE_TwoParams(FOnPresetImported, FName /*PresetName*/, bool /*bIsOverridden*/)

/**
 * @class UCameraManagerSubsystem
 * @brief A class that manages scene settings in the Unreal Engine editor.
 *
 * This class is responsible for managing camera presets, importing and exporting presets,
 * and handling the user interface for these operations.
 */
UCLASS()
class CAMERAMANAGER_API UCameraManagerSubsystem : public UEditorSubsystem
{
 GENERATED_BODY()

public:

    UCameraManagerSubsystem();

    // Initializes the subsystem.
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Deinitializes the subsystem.
    virtual void Deinitialize() override;

    // Adds a cine camera as a preset.
    void AddCineCameraAsPreset(const FName& InPresetName,const FName& InDescription, const ACineCameraActor* InCineCameraActor,const bool& bIsPresetAlreadyAvailable) const;

    // Deletes a preset.
    bool DeleteAPreset(const FName& InPresetName) const;

    // Loads a preset to a camera.
    void LoadPresetToCamera(const FName& PresetName, ACineCameraActor* InCineCamera) const;

    // Loads a preset to a camera.
    void LoadPresetToCamera(const FCineCameraPresetData& InPresetData, ACineCameraActor* InCineCamera) const;

    // Returns the names of all presets.
    TArray<FName> GetPresetNames() const;

    // Returns the names and descriptions of all presets.
    TArray<PresetDescPair> GetPresetNamesWithDesc() const;

    // Checks if a preset is available.
    bool IsPresetAvailable(const FName& InPresetName) const;

	// Checks if a preset is available.
	bool IsAnyPresetAvailable() const;

    // Imports presets.
    void ImportPresets(const TArray<FCineCameraPresetData>& InImportedPresets) const;

    // Exports presets.
    void ExportPresets(const TArray<FName>& InPresetNames, const FString& InFilePath) const;
	
	
	FOnPresetImported OnPresetImported;

private:
    UPROPERTY()
    TObjectPtr<UDataTable> CameraPresetData;

    // Overwrites an existing preset.
    void OverwriteExistingPreset(const FName& InPresetName, const FName& InDescription, const FCMCineCameraData& FCMCineCameraData) const;

    // Loads the presets table.
    void LoadPresetsTable();

public:
    // Toggles the camera manager tool tab.
    void ToggleCameraManagerToolTab();
};
