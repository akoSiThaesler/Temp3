// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "CMPresetImportHandler.h"
#include "CMCamDataPipelineWindow.h"
#include "CMCameraManagerUtils.h"
#include "CMCameraPresetData.h"
#include "CMLogChannels.h"
#include "CMSubsystem.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"


FPResetImportHandler::FPResetImportHandler()
{
    
}

FPResetImportHandler::FPResetImportHandler(const FOnPresetImportProcessCompleted& InOnPresetImportProcessCompleted)
    : OnPresetImportProcessCompleted(InOnPresetImportProcessCompleted)
{
}

void FPResetImportHandler::StartImportProcess()
{
    if(!IsValid(GEditor)){return;}
    
    const UCameraManagerSubsystem* CameraManagerSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>();
    if(!IsValid(CameraManagerSubsystem)){return;}
    
    FString OutFilePath;
    if (FCMFileUtils::OpenImportPresetDialog(OutFilePath))
    {
        TempTable = FCMFileUtils::CreateTempDataTableWithJsonFile(FCineCameraPresetData::StaticStruct(), OutFilePath);

        if (!TempTable)
        {
            UE_LOG(LogCameraManager, Error, TEXT("Failed to create temporary data table from JSON file."));
            RequestShutdown(ECMImportFailReason::Type::UnknownError);
            return;
        }

        TArray<FPresetImportData> PresetsWithDesc;
        for (const TPair<FName, unsigned char*>& CurrentRow : TempTable->GetRowMap())
        {
            FCineCameraPresetData* RowDataStruct = reinterpret_cast<FCineCameraPresetData*>(CurrentRow.Value);

            bool bIsPresetAlreadyAvailable = CameraManagerSubsystem->IsPresetAvailable(RowDataStruct->PresetName);
            
            PresetsWithDesc.Emplace(FPresetImportData(bIsPresetAlreadyAvailable, TPair<FName, FName>(RowDataStruct->PresetName, RowDataStruct->Description))); //Or MakeTuple(RowDataStruct->PresetName, RowDataStruct->Description)
        }

        if (PresetsWithDesc.IsEmpty())
        {
            UE_LOG(LogCameraManager, Error, TEXT("No rows found in the data table 2."));
            RequestShutdown(ECMImportFailReason::Type::NoValidData);
            return;
        }

        SpawnImportWindow(PresetsWithDesc);
    }
    else
    {
        RequestShutdown(ECMImportFailReason::Type::UserCancelled);
    }
}

void FPResetImportHandler::RequestShutdown(ECMImportFailReason::Type InShutdownReason)
{
    if (PresetImportWindow.IsValid())
    {
        PresetImportWindow->RequestDestroyWindow();
        PresetImportWindow.Reset();
    }

    if(IsValid(TempTable))
    {
        FCMFileUtils::ClearTempDataTable(TempTable);
        TempTable = nullptr;
    }

    if(OnPresetImportProcessCompleted.IsBound())
    {
        OnPresetImportProcessCompleted.Execute(InShutdownReason);
    }
}



TArray<FName> FPResetImportHandler::SpawnImportWindow(const TArray<FPresetImportData>& InPresets)
{
    FSlateApplication& SlateApp = FSlateApplication::Get();
    const FVector2D WindowSize = FVector2D(500.f, 350.f);

    const TSharedRef<SCMPresetPipelineImportWindow> ImportWindow = SNew(SCMPresetPipelineImportWindow)
        .Title(FText::FromName("Import Presets"))
        .Presets(InPresets)
        .OnPresetImportRequested_Raw(this, &FPResetImportHandler::OnPresetImportRequested);

    ImportWindow->MoveWindowTo(SlateApp.GetCursorPos() + FVector2D(-WindowSize.X / 2, -WindowSize.Y / 2));

    PresetImportWindow = SlateApp.AddWindow(ImportWindow);
    if (PresetImportWindow.IsValid())
    {
        PresetImportWindow->Resize(FVector2D(WindowSize));
    }

    return {};
}

void FPResetImportHandler::OnPresetImportRequested(const TArray<FName>& InSelectedPresetNames)
{
    if (!IsValid(TempTable))
    {
        UE_LOG(LogCameraManager, Error, TEXT("TempTable is not valid."));
        RequestShutdown(ECMImportFailReason::Type::UnknownError);
        return;
    }

    if (InSelectedPresetNames.IsEmpty())
    {
        RequestShutdown(ECMImportFailReason::Type::UserCancelled);
        return;
    }

    // Get related rows from table
    TArray<FCineCameraPresetData> ImportedPresets;
    TempTable->ForeachRow<FCineCameraPresetData>(TEXT(""), [&ImportedPresets, &InSelectedPresetNames](const FName& Key, const FCineCameraPresetData& Value)
    {
        //Extract selected presets from Temp Table
        if (InSelectedPresetNames.Contains(Value.PresetName))
        {
            ImportedPresets.Emplace(Value);
        }
    });

    if (ImportedPresets.IsEmpty())
    {
        UE_LOG(LogCameraManager, Error, TEXT("No rows found in the data table."));
        RequestShutdown(ECMImportFailReason::Type::NoValidData);
        return;
    }
    
    if (const UCameraManagerSubsystem* CameraManagerSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
    {
        CameraManagerSubsystem->ImportPresets(ImportedPresets);
    }
    else
    {
        UE_LOG(LogCameraManager, Error, TEXT("CameraManagerSubsystem is not valid."));
        RequestShutdown(ECMImportFailReason::Type::UnknownError);
        return;
    }
    
    RequestShutdown(ECMImportFailReason::Type::None);
}

