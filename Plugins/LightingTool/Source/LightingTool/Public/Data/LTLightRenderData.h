// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "Developer/Windows/WindowsTargetPlatformSettings/Classes/WindowsTargetSettings.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "Engine/RendererSettings.h"
#include "LTLightRenderData.generated.h"


UENUM(BlueprintType, Category = "Lighting Data")
namespace ELightRenderPreset
{
	enum Type
	{
		Custom			UMETA(DisplayName = "Custom"),
		Lumen			UMETA(DisplayName = "Lumen"),
		Static			UMETA(DisplayName = "Static"),
	};
}




USTRUCT(BlueprintType)
struct FLightRenderData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	EDefaultGraphicsRHI DefaultGraphicsRHI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	TEnumAsByte<EDynamicGlobalIlluminationMethod::Type> DynamicGlobalIlluminationMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	TEnumAsByte<EReflectionMethod::Type> ReflectionMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	bool EnableRayTracing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	bool PathTracing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	bool SupportSkinCacheShaders;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	bool UseHardwareRayTracingWhenAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	TEnumAsByte<ELumenSoftwareTracingMode::Type> SoftwareRayTracingMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	TEnumAsByte<EShadowMapMethod::Type> ShadowMapMethod;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Data")
	bool GenerateMeshDistanceField;

	// Default constructor
	FLightRenderData()
		: DefaultGraphicsRHI(EDefaultGraphicsRHI::DefaultGraphicsRHI_Default),
		  DynamicGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::None),
		  ReflectionMethod(EReflectionMethod::None),
		  EnableRayTracing(false),
		  PathTracing(false),  
		  SupportSkinCacheShaders(false),
		  UseHardwareRayTracingWhenAvailable(false),
		  SoftwareRayTracingMode(ELumenSoftwareTracingMode::DetailTracing),
		  ShadowMapMethod(EShadowMapMethod::ShadowMaps),
		  GenerateMeshDistanceField(false)
	{
	}

	// Parameterized constructor
	FLightRenderData(
		EDefaultGraphicsRHI InDefaultGraphicsRHI,
		EDynamicGlobalIlluminationMethod::Type InDynamicGlobalIlluminationMethod,
		EReflectionMethod::Type InReflectionMethod,
		bool InEnableRayTracing,
		bool InPathTracing,
		bool InSupportSkinCacheShaders,
		bool InUseHardwareRayTracingWhenAvailable,
		ELumenSoftwareTracingMode::Type InSoftwareRayTracingMode,
		EShadowMapMethod::Type InShadowMapMethod,
		bool InGenerateMeshDistanceField
	)
		: DefaultGraphicsRHI(InDefaultGraphicsRHI),
		  DynamicGlobalIlluminationMethod(InDynamicGlobalIlluminationMethod),
		  ReflectionMethod(InReflectionMethod),
		  EnableRayTracing(InEnableRayTracing),
		  PathTracing(InPathTracing),
		  SupportSkinCacheShaders(InSupportSkinCacheShaders),
		  UseHardwareRayTracingWhenAvailable(InUseHardwareRayTracingWhenAvailable),
		  SoftwareRayTracingMode(InSoftwareRayTracingMode),
		  ShadowMapMethod(InShadowMapMethod),
		  GenerateMeshDistanceField(InGenerateMeshDistanceField)
	{
	}
};



UCLASS(BlueprintType,Config = LTLightRenderingSettings)
class LIGHTINGTOOL_API ULTLightRenderData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config,BlueprintReadWrite,Category = "Lighting Tool|Light Rendering")
	TEnumAsByte<ELightRenderPreset::Type> Preset = ELightRenderPreset::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool|Light Rendering")
	FLightRenderData LumenPreset;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool|Light Rendering")
	FLightRenderData StaticPreset;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Lighting Tool|Light Rendering")
	bool PathTracingProgressViewState;
	
	UFUNCTION(BlueprintCallable,Category = "Lighting Tool|Light Rendering")
	void SavePresetData();

	UFUNCTION(BlueprintCallable,Category = "Lighting Tool|Light Rendering")
	void LoadPresetData();
};
