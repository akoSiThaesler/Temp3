// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "Volume/AutoLightmapVolume.h"
#include "ImageUtils.h"
#include "Components/BillboardComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Texture2D.h"
#include "Interfaces/IPluginManager.h"


AAutoLightmapVolume::AAutoLightmapVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;

	// Structure to hold one-time initialization
	
#if WITH_EDITORONLY_DATA
	
	if (!IsRunningCommandlet())
	{
		SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
		if (SpriteComponent)
		{
			FString PluginDir = IPluginManager::Get().FindPlugin("LightingTool")->GetBaseDir();
			const FString TextureFilePath = FPaths::Combine(PluginDir, TEXT("Resources/LightmapVolume.png"));
			if(UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(TextureFilePath))
			{
				SpriteComponent->Sprite = Cast<UTexture2D>(Texture);
			}
			
			SpriteComponent->SetRelativeScale3D(FVector(3.f, 3.f, 3.f));
			SpriteComponent->bHiddenInGame = false;
			SpriteComponent->SpriteInfo.Category = TEXT("");
			SpriteComponent->SpriteInfo.DisplayName = FText::FromName(TEXT("Auto Lightmap Volume"));
			SpriteComponent->bIsScreenSizeScaled = true;
			SpriteComponent->SetUsingAbsoluteScale(true);
			SpriteComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			SpriteComponent->SetupAttachment(GetRootComponent());
		}
	}
#endif
	
	SetActorScale3D(FVector(6.0f));
}





