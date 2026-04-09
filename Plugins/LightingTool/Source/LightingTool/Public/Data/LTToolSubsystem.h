// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "LTData.h"
#include "LTToolSubsystem.generated.h"

class ALight;
class AAutoLightmapVolume;
class ULTToolData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FObjectSelectionChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDroppedSignature, AActor*, OutActor);

UCLASS() 
class LIGHTINGTOOL_API ULTToolSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ULTToolData> LTToolData;
	
public:
	UFUNCTION(BlueprintCallable,BlueprintPure,BlueprintGetter)
	ULTToolData* GetLightingToolData() const {return LTToolData;}
	
	ULTToolSubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

private:
	static TArray<AAutoLightmapVolume*> GetAutoLightmapVolumes();

public:
	UFUNCTION(BlueprintCallable,Category = "LightingTool|Lightmap")
	void GenerateLightmapResolutions();

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void GenerateLightmapResolutionsForAssets();

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void OverrideLightmapCoordinateIndex(const int32& InIndex);
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void AddLightmapVolumeManually();

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void AddLightByDrag(TSubclassOf<AActor> InLightClass);
	
	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void ResizeToolWindow(ELTLightingToolType ToolType, const FVector2D NewSize);

	UFUNCTION(BlueprintCallable, Category = "LightingTool|Lightmap")
	void DisableAllResolutionOverrides(bool bIsInInfiniteMode);


	
private:
	UFUNCTION()
	void SetupOnActorsDroppedEvent(const TArray<UObject*>& DroppedObjects, const TArray<AActor*>& DroppedActors);

	FDelegateHandle DelegateHandle;

	UFUNCTION()
	void OnDropEventOccured();

public:
	UPROPERTY(BlueprintAssignable)
	FObjectSelectionChangedSignature OnAssetSelectionChangedSignature;

	UPROPERTY(BlueprintAssignable)
	FObjectSelectionChangedSignature OnActorSelectionChangedSignature;

	UPROPERTY(BlueprintAssignable)
	FOnActorDroppedSignature OnActorDroppedSignature;

public:
	UPROPERTY(BlueprintReadWrite,Category="LightingTool|HDRI")
	bool IncludePluginHDRIs = true;

	UPROPERTY(BlueprintReadWrite,Category="LightingTool|HDRI")
	bool IncludeProjectHDRIs = true;
	
};
