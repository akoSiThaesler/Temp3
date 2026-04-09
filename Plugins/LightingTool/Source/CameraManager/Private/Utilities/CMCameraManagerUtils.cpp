// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMCameraManagerUtils.h"
#include "CineCameraActor.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "EditorDirectories.h"
#include "IDesktopPlatform.h"
#include "Selection.h"
#include "Editor/EditorEngine.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "SLevelViewport.h"
#include <cctype>  
#include <cstdlib>

#include "EngineUtils.h"
#include "CMLogChannels.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"

bool FCMCameraUtils::RemovePresetNameFromCineCamera(ACineCameraActor* InCineCameraActor)
{
	TArray<FName>& ActorTags = InCineCameraActor->Tags;

	for(int32 Index = 0; Index < ActorTags.Num(); Index++)
	{
		if(ActorTags[Index].ToString().Contains(CM_PRESET_TAG_TITLE))
		{
			ActorTags.RemoveAt(Index);
			break;
		}
	}
	return true;
}

bool FCMCameraUtils::DoesContainsThePreset(const FName& InPresetName, ACineCameraActor* InCineCameraActor)
{
	for(int32 Index = InCineCameraActor->Tags.Num() - 1; Index >= 0; Index--)
	{
		if(InCineCameraActor->Tags[Index].ToString().Equals(CM_PRESET_TAG_TITLE+InPresetName.ToString()))
		{
			return true;
		}
	}
	return false;
}


void FCMCameraUtils::InsertNewPresetNameToCineCamera(ACineCameraActor* InCameraActor, const FName& InNewTag)
{
	for(int32 Index = InCameraActor->Tags.Num() - 1; Index >= 0; Index--)
	{
		if(InCameraActor->Tags[Index].ToString().Contains(CM_PRESET_TAG_TITLE))
		{
			InCameraActor->Tags.RemoveAt(Index);
		}
	}

	const FString TagToAdd = FString(CM_PRESET_TAG_TITLE) + InNewTag.ToString();
	InCameraActor->Tags.Add(*TagToAdd);
}

FName FCMCameraUtils::GetPresetNameFromCineCamera(const TArray<FName>& InPresetNames, ACineCameraActor* InCineCameraActor)
{
	 TArray<FName>& ActorTags = InCineCameraActor->Tags;

	if(ActorTags.IsEmpty()){return NAME_None;}

	for(int32 Index = ActorTags.Num() - 1; Index >= 0; Index--)
	{
		FString Tag = ActorTags[Index].ToString();

		if(Tag.Contains(CM_PRESET_TAG_TITLE))
		{
			Tag.RemoveFromStart(CM_PRESET_TAG_TITLE);

			if(InPresetNames.Contains(*Tag))
			{
				return *Tag;
			}
			ActorTags.RemoveAt(Index);
			
			return NAME_None;
		}
	}
	return NAME_None;
}

TArray<ACineCameraActor*> FCMCameraUtils::FilterCineCameraActors(const TArray<UObject*>& InActors)
{
	TArray<ACineCameraActor*> CineCameraActors;

	for (UObject* Actor : InActors)
	{
		if (ACineCameraActor* CineCameraActor = Cast<ACineCameraActor>(Actor))
		{
			CineCameraActors.Add(CineCameraActor);
		}
	}

	return CineCameraActors;
}

TArray<ACineCameraActor*> FCMCameraUtils::GetSelectedCineCameraActors()
{
	TArray<ACineCameraActor*> SelectedCineCameraActors;
	
	// Check if the editor pointer is valid
	if(UEditorEngine* EditorEngine = GEditor)
	{
		// Get the list of selected actors
		USelection* SelectedActors = EditorEngine->GetSelectedActors();

		// Iterate over the selected actors
		for(FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			// Cast the selected actor to ACineCameraActor

			// Check if the cast was successful
			if(ACineCameraActor* SelectedCineCameraActor = Cast<ACineCameraActor>(*Iter))
			{
				// Add the selected CineCameraActor to the array
				SelectedCineCameraActors.Add(SelectedCineCameraActor);
			}
		}
	}
	return SelectedCineCameraActors;
}

TArray<AActor*> FCMCameraUtils::GetPilotedCameraActors()
{
	//TODO When clicking stop pilot button manually detect that the camera is no longer piloted
	//GetMutableDefault<ULevelEditorViewportSettings>()->OnSettingChanged().AddRaw(this, &SLevelViewport::HandleViewportSettingChanged);
	
	TArray<AActor*> LockedCameras;
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<ILevelEditor> LevelEditor = LevelEditorModule.GetFirstLevelEditor();
	if (LevelEditor.IsValid())
	{
		for (TSharedPtr<SLevelViewport>& ViewportWindow : LevelEditor->GetViewports())
		{
			if(ViewportWindow->GetLevelViewportClient().IsAnyActorLocked())
			{
				LockedCameras.Emplace(ViewportWindow->GetLevelViewportClient().GetActiveActorLock().Get());
			}
		}
	}
	
	return LockedCameras;
}

TArray<ACineCameraActor*> FCMCameraUtils::GetAllCineCameraActors()
{
	if(!IsValid(GEditor)){return {};}
	
	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!IsValid(EditorWorld))
	{
		UE_LOG(LogCameraManager, Error, TEXT("EditorWorld not found, unable to refresh camera infos!"));
		return {};
	}

	TArray<ACineCameraActor*> FoundCameraActors;
	
	for (TActorIterator<ACineCameraActor> CameraIt(EditorWorld); CameraIt; ++CameraIt)
	{
		FoundCameraActors.Add(*CameraIt);
	}
	return FoundCameraActors;
}


bool FCMCameraUtils::AlphanumericCompare(const FString& A, const FString& B)
{
	const TCHAR* a = *A;
	const TCHAR* b = *B;

	while (*a && *b)
	{
		if (std::isdigit(*a) && std::isdigit(*b))
		{
			// Compare numbers
			TCHAR* end_a;
			TCHAR* end_b;
			long num_a = std::wcstol(a, &end_a, 10);
			long num_b = std::wcstol(b, &end_b, 10);

			if (num_a != num_b)
				return num_a < num_b;

			a = end_a;
			b = end_b;
		}
		else
		{
			// Compare characters (case-insensitive)
			TCHAR char_a = std::tolower(*a);
			TCHAR char_b = std::tolower(*b);

			if (char_a != char_b)
				return char_a < char_b;

			++a;
			++b;
		}
	}

	// If one string is a prefix of the other, the shorter string is considered less
	return *a == 0 && *b != 0;
}

ACineCameraActor* FCMCameraUtils::CreateCineCameraOnView()
{
	// Find the perspective viewport we were using
	FViewport* pViewPort = GEditor->GetActiveViewport();
	FLevelEditorViewportClient* ViewportClient = nullptr;
	for( FLevelEditorViewportClient* LevelViewport : GEditor->GetLevelViewportClients())
	{		
		if( LevelViewport->IsPerspective() && LevelViewport->Viewport == pViewPort)
		{
			ViewportClient = LevelViewport;
			break;
		}
	}

	if( ViewportClient == nullptr ){return nullptr;}

	const FScopedTransaction Transaction(NSLOCTEXT("LevelViewport", "CreateCameraHere", "Create Camera Here"));

	// Set new camera to match viewport
	ACineCameraActor* pNewCamera = Cast<ACineCameraActor>(ViewportClient->GetWorld()->SpawnActor(ACineCameraActor::StaticClass()));
	pNewCamera->SetActorLocation( ViewportClient->GetViewLocation(), false);
	pNewCamera->SetActorRotation( ViewportClient->GetViewRotation());
	pNewCamera->GetCameraComponent()->SetFieldOfView( ViewportClient->ViewFOV);

	// Deselect any currently selected actors
	GUnrealEd->SelectNone( false, true);
	GEditor->GetSelectedActors()->DeselectAll();
	GEditor->GetSelectedObjects()->DeselectAll();

	// Select newly created Camera
	GEditor->SelectActor( pNewCamera, true, true);

	// Send notification about actors that may have changed
	ULevel::LevelDirtiedEvent.Broadcast();

	// Redraw viewports to show new camera
	GEditor->RedrawAllViewports();
	
	return pNewCamera;
}


bool FCMPresetUtils::IsValidNameConvention(const FText& InText)
{
	FString nameStr = InText.ToString();

	if(nameStr.IsEmpty()){return true;}

	// Check if the name starts with a letter
	if (!FChar::IsAlpha(nameStr[0]))
	{
		return false;
	}

	// Check if the name only contains letters, numbers, spaces, or underscores
	for (TCHAR c : nameStr)
	{
		if (!FChar::IsAlnum(c) && c != '_' && c != ' ')
		{
			return false;
		}
	}
	return true;
}

bool FCMPresetUtils::IsValidNameConventionFromName(const FName& InName)
{
	FString nameStr = InName.ToString();

	if(nameStr.IsEmpty()){return true;}

	// Check if the name starts with a letter
	if (!FChar::IsAlpha(nameStr[0]))
	{
		return false;
	}

	// Check if the name only contains letters, numbers, spaces, or underscores
	for (TCHAR c : nameStr)
	{
		if (!FChar::IsAlnum(c) && c != '_' && c != ' ')
		{
			return false;
		}
	}
	return true;
}

FName FCMPresetUtils::CleanPresetName(const FName& Name)
{
	FString NameStr = Name.ToString();
	NameStr.TrimEndInline();
	return FName(*NameStr);
}

bool FCMFileUtils::OpenImportPresetDialog(FString& OutFilePath)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		const FString Title = TEXT("Please choose a preset json file to load...");
		const FString DefaultPath = GetLastDirectory();
		const FString FileTypes = TEXT("JSON Files (*.json)|*.json");

		TArray<FString> OutFilenames;

		if(DesktopPlatform->OpenFileDialog(ParentWindowHandle,Title,DefaultPath,TEXT(""),FileTypes,EFileDialogFlags::None,OutFilenames))
		{
			if(!OutFilenames.IsEmpty() && FPaths::FileExists(OutFilenames[0]))
			{
				SetLastDirectory(FPaths::GetPath(OutFilenames[0]));
				OutFilePath = OutFilenames[0];
				return true;
			}
			UE_LOG(LogCameraManager, Error, TEXT("The file does not exist or is empty."));
			return false;
		}
		
		// User canceled the dialog
		return false;
	}
	UE_LOG(LogCameraManager, Error, TEXT("Failed to get DesktopPlatformModule."));
	return false;
}

UDataTable* FCMFileUtils::CreateTempDataTableWithJsonFile(UScriptStruct* InParentTable, const FString& InFilePath)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *InFilePath))
	{
		UE_LOG(LogCameraManager, Error, TEXT("Failed to load JSON file."));
		return nullptr;
	}
		
	UDataTable* NewDataTable = NewObject<UDataTable>(GetTransientPackage(),FName("TempCameraPresetTable"), RF_Public | RF_Standalone);
	
	NewDataTable->RowStruct = InParentTable;
	NewDataTable->EmptyTable();

	auto Problems = NewDataTable->CreateTableFromJSONString(JsonString);

	if(!Problems.IsEmpty())
	{
		for(auto CurrentProblem : Problems)
		{
			UE_LOG(LogCameraManager, Error, TEXT("Failed to import presets: %s"), *CurrentProblem);	
		}
		return nullptr;
	}
	return NewDataTable;
}

void FCMFileUtils::ClearTempDataTable(UDataTable* InDataTable)
{
	if(IsValid(InDataTable))
	{
		// Remove the RF_Standalone flag
		InDataTable->ClearFlags(RF_Standalone);

		// Mark the object for deletion
		InDataTable->MarkAsGarbage();
	}
}

bool FCMFileUtils::OpenExportPresetDialog(const FString& InFilePath, FString& OutFilePath)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	
	const FText Title = FText::FromName(TEXT("Please choose a directory to save the camera preset..."));
	
	const FString FileTypes = TEXT("JSON (*.json)|*.json");

	TArray<FString> OutFilenames;
	DesktopPlatform->SaveFileDialog(
		ParentWindowWindowHandle,
		Title.ToString(),
		(InFilePath.IsEmpty()) ? GetLastDirectory() : FPaths::GetPath(InFilePath),
		(InFilePath.IsEmpty()) ? TEXT("CameraPresets.json") : FPaths::GetBaseFilename(InFilePath) + TEXT(".json"),
		FileTypes,
		EFileDialogFlags::None,
		OutFilenames
		);

	if (OutFilenames.Num() > 0)
	{
		OutFilePath = OutFilenames[0];
		return true;
	}
	return false;
}

bool FCMFileUtils::OpenDirectoryPicker(FString& OutDirectory, const FString& DefaultPath)
{
	bool bFolderSelected = false;
	if ( IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get() )
	{
		void* TopWindowHandle = FSlateApplication::Get().GetActiveTopLevelWindow().IsValid() ? FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle() : nullptr;

		FString FolderName;
		bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			TopWindowHandle,
			FText::FromString(TEXT("Please choose a directory to save the camera preset...")).ToString(),
			DefaultPath,
			FolderName
		);

		if ( bFolderSelected )
		{
			OutDirectory = FolderName;
		}
	}

	return bFolderSelected;
}

FString FCMFileUtils::GetLastDirectory()
{
	return FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_EXPORT);
	
}

void FCMFileUtils::SetLastDirectory(const FString& InLastDirectory)
{
	FEditorDirectories::Get().SetLastDirectory(ELastDirectory::GENERIC_EXPORT, InLastDirectory);
}

FString FCMFileUtils::GetDefaultExportPath()
{
	return FPaths::Combine(GetLastDirectory(),TEXT("CameraPresets.json"));
}

bool FCMFileUtils::IsValidPathForSaving(const FString& Path)
{
	// Check if the path is not empty
	if (Path.IsEmpty())
	{
		UE_LOG(LogCameraManager, Warning, TEXT("The path is empty."));
		return false;
	}

	// Check if the path is valid
	if (!FPaths::ValidatePath(Path))
	{
		UE_LOG(LogCameraManager, Warning, TEXT("Invalid path: %s"), *Path);
		return false;
	}

	// Extract the directory from the path
	FString Directory = FPaths::GetPath(Path);

	// Check if the directory exists
	if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*Directory))
	{
		UE_LOG(LogCameraManager, Warning, TEXT("Directory does not exist: %s"), *Directory);
		return false;
	}

	// Check if the path ends with the expected file extension
	if (!Path.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogCameraManager, Warning, TEXT("The path does not end with the expected extension:json"));
		return false;
	}

	// Extract the filename from the path
	FString Filename = FPaths::GetCleanFilename(Path);

	// Check if the filename is not empty
	if (Filename.IsEmpty())
	{
		UE_LOG(LogCameraManager, Warning, TEXT("The filename is empty."));
		return false;
	}

	// Additional custom checks can be added here

	return true;
}