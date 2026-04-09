// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
//#include "Developer/Windows/WindowsTargetPlatformSettings/Classes/WindowsTargetSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/RendererSettings.h"
#include "Data/LTLightRenderData.h"
#include "Engine/Light.h"
#include "Kismet/KismetMathLibrary.h"
#include "LTFunctions.generated.h"

class UUserWidget;
class APostProcessVolume;
enum class EMBAssetThumbnailFormant : uint8;
class UTexture2D;
class UStaticMesh;
class ACineCameraActor;

UCLASS()
class LIGHTINGTOOL_API ULTFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static int32 CalculateDesiredLightmapResolution(const UStaticMeshComponent* StaticMeshComponent,const float& DesiredDensity,const int32& MaxAllowedRes);
	
	static int32 CalculateDesiredLightmapResolutionForStaticMesh(const UStaticMesh* StaticMesh,const float& DesiredDensity,const int32& MaxAllowedRes);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static void CalculateDensityAndResolution(const FAssetData& InAssetData,float& OutDensity,int32& OutResolution,int32& OutCoordIndex);
	
	UFUNCTION(BlueprintPure,Category = "LightingTool|Lightmap")
	static FLinearColor GetColorOfTheDensity(const float& InDensity);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static void SetIdealLightmapDensity(const float& NewIdealDensity);

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static const float& GetIdealLightmapDensity();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static void SetMaxLightmapDensity(const float& NewMaxDensity);
	
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static const float& GetMaxLightmapDensity();
	
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static bool IsThereAnySelectedSMAsset();
	
	UFUNCTION(BlueprintCallable,Category = "LightingTool|Lightmap")
	static void ChangeViewMode();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "LightingTool|Lightmap")
	static EViewModeIndex GetViewMode();

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "LightingTool|Lightmap")
	static UTexture2D* GenerateThumbnailForAsset(const FAssetData& InAssetPath);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static bool SaveSelectedAssets();

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static void ShowNotifyToUser(const FString InMessage);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static int32 ChangeLightmapCoordinateIndex(const FName& InObjectPath,const int32 InNewIndex);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static int32 GetLightmapCoordinateIndex(const FName& InObjectPath);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static void ChangeLightmapResolution(const FName& InObjectPath,const int32 InNewResolution);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static APostProcessVolume* SpawnOrGetExistingPostProcessVolume();

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static TArray<FName> CreateEasingFuncNameArray();
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static EEasingFunc::Type GetEasingFuncValueFromName(const FName& Name);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static EEasingFunc::Type GetEasingFuncValueFromIndex(const int32 Index);
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static float AdjustDensityWithEase(float A, float B, float Alpha, TEnumAsByte<EEasingFunc::Type> EasingFunc);
	
#pragma region ProjectSettings
	
	/** Project Settings **/
	
	static void UpdateConfigAPropertyFromRendererSettings(URendererSettings* const  RendererSettings, FName PropertyName);


	/** Setters **/
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeDefaultRHI(EDefaultGraphicsRHI InNewRHI);
	
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeDynamicGlobalIlluminationMethod(TEnumAsByte<EDynamicGlobalIlluminationMethod::Type> InNewMethod);
	
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeReflectionMethod(TEnumAsByte<EReflectionMethod::Type> InNewMethod);
	
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeEnableRayTracing(bool InNewState);

	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeEnablePathTracing(bool InNewState);

	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeUseHardwareRayTracingWhenAvailable(bool InNewState);

	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeSoftwareRayTracingMode(TEnumAsByte<ELumenSoftwareTracingMode::Type> InNewMode);

	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeShadowMapMethod(TEnumAsByte<EShadowMapMethod::Type> InNewMethod);

	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ChangeGenerateMeshDistanceField(bool InNewState);

	UFUNCTION(BlueprintCallable,Category = "LightingTool|Lightmap")
	static void ChangePathTracingViewMode();

	UFUNCTION(BlueprintCallable,Category = "LightingTool|Lightmap")
	static void EnablePathTracingProgress(bool InEnable);
	
	/** Getters **/
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Lighting Tool")
	static EDefaultGraphicsRHI GetDefaultRHI();
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Lighting Tool")
	static TEnumAsByte<EDynamicGlobalIlluminationMethod::Type> GetDynamicGlobalIlluminationMethod();
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Lighting Tool")
	static TEnumAsByte<EReflectionMethod::Type> GetReflectionMethod();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Lighting Tool")
	static bool GetEnableRayTracing();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Lighting Tool")
	static bool GetEnablePathTracing();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Lighting Tool")
	static bool GetUseHardwareRayTracingWhenAvailable();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Lighting Tool")
	static TEnumAsByte<ELumenSoftwareTracingMode::Type> GetSoftwareRayTracingMode();

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Lighting Tool")
	static TEnumAsByte<EShadowMapMethod::Type> GetShadowMapMethod();

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Lighting Tool")
	static bool GetGenerateMeshDistanceField();
	
	//Preset
	UFUNCTION(BlueprintCallable, Category="Lighting Tool")
	static void ApplyLightRenderPresetData(const FLightRenderData& InLightRenderData);
	
	//Light  Render Functions
	static void SuggestRestart();

#pragma endregion ProjectSettings

#pragma region GenericFunctions

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Editor Tools")
	static UWorld* GetEditorWorld();

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Editor Tools")
	static FVector GetLocationOnViewport(float DistanceToCamera = 500.0f);

	/*UFUNCTION(BlueprintCallable, Category="Editor Tools")
	static FString CreateAFolderPicker();*/

#pragma endregion GenericFunctions

#pragma region LightFunctions
	
	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static void ChangeLightsVisibility(bool IsVisible, const TArray<ALight*>& InLights);
	
	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static void ChangeLightsBrightness(const float& InBrightness, const TArray<ALight*>& InLights);

	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static void ChangeLightsColor(const FLinearColor& InColor, const TArray<ALight*>& InLights);

	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static void ChangeLightsFunction(UMaterialInterface* InMaterial, const TArray<ALight*>& InLights);

	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static void CopyLightComponentProperties(ULightComponent* Source, ULightComponent* Destination);
	
	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static TArray<FAssetData> GetAllTexturesInPath(const FString& InPath,const FString& InUserPath);

	UFUNCTION(BlueprintCallable, Category="Light Functions")
	static TArray<FAssetData> GetAllIESTexturesInPath(const FString& InPath,const FString& InUserPath);
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Light Functions")
	static TArray<FName> GetTextureNames(const TArray<FAssetData>& InTextures);
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Light Functions")
	static FName GetRidOfTextureSuffix(const FName& InTextureName);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Light Functions")
	static UTexture2D* FindAndGetTextureFromList(const TArray<FAssetData>& InTextures,const FName& InTextureName);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Light Functions")
	static class UTextureLightProfile* FindAndGetIESTextureFromList(const TArray<FAssetData>& InTextures,const FName& InTextureName);
	
#pragma endregion LightFunctions

#pragma region HDRIFunctions
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static AActor* SpawnAndGetHDRIVolume();

	UFUNCTION(BlueprintCallable, Category="Editor Tools")
	static bool DoesToolHDRIActorExist();
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	static AActor* GetHDRIVolumeInTheLevel();
	
	UFUNCTION(BlueprintCallable, Category="Editor Tools")
	static TArray<FAssetData> GetAllHDRIsInPath(const FString& InPath,bool bInRecursivePaths);
	
	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Editor Tools")
	static TArray<FName> GetHDRINames(const TArray<FAssetData>& InTextures);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Editor Tools")
	static FName GetRidOfHDRISuffix(const FName& InHDRIName);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="Editor Tools")
	static UTextureCube* FindAndGetHDRIFromList(const TArray<FAssetData>& InHDRIs,const FName& InHDRIName);
	
	UFUNCTION(BlueprintCallable, Category="Editor Tools")
	static void ReRunConstructionScriptOfActor(AActor* InActor);
	
	UFUNCTION(BlueprintCallable, Category="Editor Tools")
	static void GoToWorldOriginWithOffset(FVector Offset);

	
#pragma endregion HDRIFunctions

};

#pragma region ProjectSettingsInline

inline EDefaultGraphicsRHI ULTFunctions::GetDefaultRHI()
{
	const UWindowsTargetSettings* Settings =  GetDefault<UWindowsTargetSettings>();
	return Settings->DefaultGraphicsRHI;
}

inline TEnumAsByte<EDynamicGlobalIlluminationMethod::Type> ULTFunctions::GetDynamicGlobalIlluminationMethod()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->DynamicGlobalIllumination;
}

inline TEnumAsByte<EReflectionMethod::Type> ULTFunctions::GetReflectionMethod()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->Reflections;
}

inline bool ULTFunctions::GetEnableRayTracing()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->bEnableRayTracing;
}


inline bool ULTFunctions::GetEnablePathTracing()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->bEnablePathTracing;
}

inline bool ULTFunctions::GetUseHardwareRayTracingWhenAvailable()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->bUseHardwareRayTracingForLumen;
}

inline TEnumAsByte<ELumenSoftwareTracingMode::Type> ULTFunctions::GetSoftwareRayTracingMode()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->LumenSoftwareTracingMode;
}

inline TEnumAsByte<EShadowMapMethod::Type> ULTFunctions::GetShadowMapMethod()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->ShadowMapMethod;
}

inline bool ULTFunctions::GetGenerateMeshDistanceField()
{
	const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
	return RenderSettings->bGenerateMeshDistanceFields;
}
#pragma endregion ProjectSettingsInline


