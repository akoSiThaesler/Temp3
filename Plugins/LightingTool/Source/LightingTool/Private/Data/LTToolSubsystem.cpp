// Copyright 2024 Leartes Studios. All Rights Reserved.

#include "Data/LTToolSubsystem.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "EditorUtilityLibrary.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EngineUtils.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "LightingTool.h"
#include "LTFunctions.h"
#include "LTLightingLogChannels.h"
#include "LTSensorVisualizer.h"
#include "LTSensorVisualizerComponent.h"
#include "LTToolData.h"
#include "LTToolStyle.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h"
#include "ActorFactories/ActorFactory.h"
#include "Animation/AnimClassInterface.h"
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/UObjectIterator.h"
#include "Volume/AutoLightmapVolume.h"
#include "Widgets/Docking/SDockTab.h"

DECLARE_DELEGATE(FOnDropSignature)


class UEditorActorSubsystem;


#define LOCTEXT_NAMESPACE "Lighting Tool Subsystem"

#pragma region DragDropClass

class FLightingToolDragOp : public FAssetDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FLightingToolDragOp, FAssetDragDropOp)

	// FDragDropOperation interface
	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override;
	virtual void OnDragged(const class FDragDropEvent& DragDropEvent) override;
	virtual void Construct() override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	void SetCanDropHere(bool bCanDropHere)
	{
		MouseCursor = bCanDropHere ? EMouseCursor::TextEditBeam : EMouseCursor::SlashedCircle;
	}

	static TSharedRef<FLightingToolDragOp> New(FAssetData AssetDataArray, UActorFactory* ActorFactory,
	                                           const FSlateIcon& InSlateIcon);

	FOnDropSignature OnDropSignature;

protected:
	FSlateIcon SlateIcon;

	FLightingToolDragOp();
};

FLightingToolDragOp::FLightingToolDragOp()
{
}

TSharedPtr<SWidget> FLightingToolDragOp::GetDefaultDecorator() const
{
	return SNew(SBox)
		.HeightOverride(64.0f)
		.WidthOverride(64.0f)
		[
			SNew(SImage)

			.Image(SlateIcon.GetIcon())
		];
}

void FLightingToolDragOp::OnDragged(const FDragDropEvent& DragDropEvent)
{
	if (CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow->MoveWindowTo(DragDropEvent.GetScreenSpacePosition());
	}
}


void FLightingToolDragOp::Construct()
{
	MouseCursor = EMouseCursor::GrabHandClosed;

	FDragDropOperation::Construct();
}

void FLightingToolDragOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	OnDropSignature.ExecuteIfBound();
}

TSharedRef<FLightingToolDragOp> FLightingToolDragOp::New(FAssetData AssetDataArray, UActorFactory* ActorFactory,
                                                         const FSlateIcon& InSlateIcon)
{
	TSharedRef<FLightingToolDragOp> Operation = MakeShareable(new FLightingToolDragOp);
	Operation->SlateIcon = InSlateIcon;
	Operation->Init({AssetDataArray}, TArray<FString>(), ActorFactory);
	Operation->Construct();
	return Operation;
}

#pragma endregion DragDropClass

ULTToolSubsystem::ULTToolSubsystem(): LTToolData()
{
}

void ULTToolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if ((LTToolData = NewObject<ULTToolData>(this, FName(TEXT("LTToolData")))))
	{
		LTToolData->LoadConfig();

		if (GEditor)
		{
			LTToolData->IdealLightMapDensity = GEditor->IdealLightMapDensity;
			LTToolData->MaxLightMapDensity = GEditor->MaxLightMapDensity;
		}
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));
	ContentBrowserModule.GetOnAssetSelectionChanged().AddLambda(
		[this](const TArray<FAssetData>& NewSelectedAssets, bool bIsPrimaryBrowser)
		{
			OnAssetSelectionChangedSignature.Broadcast();
		});

	//Actor Selection Tracing
	FLevelEditorModule& levelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	levelEditor.OnActorSelectionChanged().AddLambda([this](const TArray<UObject*>& InActors, bool bIsSelected)
	{
		OnActorSelectionChangedSignature.Broadcast();
	});

	if (GUnrealEd)
	{
		const TSharedPtr<FLTSensorVisualizer> SensorVisualizer = MakeShareable(new FLTSensorVisualizer());
		GUnrealEd->RegisterComponentVisualizer(ULTSensorVisualizerComponent::StaticClass()->GetFName(),
		                                       SensorVisualizer);
		SensorVisualizer->OnRegister();
	}
}

void ULTToolSubsystem::Deinitialize()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::GetModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));
	ContentBrowserModule.GetOnAssetSelectionChanged().RemoveAll(this);

	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(ULTSensorVisualizerComponent::StaticClass()->GetFName());
	}

	Super::Deinitialize();
}

TArray<AAutoLightmapVolume*> ULTToolSubsystem::GetAutoLightmapVolumes()
{
	TArray<AAutoLightmapVolume*> FoundActors;
	for (TActorIterator<AAutoLightmapVolume> ActorItr(GEditor->GetEditorWorldContext().World(),
	                                                  AAutoLightmapVolume::StaticClass()); ActorItr; ++ActorItr)
	{
		FoundActors.Add(*ActorItr);
	}
	return FoundActors;
}

void ULTToolSubsystem::GenerateLightmapResolutions()
{
	const FScopedTransaction Transaction(LOCTEXT("LightingTool_Lightmap", "Changed Component Lightmap Resolutions"));

	//Infinite Extent
	if (LTToolData->InfiniteExtent)
	{
		for (TObjectIterator<UStaticMeshComponent> Itr; Itr; ++Itr)
		{
			UStaticMeshComponent* SMComponent = *Itr;

			if (!IsValid(SMComponent) || !IsValid(SMComponent->GetOwner()) || SMComponent->IsEditorOnly() || SMComponent
				->Mobility == EComponentMobility::Movable || SMComponent->GetOwner()->GetName().
				                                                          Contains(FString(TEXT("Default__"))) ||
				SMComponent->GetOwner()->GetName().Contains(FString(TEXT("Default__"))) || !IsValid(
					SMComponent->GetStaticMesh())) { continue; }

			const auto TargetRes = ULTFunctions::CalculateDesiredLightmapResolution(
				SMComponent, LTToolData->LastLightmapDensity, LTToolData->MaxAllowedResolution);

			if (TargetRes <= 0) { continue; }

			SMComponent->Modify();
			SMComponent->bOverrideLightMapRes = true;
			SMComponent->OverriddenLightMapRes = TargetRes;
			SMComponent->InvalidateLightingCache();
		}
	}
	else
	{
		auto ApplyDensity = [this](const AAutoLightmapVolume* InVolume, UStaticMeshComponent* InSMComponent)
		{
			float Density = LTToolData->LastLightmapDensity;
			if (!LTToolData->DensityType.IsEqual(TEXT("None")))
			{
				const float Extent = (InVolume->GetBounds().BoxExtent.X + InVolume->GetBounds().BoxExtent.Y + InVolume->
					GetBounds().BoxExtent.Z) * 0.66;
				const float Alpha = 1 - FVector::Distance(InVolume->GetActorLocation(),
				                                          InSMComponent->GetComponentLocation()) / Extent;

				if (LTToolData->DensityType.IsEqual(TEXT("Linear")))
				{
					Density = FMath::Lerp(0, LTToolData->LastLightmapDensity, Alpha);
				}
				else
				{
					const EEasingFunc::Type EaseType =
						ULTFunctions::GetEasingFuncValueFromName(LTToolData->DensityType);
					Density = ULTFunctions::AdjustDensityWithEase(0, LTToolData->LastLightmapDensity, Alpha, EaseType);
				}

				if (!InSMComponent->GetOwner())
				{
					return;
				}
			}

			int32 TargetRes = ULTFunctions::CalculateDesiredLightmapResolution(
				InSMComponent, Density, LTToolData->MaxAllowedResolution);
			if (TargetRes <= 4)
			{
				TargetRes = 4;
			}

			InSMComponent->Modify();
			InSMComponent->bOverrideLightMapRes = true;
			InSMComponent->OverriddenLightMapRes = TargetRes;
			InSMComponent->InvalidateLightingCache();
		};

		//Only Volumes
		const TArray<AAutoLightmapVolume*> Volumes = GetAutoLightmapVolumes();
		if (Volumes.IsEmpty()) { return; }
		for (TObjectIterator<UStaticMeshComponent> Itr; Itr; ++Itr)
		{
			UStaticMeshComponent* SMComponent = *Itr;

			if (!IsValid(SMComponent) || !IsValid(SMComponent->GetStaticMesh())) { continue; }

			for (const AAutoLightmapVolume* CurrentVolume : Volumes)
			{
				if (CurrentVolume->EncompassesPoint(SMComponent->GetComponentLocation(),
				                                    SMComponent->Bounds.SphereRadius))
				{
					ApplyDensity(CurrentVolume, SMComponent);
					break;
				}
			}
		}
	}
}

void ULTToolSubsystem::GenerateLightmapResolutionsForAssets()
{
	for (const auto FoundAsset : UEditorUtilityLibrary::GetSelectedAssets())
	{
		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(FoundAsset))
		{
			const auto DesiredDensity = ULTFunctions::CalculateDesiredLightmapResolutionForStaticMesh(
				StaticMesh, LTToolData->LastLightmapDensity, LTToolData->MaxAllowedResolution);
			StaticMesh->SetLightMapResolution(DesiredDensity);
			StaticMesh->PostEditChange();
			StaticMesh->MarkPackageDirty();
		}
	}
}

void ULTToolSubsystem::OverrideLightmapCoordinateIndex(const int32& InIndex)
{
	for (const auto FoundAsset : UEditorUtilityLibrary::GetSelectedAssets())
	{
		if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(FoundAsset))
		{
			StaticMesh->SetLightMapCoordinateIndex(InIndex);
			StaticMesh->PostEditChange();
			StaticMesh->MarkPackageDirty();
		}
	}
}


void ULTToolSubsystem::AddLightmapVolumeManually()
{
	UActorFactory* ActorFactory = GEditor->FindActorFactoryByClass(UActorFactory::StaticClass());
	const TSharedRef<FAssetDragDropOp> DragDropOperation = FLightingToolDragOp::New(
		FAssetData(AAutoLightmapVolume::StaticClass(), true), ActorFactory,
		FSlateIcon(FLTToolStyle::GetToolStyleName(), "LightingTool.LightmapVolumeIcon"));
	FSlateApplication::Get().CancelDragDrop(); // calls onDrop method on the current drag drop operation

	const FVector2D CurrentCursorPosition = FSlateApplication::Get().GetCursorPos();
	const FVector2D LastCursorPosition = FSlateApplication::Get().GetLastCursorPos();

	TSharedPtr<SWindow> TargetWindow;
	
	//Find Parent Tab
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(
		TEXT("LevelEditor"));
	const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	auto FoundTab = LevelEditorTabManager->FindExistingLiveTab(FLightingToolModule::GetALToolTabID());
	if (!FoundTab)
	{
		UE_LOG(LogLighting, Error, TEXT("Failed to finding tab!"));
		return;
	}

	if (FoundTab.IsValid() && FoundTab->GetParentWindow().IsValid())
	{
		TargetWindow = FoundTab->GetParentWindow();
	}
	else
	{
		// Use the active top-level window as a fallback
		TargetWindow = FSlateApplication::Get().GetActiveTopLevelWindow();

		// If no valid window exists, create a temporary one
		if (!TargetWindow.IsValid())
		{
			TSharedRef<SWindow> FallbackWindow = SNew(SWindow)
				.Title(FText::FromString(TEXT("Fallback Drag Window")))
				.ClientSize(FVector2D(1, 1)) // Minimal size
				.IsPopupWindow(true);        // Acts as a transient popup window

			FSlateApplication::Get().AddWindow(FallbackWindow);
			TargetWindow = FallbackWindow;
		}
	}
	
	TSet<FKey> PressedMouseButtons;
	PressedMouseButtons.Add(EKeys::LeftMouseButton);

	FModifierKeysState ModifierKeyState;

	FPointerEvent FakePointerEvent(
		FSlateApplication::Get().GetUserIndexForMouse(),
		FSlateApplicationBase::CursorPointerIndex,
		CurrentCursorPosition,
		LastCursorPosition,
		PressedMouseButtons,
		EKeys::Invalid,
		0,
		ModifierKeyState);

	// Make a fake mouse event for slate, so we can initiate a drag and drop.
	FDragDropEvent DragDropEvent(FakePointerEvent, DragDropOperation);

	// Start the drag operation
	if (TargetWindow.IsValid())
	{
		FSlateApplication::Get().ProcessDragEnterEvent(TargetWindow.ToSharedRef(), DragDropEvent);
	}
	else
	{
		UE_LOG(LogLighting, Error, TEXT("No valid window to process drag event."));
	}
}


void ULTToolSubsystem::AddLightByDrag(TSubclassOf<AActor> InLightClass)
{
    FSlateIcon SlateIcon = FSlateIcon(FLTToolStyle::GetToolStyleName(),
                                      FLTToolStyle::GetLightStyleNameByClassName(*InLightClass->GetName()));
    UActorFactory* ActorFactory = GEditor->FindActorFactoryByClass(UActorFactory::StaticClass());
    const TSharedRef<FLightingToolDragOp> DragDropOperation = FLightingToolDragOp::New(
        FAssetData(InLightClass, true), ActorFactory, SlateIcon);

    // Handle when drag is dropped or canceled
    DragDropOperation->OnDropSignature.BindLambda([this]()
    {
        OnActorDroppedSignature.Broadcast(nullptr);
    });

    FSlateApplication::Get().CancelDragDrop();
    const FVector2D CurrentCursorPosition = FSlateApplication::Get().GetCursorPos();
    const FVector2D LastCursorPosition = FSlateApplication::Get().GetLastCursorPos();

    // Construct a fake mouse event for initiating the drag
    TSet<FKey> PressedMouseButtons;
    PressedMouseButtons.Add(EKeys::LeftMouseButton);

    FModifierKeysState ModifierKeyState;

    FPointerEvent FakePointerEvent(
        FSlateApplication::Get().GetUserIndexForMouse(),
        FSlateApplicationBase::CursorPointerIndex,
        CurrentCursorPosition,
        LastCursorPosition,
        PressedMouseButtons,
        EKeys::Invalid,
        0,
        ModifierKeyState);

    FDragDropEvent DragDropEvent(FakePointerEvent, DragDropOperation);

    // Try to find a valid parent window or create a fallback window
    TSharedPtr<SWindow> TargetWindow;

    // Check parent window of the tab
    const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
    const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
    const auto FoundTab = LevelEditorTabManager->FindExistingLiveTab(FLightingToolModule::GetLTToolTabID());
    if (FoundTab.IsValid() && FoundTab->GetParentWindow().IsValid())
    {
        TargetWindow = FoundTab->GetParentWindow();
    }
    else
    {
        // Use the active top-level window as a fallback
        TargetWindow = FSlateApplication::Get().GetActiveTopLevelWindow();

        // If no valid window exists, create a temporary one
        if (!TargetWindow.IsValid())
        {
            TSharedRef<SWindow> FallbackWindow = SNew(SWindow)
                .Title(FText::FromString(TEXT("Fallback Drag Window")))
                .ClientSize(FVector2D(1, 1)) // Minimal size
                .IsPopupWindow(true);        // Acts as a transient popup window

            FSlateApplication::Get().AddWindow(FallbackWindow);
            TargetWindow = FallbackWindow;
        }
    }

    // Start the drag operation
    if (TargetWindow.IsValid())
    {
        FSlateApplication::Get().ProcessDragEnterEvent(TargetWindow.ToSharedRef(), DragDropEvent);
    }
    else
    {
        UE_LOG(LogLighting, Error, TEXT("No valid window to process drag event."));
    }
}

FName GetToolTabIDWithType(ELTLightingToolType ToolType)
{
	switch (ToolType)
	{
	case ELTLightingToolType::LightMapTool:
		return FLightingToolModule::GetALToolTabID();
	case ELTLightingToolType::LightsTool:
		return FLightingToolModule::GetLTToolTabID();
	case ELTLightingToolType::HDRIManager:
		return FLightingToolModule::GetHDRIToolTabID();
	case ELTLightingToolType::LightRenderTool:
		return FLightingToolModule::GetLRToolTabID();
	default: return {};
	}
}

void ULTToolSubsystem::ResizeToolWindow(ELTLightingToolType ToolType, const FVector2D NewSize)
{
	const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(
		TEXT("LevelEditor"));
	const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	if (const auto FoundTab = LevelEditorTabManager->FindExistingLiveTab(GetToolTabIDWithType(ToolType)))
	{
		if (FoundTab->GetParentWindow().IsValid())
		{
			FoundTab->GetParentWindow()->Resize(FVector2D(NewSize.X, NewSize.Y));
		}
	}
	else
	{
		UE_LOG(LogLighting, Warning, TEXT("Not Found Tool Tab Named:%s For Resizing"),
		       *GetToolTabIDWithType(ToolType).ToString());
	}
}

void ULTToolSubsystem::DisableAllResolutionOverrides(bool bIsInInfiniteMode)
{
	// Lambda for invalidating lighting cache of a static mesh component
	auto InvalidateSMComponent = [](UStaticMeshComponent* SMComponent) {
		SMComponent->bOverrideLightMapRes = false;
		SMComponent->InvalidateLightingCache();
	};

	const TArray<AAutoLightmapVolume*> Volumes = GetAutoLightmapVolumes();
	const bool bHasVolumes = !Volumes.IsEmpty();

	if(!bIsInInfiniteMode && !bHasVolumes)
	{
		return;
	}

	for (TObjectIterator<UStaticMeshComponent> Itr; Itr; ++Itr)
	{
		UStaticMeshComponent* SMComponent = *Itr;
		if (!IsValid(SMComponent)) {continue;}

		if (bIsInInfiniteMode)
		{
			InvalidateSMComponent(SMComponent);
		}
		else
		{
			const FVector ComponentLocation = SMComponent->GetComponentLocation();
			const float SphereRadius = SMComponent->Bounds.SphereRadius;

			for (const AAutoLightmapVolume* CurrentVolume : Volumes)
			{
				if (IsValid(CurrentVolume) && CurrentVolume->EncompassesPoint(ComponentLocation, SphereRadius))
				{
					InvalidateSMComponent(SMComponent);
					break;
				}
			}
		}
	}
}


void ULTToolSubsystem::SetupOnActorsDroppedEvent(const TArray<UObject*>& DroppedObjects, const TArray<AActor*>& DroppedActors)
{
	OnActorDroppedSignature.Broadcast(DroppedActors[0]);

	FEditorDelegates::OnNewActorsDropped.Remove(DelegateHandle);
}

void ULTToolSubsystem::OnDropEventOccured()
{
}


#undef LOCTEXT_NAMESPACE
