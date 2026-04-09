// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "LTData.h"
#include "UObject/Object.h"
#include "LTToolData.generated.h"


UCLASS(Config = LightMapToolSettings)
class LIGHTINGTOOL_API ULTToolData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Config,BlueprintReadWrite,Category="Lighting Tool",meta=(NoResetToDefault))
	bool  bIsInSceneMode = true;
	
	UPROPERTY(EditAnywhere,Config,BlueprintReadWrite,Category="Lighting Tool",meta=(ClampMin =0.0f,UIMin=0.0f,ClampMax=100.0f,UIMax=100.0f,NoResetToDefault))
	float  IdealLightMapDensity = 0.2f;
	
	UPROPERTY(EditAnywhere,Config,BlueprintReadWrite,Category="Lighting Tool",meta=(ClampMin =0.01f,UIMin=0.01f,ClampMax=100.010002f,UIMax=100.010002f,NoResetToDefault))
	float  MaxLightMapDensity = 0.8f;
	
	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	bool IsAutoLightmapEnabled = false;

	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	float  LastLightmapDensity = 0.0f;
	
	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	ELTLightmapPreset LightmapPreset = ELTLightmapPreset::None;

	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	bool  InfiniteExtent = false;

	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	bool  GradientDensity = false;

	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool")
	FName  DensityType = TEXT("Linear");
		
	UPROPERTY(EditAnywhere,Config,BlueprintReadWrite,Category="Lighting Tool",meta=(ClampMin = 4,UIMin=4, ClampMax = 4096,UIMax = 4096, NoResetToDefault))
	int32  MaxAllowedResolution = 1024;

	UPROPERTY(Config,BlueprintReadWrite,Category="Lighting Tool",meta=(NoResetToDefault))
	bool bIsListViewExpanded = false;
	
	//Functions
	UFUNCTION(BlueprintCallable, Category = "Light Functions")
	void DataParamChanged();
	
	//Functions
	UFUNCTION(BlueprintPure, Category = "Light Functions")
	bool ToggleAndGetListViewExpandStatus();

	UFUNCTION(BlueprintCallable, Category = "Light Functions")
	void SetIdealDensity(const float& InIdealDensity);

	UFUNCTION(BlueprintCallable, Category = "Light Functions")
	void SetMaxAllowedDensity(const float& InMaxAllowedDensity);

	UFUNCTION(BlueprintCallable, Category = "Light Functions")
	void SetMaxAllowedResolution(const int32& InMaxAllowedResolution);

private:
	static void CheckForMaximumDensity(const float& InIdealDensity);
};
