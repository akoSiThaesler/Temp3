// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "Templates/SharedPointer.h"

// This file defines delegates and structures used for handling preset import and export events within the application.

// Type definition for a pair of FName, representing a preset name and its description
typedef TPair<FName, FName> PresetDescPair;

// Delegate declarations for preset export and import events
// FOnPresetExportRequested: Triggered when a preset export is requested, passing the list of preset names and the output file path
DECLARE_DELEGATE_TwoParams(FOnPresetExportRequested, const TArray<FName>& OutPresets, const FString& OutFilePath);

// FOnPresetImportRequested: Triggered when a preset import is requested, passing the list of preset names
DECLARE_DELEGATE_OneParam(FOnPresetImportRequested, const TArray<FName>& OutPresets);

// Structure representing data related to preset import
struct FPresetImportData
{
	bool bIsAlreadyAvailable; // Indicates if the preset is already available
	PresetDescPair NameAndDesc; // Pair containing the preset name and description

	// Default constructor initializing bIsAlreadyAvailable to false
	FPresetImportData();
    
	// Parameterized constructor initializing the structure with provided values
	FPresetImportData(bool bIsAlreadyAvailable, const PresetDescPair& NameAndDesc);
};

// Inline implementation of the default constructor
inline FPresetImportData::FPresetImportData() : bIsAlreadyAvailable(false)
{
}

// Inline implementation of the parameterized constructor
inline FPresetImportData::FPresetImportData(bool bIsAlreadyAvailable, const PresetDescPair& NameAndDesc) 
	: bIsAlreadyAvailable(bIsAlreadyAvailable), NameAndDesc(NameAndDesc)
{
}
