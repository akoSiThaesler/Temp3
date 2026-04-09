// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "LTData.h"
#include "UObject/Object.h"
#include "LTLightSpecObject.generated.h"


UCLASS(BlueprintType,Config = LightsToolData)
class LIGHTINGTOOL_API ULTLightSpecObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Settings" ,DisplayName="Enable")
	bool bEnable = true;

	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Settings" , meta=(UIMin = 0, UIMax = 160,Units="cd"))
	float Brightness = 30.0f;
	
	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Settings")
	ELTLightType FunctionType = ELTLightType::None;

	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Function", meta=(EditConditionHides,EditCondition = "FunctionType != ELTLightType::None && FunctionType != ELTLightType::Standard"))
	float Speed = 0.05;
	
	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Function", meta=(EditConditionHides,EditCondition = "FunctionType != ELTLightType::None && FunctionType != ELTLightType::Standard"))
	bool RandomInitialOffset = true;

	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Function", meta=(EditConditionHides,EditCondition = "FunctionType != ELTLightType::None && FunctionType != ELTLightType::Standard && RandomInitialOffset"))
	bool UseSeed = false;

	UPROPERTY(Config,EditAnywhere, BlueprintReadWrite, Category = "Light Function", meta=(EditConditionHides,EditCondition = "FunctionType != ELTLightType::None && FunctionType != ELTLightType::Standard && RandomInitialOffset && UseSeed"))
	int32 Seed = 1;


private:
	UPROPERTY(Config)
	FString UserMaskPath = FString();

	UPROPERTY(Config)
	FString UserIESProfilePath = FString();
	
public:
	/*** Getters and Setters ***/
	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	FString GetUserMaskPath() const { return UserMaskPath; }

	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	void SetUserMaskPath(const FString& InPath) { UserMaskPath = InPath; }

	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	FString GetUserIESProfilePath() const { return UserIESProfilePath; }

	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	void SetUserIESProfilePath(const FString& InPath) { UserIESProfilePath = InPath; }	
	
	/*** Save and Load Data ***/
	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	void SaveLightSpecData();
	
	UFUNCTION(BlueprintCallable,Category="Lighting Tool")
	void LoadLightSpecData();
};
