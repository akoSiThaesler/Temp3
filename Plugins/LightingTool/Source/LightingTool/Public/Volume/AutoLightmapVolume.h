// Copyright 2024 Leartes Studios. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "AutoLightmapVolume.generated.h"

UCLASS(hidecategories=(Navigation),Blueprintable)
class LIGHTINGTOOL_API AAutoLightmapVolume : public AVolume
{
	GENERATED_BODY()

#if WITH_EDITORONLY_DATA
	/** Billboard used to see the trigger in the editor */
	UPROPERTY(Category = TriggerBase, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif
	
public:
	// Sets default values for this actor's properties
	AAutoLightmapVolume();
	
};
