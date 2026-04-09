// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMSubsystem.h"
#include "CineCameraActor.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "CMCameraPresetData.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "FileHelpers.h"
#include "CMToolAssetData.h"
#include "JsonObjectConverter.h"
#include "CMCameraUIManager.h"
#include "CMDebug.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Utilities/CMCameraManagerUtils.h"
#include "Serialization/JsonWriter.h"

UCameraManagerSubsystem::UCameraManagerSubsystem(): CameraPresetData()
{
}

void UCameraManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadPresetsTable();
}

void UCameraManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

#pragma region CameraManager

void UCameraManagerSubsystem::LoadPresetToCamera(const FName& PresetName, ACineCameraActor* InCineCamera) const
{
	if(!IsValid(CameraPresetData)){return;}
	
	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();
	if(PresetMap.Num() == 0){return;}
	
	for (const TTuple<FName,  unsigned char*>& Pair : PresetMap)
	{
		if (const FCineCameraPresetData* RowData = reinterpret_cast<const FCineCameraPresetData*>(Pair.Value))
		{
			if(RowData->PresetName.IsEqual(PresetName))
			{
				FCMCineCameraData::LoadPreset(InCineCamera, RowData->CineCameraData);

				FCMCameraUtils::InsertNewPresetNameToCineCamera(InCineCamera,PresetName);
				return;
			}
		}
	}
}

void UCameraManagerSubsystem::LoadPresetToCamera(const FCineCameraPresetData& InPresetData,ACineCameraActor* InCineCamera) const
{
	if(!IsValid(CameraPresetData)){return;}
	
	FCMCineCameraData::LoadPreset(InCineCamera, InPresetData.CineCameraData);

	FCMCameraUtils::InsertNewPresetNameToCineCamera(InCineCamera,InPresetData.PresetName);
}

TArray<FName> UCameraManagerSubsystem::GetPresetNames() const
{
	if(!IsValid(CameraPresetData)){return {};}

	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();
	if(PresetMap.Num() == 0){return {};}
	
	TArray<FName> LocalPresetNames;

	for (const TTuple<FName,  unsigned char*>& Pair : PresetMap)
	{
		if (const FCineCameraPresetData* RowData = reinterpret_cast<const FCineCameraPresetData*>(Pair.Value))
		{
			LocalPresetNames.Add(RowData->PresetName);
		}
	}
	return LocalPresetNames;
}


TArray<PresetDescPair> UCameraManagerSubsystem::GetPresetNamesWithDesc() const
{
	// Check if CameraPresetData is valid
	if (!IsValid(CameraPresetData)) { return {}; }
    
	// Get the row map from CameraPresetData
	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();
	if (PresetMap.Num() == 0) { return {}; }
    
	// Prepare the local array to store preset names and descriptions
	TArray<PresetDescPair> LocalPresetNames;
	LocalPresetNames.Reserve(PresetMap.Num()); // Reserve memory to avoid multiple reallocations

	// Iterate over the preset map and populate the local array
	for (const TTuple<FName, uint8*>& Pair : PresetMap)
	{
		// Safely reinterpret the data and check if it's valid
		if (const FCineCameraPresetData* RowData = reinterpret_cast<const FCineCameraPresetData*>(Pair.Value))
		{
			LocalPresetNames.Add(PresetDescPair(RowData->PresetName, RowData->Description));
		}
	}

	// Return the populated array
	return LocalPresetNames;
}



bool UCameraManagerSubsystem::IsPresetAvailable(const FName& InPresetName) const
{
	if(!IsValid(CameraPresetData)){return false;}
	
	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();
	if(PresetMap.Num() == 0){return false;}

	for (const auto& Pair : PresetMap)
	{
		if (const FCineCameraPresetData* RowData = reinterpret_cast<const FCineCameraPresetData*>(Pair.Value))
		{
			if(RowData->PresetName.IsEqual(InPresetName))
			{
				return true;
			}
		}
	}
	
	return false;
}

bool UCameraManagerSubsystem::IsAnyPresetAvailable() const
{
	if(!IsValid(CameraPresetData)){return false;}
	
	return !CameraPresetData->GetRowMap().IsEmpty();
}

void UCameraManagerSubsystem::ImportPresets(const TArray<FCineCameraPresetData>& InImportedPresets) const
{
	if(!IsValid(GEditor)){return;}
	
	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!IsValid(EditorWorld))
	{
		UE_LOG(LogCameraManager, Error, TEXT("EditorWorld not found, unable to refresh camera infos!"));
		return;
	}
	
	TArray<FName> OwnedPresetNames = GetPresetNames();
	
	for(auto CurrentPreset : InImportedPresets)
	{
		bool bIsPresetAlreadyAvailable = OwnedPresetNames.Contains(CurrentPreset.PresetName);

		const FName GeneratedRowName = DataTableUtils::MakeValidName(*CurrentPreset.PresetName.ToString());
			
		CameraPresetData->AddRow(GeneratedRowName,CurrentPreset);
		
		if(bIsPresetAlreadyAvailable)
		{
			for (TActorIterator<ACineCameraActor> CameraIt(EditorWorld); CameraIt; ++CameraIt)
			{
				if(FCMCameraUtils::DoesContainsThePreset(CurrentPreset.PresetName,*CameraIt))
				{
					LoadPresetToCamera(CurrentPreset,*CameraIt);
				}
			}
		}
		OnPresetImported.ExecuteIfBound(CurrentPreset.PresetName,bIsPresetAlreadyAvailable);
	}
}

void AddRowsToDataTable(const FString& JsonFilePath)
{
    // Load the JSON file
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
    {
        UE_LOG(LogCameraManager, Error, TEXT("Failed to load JSON file: %s"), *JsonFilePath);
        return;
    }

    // Parse the JSON data
    TArray<TSharedPtr<FJsonValue>> JsonArray;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, JsonArray) || !JsonArray.Num())
    {
        UE_LOG(LogCameraManager, Error, TEXT("Failed to parse JSON file: %s"), *JsonFilePath);
        return;
    }

    // Iterate through each JSON object and add it to the data table
    for (const TSharedPtr<FJsonValue>& JsonValue : JsonArray)
    {
        TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
        if (!JsonObject.IsValid())
        {
            continue;
        }

        FCineCameraPresetData NewRow;
        if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FCineCameraPresetData::StaticStruct(), &NewRow, 0, 0))
        {
            UE_LOG(LogCameraManager, Warning, TEXT("Failed to convert JSON to struct for row: %s. Attempting to handle invalid values."), *JsonObject->GetStringField(FString(TEXT("PresetName"))));
            continue;
        }

        // Manually handle CineCameraData conversion
        if (JsonObject->HasField(FString(TEXT("CineCameraData"))))
        {
        	const TSharedPtr<FJsonObject> CineCameraDataObject = JsonObject->GetObjectField(FString(TEXT("CineCameraData")));
            FCMCineCameraData CineCameraData;
            if (!FJsonObjectConverter::JsonObjectToUStruct(CineCameraDataObject.ToSharedRef(), FCMCineCameraData::StaticStruct(), &CineCameraData, 0, 0))
            {
                continue;
            }
            NewRow.CineCameraData = CineCameraData;
        }
    }
}

void UCameraManagerSubsystem::ExportPresets(const TArray<FName>& InPresetNames, const FString& InFilePath) const
{
	// Convert the data table to JSON string
	FString PresetData = CameraPresetData->GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs);

	// Replace all instances of (INVALID) with 0
	PresetData.ReplaceInline(TEXT("(INVALID)"), TEXT("0"));

	// Parse the JSON string into a JSON array
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PresetData);
	if (!FJsonSerializer::Deserialize(Reader, JsonArray) || JsonArray.Num() == 0)
	{
		UE_LOG(LogCameraManager, Warning, TEXT("Failed to parse JSON data"));
		return;
	}

	// Filter the presets based on InPresetNames
	TArray<TSharedPtr<FJsonValue>> FilteredPresetArray;
	for (const TSharedPtr<FJsonValue>& Value : JsonArray)
	{
		TSharedPtr<FJsonObject> PresetObject = Value->AsObject();
		FString PresetName;
		if (PresetObject->TryGetStringField(TEXT("PresetName"), PresetName) && InPresetNames.Contains(FName(*PresetName)))
		{
			FilteredPresetArray.Add(Value);
		}
	}

	// Serialize the filtered JSON array to a string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FilteredPresetArray, Writer);
    
	// Save the filtered JSON string to a file
	if (FFileHelper::SaveStringToFile(OutputString, *InFilePath))
	{
		FCMFileUtils::SetLastDirectory(FPaths::GetPath(InFilePath));
		CMDebug::ShowNotifyInfo(TEXT("Presets exported successfully!"));
	}
	else
	{
		CMDebug::ShowNotifyError(TEXT("Failed to export presets!"));
	}
}


void UCameraManagerSubsystem::OverwriteExistingPreset(const FName& InPresetName, const FName& InDescription, const FCMCineCameraData& InPresetData) const
{
	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();
	if(PresetMap.Num() == 0){return;}

	for (const auto& Pair : PresetMap)
	{
		if (FCineCameraPresetData* RowData = reinterpret_cast<FCineCameraPresetData*>(Pair.Value))
		{
			if(RowData->PresetName.IsEqual(InPresetName))
			{
				RowData->Description = InDescription;
				RowData->CineCameraData = InPresetData;
				return;
			}
		}
	}
}

void UCameraManagerSubsystem::LoadPresetsTable()
{
	FString ErrorMessage;

	if (UObject* CameraPresetDataObject = CMToolAssetData::CameraPresetDataPath.TryLoad())
	{
		CameraPresetData = Cast<UDataTable>(CameraPresetDataObject);
		if (!CameraPresetData)
		{
			ErrorMessage = TEXT("Failed to cast loaded data to UDataTable!");
		}
	}
	else
	{
		ErrorMessage = TEXT("Failed to load camera presets data!");
	}

	if (!ErrorMessage.IsEmpty())
	{
		UE_LOG(LogCameraManager, Error, TEXT("%s"), *ErrorMessage);
	}
}

void UCameraManagerSubsystem::ToggleCameraManagerToolTab()
{
	FCameraUIManager::ManagerImpInstance->ToggleCameraManagerTab();
}


void UCameraManagerSubsystem::AddCineCameraAsPreset(const FName& InPresetName,const FName& InDescription, const ACineCameraActor* InCineCameraActor, const bool& bIsPresetAlreadyAvailable) const
{
	if(!IsValid(CameraPresetData))
	{
		UE_LOG(LogCameraManager, Error,TEXT("Failed to load camera presets data, skippin adding preset process!"));
		return;
	}
	if(!IsValid(InCineCameraActor))
	{
		UE_LOG(LogCameraManager, Error,TEXT("Camera Actor is null, skippin adding preset process!"));
		return;
	}

	const FCMCineCameraData CreatedPreset = FCMCineCameraData::CreatePreset(InCineCameraActor);

	if(bIsPresetAlreadyAvailable)
	{
		UE_LOG(LogCameraManager, Warning,TEXT("Preset with name %s already exists, overwriting it!"),*InPresetName.ToString());
		OverwriteExistingPreset(InPresetName,InDescription,CreatedPreset);
	}
	else
	{
		UE_LOG(LogCameraManager,Warning,TEXT("Adding new preset with name %s"),*InPresetName.ToString());
		const FName GeneratedRowName =  DataTableUtils::MakeValidName(*InPresetName.ToString());
		
		CameraPresetData->AddRow(GeneratedRowName,FCineCameraPresetData(InPresetName, InDescription, CreatedPreset));
	}

	if(CameraPresetData->MarkPackageDirty())
	{
		CameraPresetData->PostEditChange();

		UEditorLoadingAndSavingUtils::SavePackages({CameraPresetData->GetOutermost()}, true);
	}
}

bool UCameraManagerSubsystem::DeleteAPreset(const FName& InPresetName) const
{
	//Loop through the data table and remove the row with the matching name
	const TMap<FName, uint8*>& PresetMap = CameraPresetData->GetRowMap();

	if(PresetMap.Num() == 0){return false;}
	
	for (const auto& Pair : PresetMap)
	{
		if (FCineCameraPresetData* RowData = reinterpret_cast<FCineCameraPresetData*>(Pair.Value))
		{
			if(RowData->PresetName.IsEqual(InPresetName))
			{
				CameraPresetData->RemoveRow(Pair.Key);

				return true;
			}
		}
	}
	return false;
}

#pragma endregion CameraManager
