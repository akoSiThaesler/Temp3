// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LTFunctions.h"
#include "CineCameraActor.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ImageUtils.h"
#include "ISettingsEditorModule.h"
#include "LevelEditorActions.h"
#include "LevelEditorViewport.h"
#include "LTLightingLogChannels.h"
#include "MTDebug.h"
#include "ObjectTools.h"
#include "StaticMeshResources.h"
#include "UnrealEdGlobals.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Blueprint.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/TextureLightProfile.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/KismetMathLibrary.h"

constexpr const char* LIGHTING_TOOL_PATH_TRACING_SETTINGS = "PathTracingSettings";

constexpr const char* LIGHTING_TOOL_HDRI_BACKDROP_NAME = "LightingToolUniqueHDRIBackdrop";

/** Interpolate a linear alpha value using an ease mode and function. */
template<typename FloatType UE_REQUIRES(TIsFloatingPoint<FloatType>::Value)>

FloatType EaseAlpha(FloatType InAlpha, uint8 EasingFunc, FloatType BlendExp, int32 Steps)
{
	switch (EasingFunc)
	{
	case EEasingFunc::Step:					return FMath::InterpStep<FloatType>(0.f, 1.f, InAlpha, Steps);
	case EEasingFunc::SinusoidalIn:			return FMath::InterpSinIn<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::SinusoidalOut:		return FMath::InterpSinOut<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::SinusoidalInOut:		return FMath::InterpSinInOut<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::EaseIn:				return FMath::InterpEaseIn<FloatType>(0.f, 1.f, InAlpha, BlendExp);
	case EEasingFunc::EaseOut:				return FMath::InterpEaseOut<FloatType>(0.f, 1.f, InAlpha, BlendExp);
	case EEasingFunc::EaseInOut:			return FMath::InterpEaseInOut<FloatType>(0.f, 1.f, InAlpha, BlendExp);
	case EEasingFunc::ExpoIn:				return FMath::InterpExpoIn<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::ExpoOut:				return FMath::InterpExpoOut<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::ExpoInOut:			return FMath::InterpExpoInOut<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::CircularIn:			return FMath::InterpCircularIn<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::CircularOut:			return FMath::InterpCircularOut<FloatType>(0.f, 1.f, InAlpha);
	case EEasingFunc::CircularInOut:		return FMath::InterpCircularInOut<FloatType>(0.f, 1.f, InAlpha);
	default: ;
	}
	return InAlpha;
}

#pragma region LightmapResolution

float CalculateSurfaceAreaForSMComponent(const UStaticMeshComponent* StaticMeshComponent)
{
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();

	if (!StaticMesh || StaticMesh->GetRenderData()->LODResources.Num() <= 0)
	{
		UE_LOG(LogLighting,Error,TEXT("No LOD or Invalid Static Mesh."));
		return 0.0f;
	}
	
	const FVector& Scale = StaticMeshComponent->GetComponentScale();
	FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[0];
	float SurfaceArea = 0.0f;

	for (int32 i = 0; i < LOD.IndexBuffer.GetNumIndices(); i += 3)
	{
		FVector Vertex1 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i))) * Scale;
		FVector Vertex2 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i + 1))) * Scale;
		FVector Vertex3 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i + 2))) * Scale;

		// Calculate triangle area using the cross product method
		FVector Edge1 = Vertex2 - Vertex1;
		FVector Edge2 = Vertex3 - Vertex1;
		FVector CrossProduct = FVector::CrossProduct(Edge1, Edge2);
		SurfaceArea += CrossProduct.Size() * 0.5f;
	}

	return SurfaceArea;
}

float CalculateSurfaceAreaForSM(const UStaticMesh* StaticMesh)
{
	if (!StaticMesh || StaticMesh->GetRenderData()->LODResources.Num() <= 0)
	{
		UE_LOG(LogLighting,Error,TEXT("No LOD or Invalid Static Mesh."));
		return 0.0f;
	}
	
	const FStaticMeshLODResources& LOD = StaticMesh->GetRenderData()->LODResources[0];
	float SurfaceArea = 0.0f;

	for (int32 i = 0; i < LOD.IndexBuffer.GetNumIndices(); i += 3)
	{
		FVector Vertex1 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i)));
		FVector Vertex2 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i + 1)));
		FVector Vertex3 = FVector(LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(LOD.IndexBuffer.GetIndex(i + 2)));

		// Calculate triangle area using the cross product method
		FVector Edge1 = Vertex2 - Vertex1;
		FVector Edge2 = Vertex3 - Vertex1;
		FVector CrossProduct = FVector::CrossProduct(Edge1, Edge2);
		SurfaceArea += CrossProduct.Size() * 0.5f;
	}

	return SurfaceArea;
}

int32 ULTFunctions::CalculateDesiredLightmapResolution(const UStaticMeshComponent* StaticMeshComponent, const float& DesiredDensity,const int32& MaxAllowedRes)
{
	const float SurfaceArea = CalculateSurfaceAreaForSMComponent(StaticMeshComponent);

	if (SurfaceArea <= 0)
	{
		UE_LOG(LogLighting,Error,TEXT("Invalid SurfaceArea value."));
		return 0;
	}
	
	int32 RequiredPixels = FMath::Sqrt(FMath::Abs(SurfaceArea * DesiredDensity));

	// Adjust to be a multiple of 4 if not already
	if (RequiredPixels % 4 != 0)
	{
		RequiredPixels += 4 - (RequiredPixels % 4);
	}

	// Clamping the resolution between the minimum and maximum values
	RequiredPixels = FMath::Clamp(RequiredPixels, 4, MaxAllowedRes);

	return RequiredPixels;
}


int32 ULTFunctions::CalculateDesiredLightmapResolutionForStaticMesh(const UStaticMesh* StaticMesh,const float& DesiredDensity,const int32& MaxAllowedRes)
{
	const float SurfaceArea = CalculateSurfaceAreaForSM(StaticMesh);

	if (SurfaceArea <= 0)
	{
		UE_LOG(LogLighting,Error,TEXT("Invalid SurfaceArea value."));
		return 0;
	}
	
	int32 RequiredPixels = FMath::Sqrt(FMath::Abs(SurfaceArea * DesiredDensity));

	// Adjust to be a multiple of 4 if not already
	if (RequiredPixels % 4 != 0)
	{
		RequiredPixels += 4 - (RequiredPixels % 4);
	}

	// Clamping the resolution between the minimum and maximum values
	RequiredPixels = FMath::Clamp(RequiredPixels, 4, MaxAllowedRes);
	return RequiredPixels;
}

void ULTFunctions::CalculateDensityAndResolution(const FAssetData& InAssetData,float& OutDensity,int32& OutResolution,int32& OutCoordIndex)
{
	const auto FoundAsset = InAssetData.GetAsset();
	if(!FoundAsset){return;}
	
	const UStaticMesh* StaticMesh = Cast<UStaticMesh>(FoundAsset);
	if(!StaticMesh){return;}
	
	const float SurfaceArea = CalculateSurfaceAreaForSM(StaticMesh);
	
	if (SurfaceArea <= 0)
	{
		UE_LOG(LogLighting,Error,TEXT("Invalid SurfaceArea value."));
		return;
	}

	// Calculate the number of lightmap pixels
	const int32 LightmapPixels = StaticMesh->GetLightMapResolution() * StaticMesh->GetLightMapResolution();

	// Calculate the current lightmap density
	const float CurrentDensity = LightmapPixels / SurfaceArea;

	OutResolution = StaticMesh->GetLightMapResolution();
	OutCoordIndex = StaticMesh->GetLightMapCoordinateIndex();
	OutDensity =  CurrentDensity;
}

FLinearColor ULTFunctions::GetColorOfTheDensity(const float& InDensity)
{
	float Density = InDensity;
	if ( Density > GEditor->IdealLightMapDensity)
	{
		const float Range = GEditor->MaxLightMapDensity - GEditor->IdealLightMapDensity;
		Density -= GEditor->IdealLightMapDensity;
		return FLinearColor( FVector4( Density/Range, (Range-Density)/Range, 0.0f, 1.0f));
	}
	else
	{
		const float Range = GEditor->IdealLightMapDensity - GEngine->MinLightMapDensity;
		Density -= GEngine->MinLightMapDensity;
		return FLinearColor( FVector4( 0.0f, Density/Range, (Range-Density)/Range, 1.0f));
	}
}

void ULTFunctions::SetIdealLightmapDensity(const float& NewIdealDensity)
{
	FLevelEditorActionCallbacks::SetLightingDensityIdeal(NewIdealDensity);
}

const float& ULTFunctions::GetIdealLightmapDensity()
{
	return GEditor->IdealLightMapDensity;
}

void ULTFunctions::SetMaxLightmapDensity(const float& NewMaxDensity)
{
	FLevelEditorActionCallbacks::SetLightingDensityMaximum(NewMaxDensity);
}

const float& ULTFunctions::GetMaxLightmapDensity()
{
	return GEditor->MaxLightMapDensity;
}

bool ULTFunctions::IsThereAnySelectedSMAsset()
{
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	if(SelectedAssets.IsEmpty())
	{
		return false;
	}
	
	for(auto CurrentAsset : SelectedAssets)
	{
		if(CurrentAsset.GetClass()->GetName().Equals("StaticMesh"))
		{
			return true;
		}
	}
	return false;
}

void ULTFunctions::ChangeViewMode()
{
	if(!GEditor){return;}
	if(UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEditor))
	{
		if(const auto ViewportClient = static_cast<FEditorViewportClient*>(EditorEngine->GetActiveViewport()->GetClient()))
		{
			if(ViewportClient->GetViewMode() != VMI_LightmapDensity)
			{
				ViewportClient->SetViewMode(VMI_LightmapDensity);
			}
			else
			{
				ViewportClient->SetViewMode(VMI_Lit);
			}
		}
	}
}

EViewModeIndex ULTFunctions::GetViewMode()
{
	if(!GEditor){return VMI_Lit;}
	if(UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEditor))
	{
		if (EditorEngine->GetActiveViewport())
		{
			if (EditorEngine->GetActiveViewport()->GetClient())
			{
				if(const auto ViewportClient = static_cast<FEditorViewportClient*>(EditorEngine->GetActiveViewport()->GetClient()))
				{
					return ViewportClient->GetViewMode();
				}
			}
		}
	}
	return VMI_Lit;
}

UTexture2D* ULTFunctions::GenerateThumbnailForAsset(const FAssetData& InAssetPath)
{
	FString PackageFilename;
	const FName ObjectFullName = FName(*InAssetPath.GetFullName());
	TSet<FName> ObjectFullNames;
	ObjectFullNames.Add(ObjectFullName);

	if (FPackageName::DoesPackageExist(InAssetPath.PackageName.ToString(), &PackageFilename))
	{
		FThumbnailMap ThumbnailMap;
		IImageWrapperModule& ImageWrapperModule = FModuleManager::Get().LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

		ThumbnailTools::LoadThumbnailsFromPackage(PackageFilename, ObjectFullNames,ThumbnailMap);

		if(const FObjectThumbnail* AssetTN = ThumbnailMap.Find(ObjectFullName))
		{
			const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
			if(ImageWrapper.Get())
			{
				ImageWrapper->SetRaw(AssetTN->GetUncompressedImageData().GetData(), AssetTN->GetUncompressedImageData().Num(),
				AssetTN->GetImageWidth(), AssetTN->GetImageHeight(), ERGBFormat::BGRA, 8);
				const TArray64<uint8>& CompressedByteArray = ImageWrapper->GetCompressed();

				if(UTexture2D* CreatedTexture = FImageUtils::ImportBufferAsTexture2D(CompressedByteArray))
				{
					return CreatedTexture;
				}
			}
		}
		else
		{
			if(const auto GeneratedObjectThumbnail = ThumbnailTools::GenerateThumbnailForObjectToSaveToDisk(InAssetPath.GetAsset()))
			{
				const TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
				if(ImageWrapper.Get())
				{
					ImageWrapper->SetRaw(GeneratedObjectThumbnail->GetUncompressedImageData().GetData(), GeneratedObjectThumbnail->GetUncompressedImageData().Num(),
					GeneratedObjectThumbnail->GetImageWidth(), GeneratedObjectThumbnail->GetImageHeight(), ERGBFormat::BGRA, 8);
					const TArray64<uint8>& CompressedByteArray = ImageWrapper->GetCompressed();

					if(UTexture2D* CreatedTexture = FImageUtils::ImportBufferAsTexture2D(CompressedByteArray))
					{
						return  CreatedTexture;
					}
				}	
			}
		}
	}
	const FSoftObjectPath PlaceholderTexturePath("/Engine/EditorResources/S_ReflActorIcon");
	if(UObject* ToolWindowObject = PlaceholderTexturePath.TryLoad())
	{
		if(const auto PlaceHolderTexture = Cast<UTexture2D>(ToolWindowObject))
		{
			return PlaceHolderTexture;
		}
	}
	return nullptr;
}

bool IsInEditorAndNotPlaying()
{
	if (!IsInGameThread())
	{
		UE_LOG(LogLighting, Error, TEXT("Not on the main thread, terminating the task.."));
		return false;
	}
	if (!GIsEditor)
	{
		UE_LOG(LogLighting, Error, TEXT("Not in the Editor."));
		return false;
	}
	if (GEditor->PlayWorld || GIsPlayInEditorWorld)
	{
		UE_LOG(LogLighting, Error, TEXT("The Editor is currently in a play mode."));
		return false;
	}
	return true;
	
}

bool IsAssetRegistryModuleLoading()
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	if (AssetRegistryModule.Get().IsLoadingAssets())
	{
		UE_LOG(LogLighting, Error, TEXT("The AssetRegistry is currently loading."));
		return false;
	}
	return true;
}

bool ULTFunctions::SaveSelectedAssets()
{
	if(!IsInEditorAndNotPlaying() || !IsAssetRegistryModuleLoading()){return false;}
	
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	TArray<UPackage*> Packages;
	for(auto& CurrentAsset : SelectedAssets)
	{
		if(CurrentAsset.GetClass()->GetName().Equals(TEXT("StaticMesh")))
		{
			Packages.Add(CurrentAsset.GetPackage());
		}
	}
	
	// Save without a prompt
	return UEditorLoadingAndSavingUtils::SavePackages(Packages, true);
}

void ULTFunctions::ShowNotifyToUser(const FString InMessage)
{
	LTDebug::ShowNotifyInfo(InMessage);
}

int32 ULTFunctions::ChangeLightmapCoordinateIndex(const FName& InObjectPath, const int32 InNewIndex)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if(const auto FoundAsset = AssetRegistry.GetAssetByObjectPath(InObjectPath.ToString()).GetAsset())
	{
		if(const auto FoundSM = Cast<UStaticMesh>(FoundAsset))
		{
			FoundSM->SetLightMapCoordinateIndex(InNewIndex);
			FoundSM->PostEditChange();
			FoundSM->MarkPackageDirty();
			return FoundSM->GetLightMapCoordinateIndex();
		}
	}
	return -1;
}

int32 ULTFunctions::GetLightmapCoordinateIndex(const FName& InObjectPath)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if(const auto FoundAsset = AssetRegistry.GetAssetByObjectPath(InObjectPath.ToString()).GetAsset())
	{
		if(const auto FoundSM = Cast<UStaticMesh>(FoundAsset))
		{
			return FoundSM->GetLightMapCoordinateIndex();
		}
	}
	return 0;
}

void ULTFunctions::ChangeLightmapResolution(const FName& InObjectPath, const int32 InNewResolution)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if(const auto FoundAsset = AssetRegistry.GetAssetByObjectPath(InObjectPath.ToString()).GetAsset())
	{
		if(const auto FoundSM = Cast<UStaticMesh>(FoundAsset))
		{
			FoundSM->SetLightMapResolution(InNewResolution);
			FoundSM->PostEditChange();
			FoundSM->MarkPackageDirty();
		}
	}
}

APostProcessVolume* ULTFunctions::SpawnOrGetExistingPostProcessVolume()
{
	if (UWorld* World = GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<APostProcessVolume> ActorItr(World); ActorItr; ++ActorItr) {

			APostProcessVolume* PostProcessActor = *ActorItr;

			if(PostProcessActor->Tags.Contains(LIGHTING_TOOL_PATH_TRACING_SETTINGS))
			{
				return PostProcessActor;
			}
		}
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.bDeferConstruction = false;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		if (const auto PPVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform(), SpawnParams))
		{
			PPVolume->SetActorLabel(LIGHTING_TOOL_PATH_TRACING_SETTINGS);
			PPVolume->Tags.Add(LIGHTING_TOOL_PATH_TRACING_SETTINGS);
			
			PPVolume->bEnabled = true;
			PPVolume->bUnbound = true;
			// You can also modify PostProcessVolume->Settings to modify PP settings

			return PPVolume;
		}
	}
	return nullptr;
}

TArray<FName> ULTFunctions::CreateEasingFuncNameArray()
{
	const UEnum* EasingEnum = StaticEnum<EEasingFunc::Type>();

	TArray<FName> EnumNames;
	if (!EasingEnum)
	{
		return EnumNames; // Return an empty array if the enum is null
	}
	
	for (int32 i = 0; i < EasingEnum->NumEnums() - 1; ++i) // -1 to avoid Max enum
	{
		// Get the name of the enum value at index i
		FName EnumName = EasingEnum->GetNameByIndex(i);
		EnumNames.Add(*EnumName.ToString().Replace(TEXT("EEasingFunc::"),TEXT("")));
	}

	return EnumNames;
}

EEasingFunc::Type ULTFunctions::GetEasingFuncValueFromName(const FName& Name)
{
	const UEnum* EasingEnum = StaticEnum<EEasingFunc::Type>();
	if (!EasingEnum)
	{
		return EEasingFunc::Type::Linear; // Default to Linear in case of error
	}
	
	FName EnumName = Name;

	// Find the index of the enum by name, then return the enum value at that index
	const int32 Index = EasingEnum->GetIndexByName(EnumName, EGetByNameFlags::None);
	if (Index == INDEX_NONE)
	{
		return EEasingFunc::Type::Linear; // Default to Linear if name not found
	}

	return static_cast<EEasingFunc::Type>(EasingEnum->GetValueByIndex(Index));
}

EEasingFunc::Type ULTFunctions::GetEasingFuncValueFromIndex(const int32 Index)
{
	const UEnum* EasingEnum = StaticEnum<EEasingFunc::Type>();
	if (!EasingEnum)
	{
		UE_LOG(LogTemp, Warning, TEXT("EasingEnum is null. Make sure EEasingFunc::Type is a valid UENUM."));
		return EEasingFunc::Type::Linear; // Default to Linear in case of error
	}

	if (Index < 0 || Index >= EasingEnum->NumEnums() - 1) // Checking index validity, excluding the Max enum
	{
		UE_LOG(LogTemp, Warning, TEXT("Index %d is out of range for EEasingFunc::Type. Returning Linear as default."), Index);
		return EEasingFunc::Type::Linear; // Default to Linear in case of invalid index
	}

	// Retrieve and return the enum value at the given index
	return static_cast<EEasingFunc::Type>(EasingEnum->GetValueByIndex(Index));
}

float Lerp(float A, float B, float V)
{
	return A + V*(B-A);
}	

float ULTFunctions::AdjustDensityWithEase(float A, float B, float Alpha, TEnumAsByte<EEasingFunc::Type> EasingFunc)
{
	return Lerp(A, B, EaseAlpha(Alpha, EasingFunc, 2.0f, 2.0f));
}

#pragma endregion LightmapResolution

#pragma region ProjectSettings


void ULTFunctions::UpdateConfigAPropertyFromRendererSettings(URendererSettings* const RendererSettings, FName PropertyName)
{
	check(RendererSettings->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig));

	const FString RelativePath = RendererSettings->GetDefaultConfigFilename();
	const FString FullPath = FPaths::ConvertRelativePathToFull(RelativePath);

	const bool bIsWriteable = !FPlatformFileManager::Get().GetPlatformFile().IsReadOnly(*FullPath);

	if (!bIsWriteable)
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*FullPath, false);
	}

	for (TFieldIterator<FProperty> PropIt(RendererSettings->GetClass()); PropIt; ++PropIt)
	{
		const FProperty* Property = *PropIt;
		if (Property->GetFName() == PropertyName)
		{
			RendererSettings->UpdateSinglePropertyInConfigFile(Property, RendererSettings->GetDefaultConfigFilename());
		}
	}

	if (!bIsWriteable)
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*FullPath, true);
	}

	RendererSettings->PostEditChange();
}


void ULTFunctions::ChangeDefaultRHI(EDefaultGraphicsRHI InNewRHI)
{
	UWindowsTargetSettings* const Settings =  GetMutableDefault<UWindowsTargetSettings>();
	Settings->DefaultGraphicsRHI = InNewRHI;
	if (const FProperty* DefaultGraphicsRHIProperty = Settings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UWindowsTargetSettings, DefaultGraphicsRHI)))
	{
		Settings->UpdateSinglePropertyInConfigFile(DefaultGraphicsRHIProperty, Settings->GetDefaultConfigFilename());
	}
	SuggestRestart();
}


void ULTFunctions::ChangeDynamicGlobalIlluminationMethod(TEnumAsByte<EDynamicGlobalIlluminationMethod::Type> InNewMethod)
{
	URendererSettings* Settings = GetMutableDefault<URendererSettings>();
	Settings->DynamicGlobalIllumination = InNewMethod;
	
	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, DynamicGlobalIllumination));

    
	FString Command = TEXT("r.DynamicGlobalIlluminationMethod ");

	// Convert enum value to string
	Command.Append(FString::FromInt(static_cast<int32>(InNewMethod)));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}
}

void ULTFunctions::ChangeReflectionMethod(TEnumAsByte<EReflectionMethod::Type> InNewMethod)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->Reflections = InNewMethod;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, Reflections));
    
	FString Command = TEXT("r.ReflectionMethod ");

	// Convert enum value to string
	Command.Append(FString::FromInt(static_cast<int32>(InNewMethod)));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}
}


void ULTFunctions::ChangeEnableRayTracing(bool InNewState)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->bEnableRayTracing = InNewState;
	
	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, Reflections));
	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, bSupportSkinCacheShaders));
    
	FString Command = TEXT("r.RayTracing ");

	// Convert enum value to string
	Command.Append(FString::FromInt(InNewState));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);

		if(InNewState)
		{
			Settings->bSupportSkinCacheShaders = 1;
			GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), TEXT("r.SkinCache.CompileShaders 1"));
		}
	}

	SuggestRestart();
}

void ULTFunctions::ChangeEnablePathTracing(bool InNewState)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->bEnablePathTracing = InNewState;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, bEnablePathTracing));
    
	FString Command = TEXT("r.PathTracing ");

	// Convert enum value to string
	Command.Append(FString::FromInt(InNewState));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}

	SuggestRestart();
}

void ULTFunctions::ChangeUseHardwareRayTracingWhenAvailable(bool InNewState)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->bUseHardwareRayTracingForLumen = InNewState;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, bUseHardwareRayTracingForLumen));
    
	FString Command = TEXT("r.Lumen.HardwareRayTracing.LightingMode ");

	// Convert enum value to string
	Command.Append(FString::FromInt(InNewState));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}
}

void ULTFunctions::ChangeSoftwareRayTracingMode(TEnumAsByte<ELumenSoftwareTracingMode::Type> InNewMode)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->LumenSoftwareTracingMode = InNewMode;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, LumenSoftwareTracingMode));
    
	FString Command = TEXT("r.Lumen.TraceMeshSDFs ");

	// Convert enum value to string
	Command.Append(FString::FromInt(static_cast<int32>(InNewMode)));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}
	
	SuggestRestart();
}

void ULTFunctions::ChangeShadowMapMethod(TEnumAsByte<EShadowMapMethod::Type> InNewMethod)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->ShadowMapMethod = InNewMethod;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, ShadowMapMethod));
    
	FString Command = TEXT("r.Shadow.Virtual.Enable ");

	// Convert enum value to string
	Command.Append(FString::FromInt(static_cast<int32>(InNewMethod)));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}
	
	SuggestRestart();
}

void ULTFunctions::ChangeGenerateMeshDistanceField(bool InNewState)
{
	URendererSettings* const  Settings = GetMutableDefault<URendererSettings>();
	Settings->bGenerateMeshDistanceFields = InNewState;

	UpdateConfigAPropertyFromRendererSettings(Settings,GET_MEMBER_NAME_CHECKED(URendererSettings, bGenerateMeshDistanceFields));
    
	FString Command = TEXT("r.GenerateMeshDistanceFields ");

	// Convert enum value to string
	Command.Append(FString::FromInt(InNewState));

	// Check if GUnrealEd and World are available
	if (GUnrealEd && GEditor->GetEditorWorldContext().World())
	{
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(), *Command);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("GUnrealEd or Editor World Context not accessible!"));
	}

	SuggestRestart();
}

void ULTFunctions::ChangePathTracingViewMode()
{
	if(!GEditor){return;}
	if(UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEditor))
	{
		if(const auto ViewportClient = static_cast<FEditorViewportClient*>(EditorEngine->GetActiveViewport()->GetClient()))
		{
			if(ViewportClient->GetViewMode() != VMI_PathTracing)
			{
				ViewportClient->SetViewMode(VMI_PathTracing);
			}
			else
			{
				ViewportClient->SetViewMode(VMI_Lit);
			}
		}
	}
}

void ULTFunctions::EnablePathTracingProgress(bool InEnable)
{
	if(GUnrealEd)
	{
		const FString Command = FString::Printf(TEXT("r.PathTracing.ProgressDisplay %d"),static_cast<int32>(InEnable));
		GUnrealEd->Exec(GEditor->GetEditorWorldContext().World(),*Command);
	}
}


void ULTFunctions::ApplyLightRenderPresetData(const FLightRenderData& InLightRenderData)
{
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!GUnrealEd || !World) { return; }

    UWindowsTargetSettings* const WindowsTargetSettingsSettings = GetMutableDefault<UWindowsTargetSettings>();
    URendererSettings* const RendererSettings = GetMutableDefault<URendererSettings>();

    // Default Graphics RHI
    if (WindowsTargetSettingsSettings->DefaultGraphicsRHI != InLightRenderData.DefaultGraphicsRHI)
    {
        WindowsTargetSettingsSettings->DefaultGraphicsRHI = InLightRenderData.DefaultGraphicsRHI;
        WindowsTargetSettingsSettings->UpdateSinglePropertyInConfigFile(WindowsTargetSettingsSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UWindowsTargetSettings, DefaultGraphicsRHI)), WindowsTargetSettingsSettings->GetDefaultConfigFilename());
    }

	//Skin Cache Shaders
	if(RendererSettings->bSupportSkinCacheShaders != InLightRenderData.SupportSkinCacheShaders)
	{
		RendererSettings->bSupportSkinCacheShaders = 1;
		UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, bSupportSkinCacheShaders));
		FString Command = TEXT("r.SkinCache.CompileShaders ");
		Command.Append(FString::FromInt(InLightRenderData.SupportSkinCacheShaders));
		GUnrealEd->Exec(World, *Command);
	}
	
    // Ray Tracing
    if (RendererSettings->bEnableRayTracing != InLightRenderData.EnableRayTracing)
    {
        RendererSettings->bEnableRayTracing = InLightRenderData.EnableRayTracing;
        UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, bEnableRayTracing));
        FString Command = TEXT("r.RayTracing ");
        Command.Append(FString::FromInt(InLightRenderData.EnableRayTracing));
        GUnrealEd->Exec(World, *Command);
    }

	// Path Tracing
	if (RendererSettings->bEnablePathTracing != InLightRenderData.PathTracing)
	{
		RendererSettings->bEnablePathTracing = InLightRenderData.PathTracing;
		UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, bEnablePathTracing));
		FString Command = TEXT("r.PathTracing ");
		Command.Append(FString::FromInt(InLightRenderData.PathTracing));
		GUnrealEd->Exec(World, *Command);
	}
	
    // Use Hardware Ray Tracing For Lumen
    if (RendererSettings->bUseHardwareRayTracingForLumen != InLightRenderData.UseHardwareRayTracingWhenAvailable)
    {
        RendererSettings->bUseHardwareRayTracingForLumen = InLightRenderData.UseHardwareRayTracingWhenAvailable;
        UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, bUseHardwareRayTracingForLumen));
        FString Command = TEXT("r.Lumen.HardwareRayTracing.LightingMode ");
        Command.Append(FString::FromInt(InLightRenderData.UseHardwareRayTracingWhenAvailable));
        Command.Append(FString());
        GUnrealEd->Exec(World, *Command);
    }

    // Dynamic Global Illumination
    if (RendererSettings->DynamicGlobalIllumination != InLightRenderData.DynamicGlobalIlluminationMethod)
    {
        RendererSettings->DynamicGlobalIllumination = InLightRenderData.DynamicGlobalIlluminationMethod;
        UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, DynamicGlobalIllumination));
        FString Command = L"r.DynamicGlobalIlluminationMethod ";
        Command.Append(FString::FromInt(static_cast<int32>(InLightRenderData.DynamicGlobalIlluminationMethod)));
        GUnrealEd->Exec(World, *Command);
    }

    // Reflections
    if (RendererSettings->Reflections != InLightRenderData.ReflectionMethod)
    {
        RendererSettings->Reflections = InLightRenderData.ReflectionMethod;
        UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, Reflections));
        FString Command = TEXT("r.ReflectionMethod ");
        Command.Append(FString::FromInt(static_cast<int32>(InLightRenderData.ReflectionMethod)));
        GUnrealEd->Exec(World, *Command);
    }

    // Software Ray Tracing Mode
    if (RendererSettings->LumenSoftwareTracingMode != InLightRenderData.SoftwareRayTracingMode)
    {
        RendererSettings->LumenSoftwareTracingMode = InLightRenderData.SoftwareRayTracingMode;
        UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, LumenSoftwareTracingMode));
        FString Command = TEXT("r.Lumen.TraceMeshSDFs ");
        Command.Append(FString::FromInt(static_cast<int32>(InLightRenderData.SoftwareRayTracingMode)));
        GUnrealEd->Exec(World, *Command);
    }

	if (RendererSettings->ShadowMapMethod != InLightRenderData.ShadowMapMethod)
	{
		RendererSettings->ShadowMapMethod = InLightRenderData.ShadowMapMethod;
		UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, ShadowMapMethod));
		FString Command = TEXT("r.Shadow.Virtual.Enable ");
		Command.Append(FString::FromInt(static_cast<int32>(InLightRenderData.ShadowMapMethod)));
		GUnrealEd->Exec(World, *Command);
	}

	// Use Generate Mesh Distance Field
	if (RendererSettings->bGenerateMeshDistanceFields != InLightRenderData.GenerateMeshDistanceField)
	{
		RendererSettings->bGenerateMeshDistanceFields = InLightRenderData.GenerateMeshDistanceField;
		UpdateConfigAPropertyFromRendererSettings(RendererSettings, GET_MEMBER_NAME_CHECKED(URendererSettings, bGenerateMeshDistanceFields));
		FString Command = TEXT("r.GenerateMeshDistanceFields ");
		Command.Append(FString::FromInt(InLightRenderData.GenerateMeshDistanceField));
		GUnrealEd->Exec(World, *Command);
	}

    SuggestRestart();
}

void ULTFunctions::SuggestRestart()
{
	if (ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor"))
	{
		SettingsEditorModule->OnApplicationRestartRequired();
	}
}

UWorld* ULTFunctions::GetEditorWorld()
{
	return GEditor->GetEditorWorldContext().World();
}

FVector ULTFunctions::GetLocationOnViewport(float DistanceToCamera)
{
	if(!GCurrentLevelEditingViewportClient || !GEditor){return FVector::ZeroVector;}
	
	const FVector StartLoc = GCurrentLevelEditingViewportClient->GetViewLocation();
	FVector EndLoc = StartLoc + GCurrentLevelEditingViewportClient->GetViewRotation().Vector() * DistanceToCamera;

	auto EditorWorld = GEditor->GetEditorWorldContext().World();
	if(!EditorWorld){return FVector::ZeroVector;}

	static const FName LineTraceSingleName(TEXT("EditorToolLineTrace"));
	FCollisionQueryParams CollisionParams(LineTraceSingleName);
	CollisionParams.bTraceComplex = false;
	FCollisionObjectQueryParams ObjectParams;
	
	ObjectParams.AddObjectTypesToQuery(ECC_Visibility);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	FHitResult HitResult;

	EditorWorld->LineTraceSingleByObjectType(HitResult, StartLoc, EndLoc, ObjectParams, CollisionParams);

	if(HitResult.bBlockingHit)
	{
		return HitResult.ImpactPoint + GCurrentLevelEditingViewportClient->GetViewRotation().Vector() * -50.0f;
	}
	return EndLoc;
}



void ULTFunctions::ChangeLightsVisibility(bool IsVisible, const TArray<ALight*>& InLights)
{
	for(const auto& Light : InLights)
	{
		Light->GetLightComponent()->SetVisibility(IsVisible);
	}
}

void ULTFunctions::ChangeLightsBrightness(const float& InBrightness, const TArray<ALight*>& InLights)
{
	for(const auto& Light : InLights)
	{
		Light->GetLightComponent()->SetIntensity(InBrightness);
	}
}

void ULTFunctions::ChangeLightsColor(const FLinearColor& InColor, const TArray<ALight*>& InLights)
{
	for(const auto& Light : InLights)
	{
		Light->GetLightComponent()->SetLightColor(InColor);
	}
}

void ULTFunctions::ChangeLightsFunction(UMaterialInterface* InMaterial, const TArray<ALight*>& InLights)
{
	for(const auto& Light : InLights)
	{
		Light->GetLightComponent()->SetLightFunctionMaterial(InMaterial);
	}
}

void ULTFunctions::CopyLightComponentProperties(ULightComponent* Source, ULightComponent* Destination)
{
	if (Source && Destination)
	{
		for (TFieldIterator<FProperty> It(Source->GetClass()); It; ++It)
		{
			const FProperty* SourceProp = *It;

			// Skip transform properties
			FString PropName = SourceProp->GetName();
			if (PropName.Equals("RelativeLocation") || PropName.Equals("RelativeRotation") ||
				PropName.Equals("RelativeScale3D") || PropName.Equals("bAbsoluteLocation") ||
				PropName.Equals("bAbsoluteRotation") || PropName.Equals("bAbsoluteScale"))
			{
				continue; // Skip this property
			}

			// Check if the property is editable
			if (SourceProp->HasAnyPropertyFlags(CPF_Edit))
			{
				// Check if the destination has this property and they are of the same class
				const FProperty* DestProp = Destination->GetClass()->FindPropertyByName(SourceProp->GetFName());
				if (DestProp && SourceProp->GetClass() == DestProp->GetClass())
				{
					const void* SourceValue = SourceProp->ContainerPtrToValuePtr<void>(Source);
					void* DestValue = DestProp->ContainerPtrToValuePtr<void>(Destination);

					SourceProp->CopyCompleteValue(DestValue, SourceValue);
				}
			}
		}
	}
}

TArray<FAssetData> ULTFunctions::GetAllTexturesInPath(const FString& InPath,const FString& InUserPath)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.ClassPaths.Add(UTexture2D::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(*InPath);
	if(!InUserPath.IsEmpty())
	{
		Filter.PackagePaths.Add(*InUserPath);
	}
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	
	return AssetData;
}

TArray<FAssetData> ULTFunctions::GetAllIESTexturesInPath(const FString& InPath,const FString& InUserPath)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.ClassPaths.Add(UTextureLightProfile::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(*InPath);
	Filter.PackagePaths.Add(TEXT("/Engine/EngineLightProfiles"));
	if(!InUserPath.IsEmpty())
	{
		Filter.PackagePaths.Add(*InUserPath);
	}
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	return AssetData;
}

TArray<FName> ULTFunctions::GetTextureNames(const TArray<FAssetData>& InTextures)
{
	TArray<FName> LocalTextureNames;

	for(const FAssetData& LocalAssetData : InTextures)
	{
		LocalTextureNames.Add(GetRidOfTextureSuffix(LocalAssetData.AssetName));
	}
	return LocalTextureNames;
}

FName ULTFunctions::GetRidOfTextureSuffix(const FName& InTextureName)
{
	FString LocalString = InTextureName.ToString();
	if(LocalString.StartsWith(TEXT("T_")))
	{
		LocalString.RemoveFromStart(TEXT("T_"));
	}
	return *LocalString;
}

UTexture2D* ULTFunctions::FindAndGetTextureFromList(const TArray<FAssetData>& InTextures, const FName& InTextureName)
{
	if(InTextureName.IsNone() && InTextures.IsEmpty())
	{
		return nullptr;
	}
	
	for(const FAssetData& LocalData : InTextures)
	{
		if(LocalData.AssetName.ToString().Contains(InTextureName.ToString()))
		{
			const auto LocalAsset =  LocalData.GetAsset();
			return Cast<UTexture2D>(LocalAsset);
		}
	}
	return nullptr;
}

UTextureLightProfile* ULTFunctions::FindAndGetIESTextureFromList(const TArray<FAssetData>& InTextures, const FName& InTextureName)
{
	if(InTextureName.IsNone() && InTextures.IsEmpty())
	{
		return nullptr;
	}
	
	for(const FAssetData& LocalData : InTextures)
	{
		if(LocalData.AssetName.ToString().Contains(InTextureName.ToString()))
		{
			const auto LocalAsset =  LocalData.GetAsset();
			return Cast<UTextureLightProfile>(LocalAsset);
		}
	}
	return nullptr;
	
}

AActor* ULTFunctions::SpawnAndGetHDRIVolume()
{
	const auto EditorWorld = GEditor->GetEditorWorldContext().World();
	if(!EditorWorld){return nullptr;}
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	const auto FoundAsset = AssetRegistryModule.Get().GetAssetByObjectPath(FString(TEXT("/HDRIBackdrop/Blueprints/HDRIBackdrop.HDRIBackdrop")));

	// Ensure the asset is a Blueprint
	UClass* AssetClass = nullptr;
	if (FoundAsset.GetClass()->GetFName() == UBlueprint::StaticClass()->GetFName())
	{
		// Convert asset to Blueprint and then to UClass
		if (const UBlueprint* BlueprintAsset = Cast<UBlueprint>(FoundAsset.GetAsset()))
		{
			AssetClass = BlueprintAsset->GeneratedClass;
		}
	}

	if (!AssetClass || !AssetClass->IsChildOf(AActor::StaticClass())) return nullptr;

	// Spawn the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* SpawnedActor =  EditorWorld->SpawnActor<AActor>(AssetClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if(SpawnedActor)
	{
		SpawnedActor->Tags.Add(LIGHTING_TOOL_HDRI_BACKDROP_NAME);
	}
	else
	{
		UE_LOG(LogLighting,Error,TEXT("Failed to create HDRI Backdrop Actor!"));
	}
	
	return SpawnedActor;
}

AActor* ULTFunctions::GetHDRIVolumeInTheLevel()
{
	if(const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> ActorItr(EditorWorld); ActorItr; ++ActorItr)
		{
			AActor* Actor = *ActorItr;

			TArray<FName>& ActorTags = Actor->Tags;

			if(ActorTags.IsEmpty()){continue;}

			for(const FName& CurrentTag : ActorTags)
			{
				if(CurrentTag.ToString().Contains(LIGHTING_TOOL_HDRI_BACKDROP_NAME))
				{
					return Actor;
				}
			}
		}
	}
	return nullptr;
}


bool ULTFunctions::DoesToolHDRIActorExist()
{
	if(const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> ActorItr(EditorWorld); ActorItr; ++ActorItr)
		{
			AActor* Actor = *ActorItr;

			TArray<FName>& ActorTags = Actor->Tags;

			if(ActorTags.IsEmpty()){continue;}

			for(const FName& CurrentTag : ActorTags)
			{
				if(CurrentTag.ToString().Contains(LIGHTING_TOOL_HDRI_BACKDROP_NAME))
				{
					return true;
				}
			}
		}
	}
	return false;
}


TArray<FAssetData> ULTFunctions::GetAllHDRIsInPath(const FString& InPath,bool bInRecursivePaths)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	TArray<FAssetData> AssetData;
	FARFilter Filter;
	Filter.ClassPaths.Add(UTextureCube::StaticClass()->GetClassPathName());
	Filter.PackagePaths.Add(*InPath);
	Filter.bRecursivePaths = bInRecursivePaths;
	AssetRegistryModule.Get().GetAssets(Filter, AssetData);
	
	return AssetData;
}

TArray<FName> ULTFunctions::GetHDRINames(const TArray<FAssetData>& InTextures)
{
	TArray<FName> LocalTextureNames;

	for(const FAssetData& LocalAssetData : InTextures)
	{
		LocalTextureNames.Add(GetRidOfHDRISuffix(LocalAssetData.AssetName));
	}
	return LocalTextureNames;
}

FName ULTFunctions::GetRidOfHDRISuffix(const FName& InHDRIName)
{
	FString LocalString = InHDRIName.ToString();
	if(LocalString.StartsWith(TEXT("TC_")))
	{
		LocalString.RemoveFromStart(TEXT("TC_"));
	}
	return *LocalString;
}

UTextureCube* ULTFunctions::FindAndGetHDRIFromList(const TArray<FAssetData>& InHDRIs, const FName& InHDRIName)
{
	for(const FAssetData& LocalData : InHDRIs)
	{
		if(LocalData.AssetName.ToString().Contains(InHDRIName.ToString()))
		{
			const auto LocalAsset =  LocalData.GetAsset();
			return Cast<UTextureCube>(LocalAsset);
		}
	}
	return nullptr;
}


void ULTFunctions::ReRunConstructionScriptOfActor(AActor* InActor)
{
	InActor->RerunConstructionScripts();
}

void ULTFunctions::GoToWorldOriginWithOffset(FVector Offset)
{
	UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEditor);
	const auto ViewportClient = static_cast<FEditorViewportClient*>(EditorEngine->GetActiveViewport()->GetClient());
	if(!ViewportClient){return;}
	
	ViewportClient->ViewTransformPerspective.SetLocation(Offset);
	ViewportClient->ViewTransformPerspective.SetRotation(FRotator::ZeroRotator);
	ViewportClient->ViewTransformPerspective.SetLookAt(FVector::ZeroVector);
	
	// Broadcast 'camera moved' delegate
	FEditorDelegates::OnEditorCameraMoved.Broadcast(ViewportClient->GetViewLocation(), ViewportClient->GetViewRotation(), ViewportClient->ViewportType, ViewportClient->ViewIndex);
}

#pragma endregion ProjectSettings


