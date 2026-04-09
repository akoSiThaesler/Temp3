// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "LTData.generated.h"

class UTexture2D;

UENUM(BlueprintType, Category = "Lighting Tool")
enum class ELTLightingToolType: uint8
{
	None				UMETA(DisplayName = "None"),
	LightMapTool		UMETA(DisplayName = "Light Map Tool"),
	LightsTool			UMETA(DisplayName = "Lights Tool"),
	LightRenderTool		UMETA(DisplayName = "Light Render Settings Tool"),
	HDRIManager			UMETA(DisplayName = "HDRI Tool")
};

UENUM(BlueprintType, Category = "Lighting Tool")
enum class ELTLightmapPreset: uint8
{
	None			UMETA(DisplayName = "None"),
	BestPerformance	UMETA(DisplayName = "Best Performance"),
	Performance		UMETA(DisplayName = "Performance"),
	Quality			UMETA(DisplayName = "Quality"),
	BestQuality		UMETA(DisplayName = "Best Quality")
};

UENUM(BlueprintType, Category = "Lighting Tool")
enum class ELTLightType: uint8
{
	None			UMETA(DisplayName = "None"),
	Standard		UMETA(DisplayName = "Standard"),
	Flickering		UMETA(DisplayName = "Flickering"),
	Breathing		UMETA(DisplayName = "Breathing")
};

USTRUCT(BlueprintType)
struct FLTAssetViewData
{
	GENERATED_BODY()
	
	FORCEINLINE FLTAssetViewData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	FName AssetName = FName();

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	FName ObjectPath = FName();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	float CurrentDensity = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	int32 LightmapResolution = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	int32 CoordIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	FLinearColor Color = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	int32 AssetIndex = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Lighting Tool")
	bool bIsSelected = false;
};

FLTAssetViewData::FLTAssetViewData()
{
	
}
