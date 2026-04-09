// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once


#include "CoreMinimal.h"
#include "CineCameraComponent.h"
#include "Engine/DataTable.h"
#include "Camera/CameraTypes.h"
#include "CineCameraActor.h"
#include "CMCameraPresetData.generated.h"


USTRUCT(BlueprintType)
struct FCMCineCameraData
{
	GENERATED_USTRUCT_BODY()

	// Default constructor
	FCMCineCameraData();
	
	//Cine Camera Actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FCameraLookatTrackingSettings CameraLookatTrackingSettings;
	
	//Cine Camera Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FCameraFilmbackSettings Filmback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FCameraLensSettings LensSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FCameraFocusSettings FocusSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float CurrentFocalLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float CurrentAperture;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float OrthoWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float OrthoNearClipPlane;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float OrthoFarClipPlane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float AspectRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	uint8 bConstrainAspectRatio : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	uint8 bUseFieldOfViewForLOD : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	bool bDrawFrustumAllowed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	uint8 bCameraMeshHiddenInGame : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	uint8 bLockToHmd : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	float PostProcessBlendWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FPostProcessSettings PostProcessSettings;
	
	static FCMCineCameraData CreatePreset(const ACineCameraActor* InCineCameraActor);
	static void LoadPreset(const ACineCameraActor* InCineCameraActor, const FCMCineCameraData& InPresetData);
};


inline FCMCineCameraData::FCMCineCameraData()
	: CameraLookatTrackingSettings(FCameraLookatTrackingSettings()),
	  Filmback(FCameraFilmbackSettings()),
	  LensSettings(FCameraLensSettings()),
	  FocusSettings(FCameraFocusSettings()),
	  CurrentFocalLength(35.0f),
	  CurrentAperture(2.8f),
	  FieldOfView(90.0f),
	  OrthoWidth(512.0f),
	  OrthoNearClipPlane(0.0f),
	  OrthoFarClipPlane(WORLD_MAX),
	  AspectRatio(1.777778f),
	  bConstrainAspectRatio(false),
	  bUseFieldOfViewForLOD(true),
	  bDrawFrustumAllowed(true),
	  bCameraMeshHiddenInGame(true),
	  bLockToHmd(true),
	  ProjectionMode(ECameraProjectionMode::Perspective),
	  PostProcessBlendWeight(1.0f)
{
}

inline FCMCineCameraData FCMCineCameraData::CreatePreset(const ACineCameraActor* InCineCameraActor)
{
	// Extract settings from the CineCameraActor
	FCMCineCameraData NewPreset;
	NewPreset.FieldOfView = InCineCameraActor->GetCineCameraComponent()->FieldOfView;
	NewPreset.OrthoWidth = InCineCameraActor->GetCineCameraComponent()->OrthoWidth;
	NewPreset.OrthoNearClipPlane = InCineCameraActor->GetCineCameraComponent()->OrthoNearClipPlane;
	NewPreset.OrthoFarClipPlane = InCineCameraActor->GetCineCameraComponent()->OrthoFarClipPlane;
	NewPreset.AspectRatio = InCineCameraActor->GetCineCameraComponent()->AspectRatio;
	NewPreset.bConstrainAspectRatio = InCineCameraActor->GetCineCameraComponent()->bConstrainAspectRatio;
	NewPreset.ProjectionMode = InCineCameraActor->GetCineCameraComponent()->ProjectionMode;
	NewPreset.Filmback = InCineCameraActor->GetCineCameraComponent()->Filmback;
	NewPreset.LensSettings = InCineCameraActor->GetCineCameraComponent()->LensSettings;
	NewPreset.FocusSettings = InCineCameraActor->GetCineCameraComponent()->FocusSettings;
	NewPreset.CurrentFocalLength = InCineCameraActor->GetCineCameraComponent()->CurrentFocalLength;
	NewPreset.CurrentAperture = InCineCameraActor->GetCineCameraComponent()->CurrentAperture;
	NewPreset.bUseFieldOfViewForLOD = InCineCameraActor->GetCineCameraComponent()->bUseFieldOfViewForLOD;
	NewPreset.bDrawFrustumAllowed = InCineCameraActor->GetCineCameraComponent()->bDrawFrustumAllowed;
	NewPreset.bCameraMeshHiddenInGame = InCineCameraActor->GetCineCameraComponent()->bCameraMeshHiddenInGame;
	NewPreset.bLockToHmd = InCineCameraActor->GetCineCameraComponent()->bLockToHmd;

	NewPreset.PostProcessSettings = InCineCameraActor->GetCineCameraComponent()->PostProcessSettings;
	NewPreset.PostProcessBlendWeight = InCineCameraActor->GetCineCameraComponent()->PostProcessBlendWeight;

	for (TFieldIterator<FProperty> PropIt(InCineCameraActor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Prop = *PropIt;
		
		if (Prop->GetName() == "LookatTrackingSettings")
		{
			if (FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
			{
				const void* StructData = StructProperty->ContainerPtrToValuePtr<void>(InCineCameraActor);

				if (StructProperty->Struct == FCameraLookatTrackingSettings::StaticStruct())
				{
					if(const FCameraLookatTrackingSettings* ConstLookatSettings = static_cast<const FCameraLookatTrackingSettings*>(StructData))
					{
						NewPreset.CameraLookatTrackingSettings = *ConstLookatSettings;
						break;
					}
				}
			}
		}

	}

	return NewPreset;
}

inline void FCMCineCameraData::LoadPreset(const ACineCameraActor* InCineCameraActor, const FCMCineCameraData& InPresetData)
{
    if (!InCineCameraActor) {return;}

    UCineCameraComponent* CameraComponent = InCineCameraActor->GetCineCameraComponent();
    if (!CameraComponent) return;

    // Load basic camera settings
    CameraComponent->SetFieldOfView(InPresetData.FieldOfView);
    CameraComponent->SetOrthoWidth(InPresetData.OrthoWidth);
    CameraComponent->SetOrthoNearClipPlane(InPresetData.OrthoNearClipPlane);
    CameraComponent->SetOrthoFarClipPlane(InPresetData.OrthoFarClipPlane);
    CameraComponent->SetAspectRatio(InPresetData.AspectRatio);
    CameraComponent->bConstrainAspectRatio = InPresetData.bConstrainAspectRatio;
    CameraComponent->ProjectionMode = InPresetData.ProjectionMode;
    CameraComponent->bUseFieldOfViewForLOD = InPresetData.bUseFieldOfViewForLOD;
    CameraComponent->bDrawFrustumAllowed = InPresetData.bDrawFrustumAllowed;
    CameraComponent->bCameraMeshHiddenInGame = InPresetData.bCameraMeshHiddenInGame;
    CameraComponent->bLockToHmd = InPresetData.bLockToHmd;

    // Load filmback settings
    CameraComponent->Filmback = InPresetData.Filmback;

    // Load lens settings
    CameraComponent->LensSettings = InPresetData.LensSettings;

    // Load focus settings
    CameraComponent->FocusSettings = InPresetData.FocusSettings;

    // Load additional settings like focal length and aperture
    CameraComponent->CurrentFocalLength = InPresetData.CurrentFocalLength;
    CameraComponent->CurrentAperture = InPresetData.CurrentAperture;

    // Load post-process settings
    CameraComponent->PostProcessSettings = InPresetData.PostProcessSettings;
    CameraComponent->PostProcessBlendWeight = InPresetData.PostProcessBlendWeight;

    for (TFieldIterator<FProperty> PropIt(CameraComponent->GetClass(), EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
    {
        FProperty* Prop = *PropIt;
        if (Prop->GetName() == TEXT("LookatTrackingSettings"))
        {
            void* PropContainer = Prop->ContainerPtrToValuePtr<void>(CameraComponent);
            Prop->CopyCompleteValue_InContainer(PropContainer, &InPresetData.CameraLookatTrackingSettings);
            break;
        }
    }
}



USTRUCT(BlueprintType)
struct FCineCameraPresetData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	// Default constructor
	FCineCameraPresetData();

	// Fully Parameterized constructor
	FCineCameraPresetData(const FName& InPresetName, const FName& InDescription, const FCMCineCameraData& InCineCameraData);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FName PresetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FName Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Preset")
	FCMCineCameraData CineCameraData;
};

inline FCineCameraPresetData::FCineCameraPresetData(){}

inline FCineCameraPresetData::FCineCameraPresetData(const FName& InPresetName, const FName& InDescription,const FCMCineCameraData& InCineCameraData): PresetName(InPresetName), Description(InDescription), CineCameraData(InCineCameraData){}

