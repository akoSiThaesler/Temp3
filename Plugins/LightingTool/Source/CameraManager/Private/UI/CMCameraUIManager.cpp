// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "CMCameraUIManager.h"
#include "CineCameraActor.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "EditorViewportCommands.h"
#include "EngineUtils.h"
#include "LevelEditor.h"
#include "ILevelEditor.h"
#include "LevelEditorActions.h"
#include "LevelEditorViewport.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "SLevelViewport.h"
#include "CMCamDataPipelineWindow.h"
#include "CMCameraCommands.h"
#include "CMCameraPresetData.h"
#include "CMCameraSettings.h"
#include "CMCineCameraDetailsCustomization.h"
#include "CMDebug.h"
#include "CMInputProcessor.h"
#include "CMPresetSelectionWindow.h"
#include "CMSubsystem.h"
#include "CMToolStyle.h"
#include "SWarningOrErrorBox.h"
#include "UnrealEdGlobals.h"
#include "Editor/TransBuffer.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/ScopedSlowTask.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Utilities/CMCameraManagerUtils.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Docking/SDockTab.h"
#include "Utilities/CMDelayedDelegateExecutor.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SSearchBox.h"
#include "Utilities/CMPresetImportHandler.h"
#include "Widgets/Input/SButton.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SComboButton.h"


TSharedPtr<FCameraUIManagerImp> FCameraUIManager::ManagerImpInstance;

static const FName CameraManagerTabName("CMCameraManagerTool");
static const FText CameraManagerTabDisplay = FText::FromString("Camera Manager");
static const FText CameraManagerTabDesc = FText::FromString("Launch Camera Manager");

#define LOCTEXT_NAMESPACE "CameraManagerImporter"

void FCameraUIManagerImp::Initialize()
{
	ActivateInputProcessor();

	InitCameraLevelCommands();
	
	SetupManagerDelegates();
}

void FCameraUIManagerImp::Shutdown()
{
	DeactivateInputProcessor();

	RemoveManagerDelegates();
}

void FCameraUIManager::Initialize()
{
	FCMCameraContextMenuCommands::Register();
	FCMCameraLevelCommands::Register();
	
	if (!ManagerImpInstance.IsValid())
	{
		ManagerImpInstance = MakeShareable(new FCameraUIManagerImp);

		if(ManagerImpInstance.IsValid())
		{
			ManagerImpInstance->Initialize();
		}
	}
}

void FCameraUIManager::Shutdown()
{
	FCMCameraContextMenuCommands::Unregister();
	FCMCameraLevelCommands::Unregister();
	
	if (ManagerImpInstance.IsValid())
	{
		ManagerImpInstance->Shutdown();
		ManagerImpInstance.Reset();
	}
}

void FCameraUIManagerImp::SetupManagerDelegates()
{
	FLevelEditorModule& levelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	levelEditor.OnLevelEditorCreated().AddLambda([this](const TSharedPtr<ILevelEditor>& InLevelEditor)
	{
		if (InLevelEditor.IsValid())
		{
			InLevelEditor->OnActiveViewportChanged().AddRaw(this,&FCameraUIManagerImp::OnActiveViewportChanged);

			//Initial Active Viewport when the tool is opened for the first time
			OnActiveViewportChanged(nullptr,InLevelEditor->GetActiveViewportInterface());
		}
		else
		{
			UE_LOG(LogCameraManager,Warning,TEXT("levelEditor is not valid!"));
		}
	});
}

void FCameraUIManagerImp::RemoveManagerDelegates() const
{
	FLevelEditorModule& levelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	if(levelEditor.GetFirstLevelEditor().IsValid())
	{
		levelEditor.GetFirstLevelEditor()->OnActiveViewportChanged().RemoveAll(this);
	}
}

void FCameraUIManagerImp::SetupUIDelegates()
{
	if (GEngine)
	{
		GEngine->OnLevelActorDeleted().AddRaw(this, &FCameraUIManagerImp::OnLevelActorDeleted);
		GEngine->OnLevelActorAdded().AddRaw(this, &FCameraUIManagerImp::OnLevelActorAdded);
		
		if (GEditor != nullptr && GEditor->Trans != nullptr)
		{
			UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
			TransBuffer->OnTransactionStateChanged().AddRaw(this, &FCameraUIManagerImp::OnTransactionStateChanged);
		}
	}
	//Actor Selection Tracing
	FLevelEditorModule& levelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	levelEditor.OnActorSelectionChanged().AddRaw(this, &FCameraUIManagerImp::OnActorSelectionChanged);
	levelEditor.OnMapChanged().AddRaw(this, &FCameraUIManagerImp::OnMapChanged);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FCameraUIManagerImp::OnTick), 0.3f);
}

void FCameraUIManagerImp::RemoveUIDelegates() const
{
	if (GEngine)
	{
		GEngine->OnLevelActorDeleted().RemoveAll(this);
		GEngine->OnLevelActorAdded().RemoveAll(this);
	}

	if (GEditor != nullptr && GEditor->Trans != nullptr)
	{
		UTransBuffer* TransBuffer = CastChecked<UTransBuffer>(GEditor->Trans);
		TransBuffer->OnTransactionStateChanged().RemoveAll(this);
	}
	
	FLevelEditorModule& levelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	levelEditor.OnActorSelectionChanged().RemoveAll(this);
	levelEditor.OnMapChanged().RemoveAll(this);

	// Cleanup the ticker handle
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	}
}



void FCameraUIManagerImp::SetPilotStateChangedViaTool(bool InState)
{
	bIsPilotStateChangedViaTool = InState;
}

bool FCameraUIManagerImp::GetPilotStateChangedViaTool() const
{
	return bIsPilotStateChangedViaTool;	
}

bool FCameraUIManagerImp::OnTick(float DeltaTime)
{
	CheckForViewportCameraPilotChange();

	return true;
}

// Your function
void FCameraUIManagerImp::OnActiveViewportChanged(TSharedPtr<IAssetViewport> OldViewport, TSharedPtr<IAssetViewport> NewViewport)
{
	// Cast to SLevelViewport
	ActiveViewport = StaticCastSharedPtr<SLevelViewport>(NewViewport);

	if (ActiveViewport.IsValid())
	{
		ActiveActorLock = ActiveViewport.Pin().Get()->GetLevelViewportClient().GetActiveActorLock();
	}
}

void FCameraUIManagerImp::CheckForViewportCameraPilotChange()
{
	if(ActiveViewport.IsValid() && ActiveViewport.Pin().Get()->GetLevelViewportClient().GetActiveActorLock() != ActiveActorLock)
	{
		ActiveActorLock = ActiveViewport.Pin().Get()->GetLevelViewportClient().GetActiveActorLock();
		if(CameraManagerWidget.IsValid())
		{
			CameraManagerWidget->OnCameraPilotStateChanged();
		}
	}
}


void FCameraUIManagerImp::OnLevelActorAdded(AActor* InAddedActor) const
{
	if (ACineCameraActor* AddedCamera = Cast<ACineCameraActor>(InAddedActor))
	{
		if(CameraManagerWidget.IsValid())
		{
			UE_LOG(LogTemp,Warning,TEXT("Camera Added!!!"));
			CameraManagerWidget->AddNewlyAddedCineCameraToTheList(AddedCamera);
		}
	}

}

inline void FCameraUIManagerImp::OnLevelActorDeleted(AActor* InDeletedActor) const
{
	if (const ACineCameraActor* DeletedCamera = Cast<ACineCameraActor>(InDeletedActor))
	{
		if(CameraManagerWidget.IsValid())
		{
			CameraManagerWidget->RemoveDeletedCineCameraFromList(DeletedCamera);
		}
	}
}

void FCameraUIManagerImp::OnMapChanged(UWorld* World, EMapChangeType MapChangeType) const
{
	if(IsValid(World) && MapChangeType == EMapChangeType::LoadMap && CameraManagerWidget.IsValid())
	{
		CameraManagerWidget->RequestCameraUIRegenerate();
	}
}

void FCameraUIManagerImp::OnTransactionStateChanged(const FTransactionContext& TransactionContext, ETransactionStateEventType TransactionState) const
{
	if (TransactionState == ETransactionStateEventType::UndoRedoFinalized && TransactionContext.Title.ToString().Contains(TEXT("Delete Elements")) && CameraManagerWidget.IsValid())
	{
		CameraManagerWidget->RequestCameraUIRegenerate();	
	}
}

void FCameraUIManagerImp::OnActorSelectionChanged(const TArray<UObject*>& InActors, bool bIsSelected) const
{
	if(CameraManagerWidget.IsValid())
	{
		CameraManagerWidget->LevelCameraSelectionChanged();
	}
}

void FCameraUIManagerImp::ToggleCameraManagerTab()
{
	const TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(CameraManagerTabName);
	if (ExistingTab.IsValid())
	{
		// If the tab is already spawned, then close it
		ExistingTab->RequestCloseTab();
	}
	else
	{
		// Register the tab spawner again before invoking it
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(CameraManagerTabName,FOnSpawnTab::CreateRaw(this, &FCameraUIManagerImp::OnSpawnPluginTab))
		                        .SetDisplayName(CameraManagerTabDisplay)
		                        .SetTooltipText(CameraManagerTabDesc)
		                        .SetIcon(FSlateIcon(FCMToolStyle::GetCreatedToolSlateStyleSet()->GetStyleSetName(),"CameraManager.CameraManagerIcon"))
		                        .SetAutoGenerateMenuEntry(false);

		// If the tab is not spawned, then invoke it
		FGlobalTabmanager::Get()->TryInvokeTab(CameraManagerTabName);

		if (CameraManagerMainDock.IsValid() && CameraManagerMainDock->GetParentWindow().IsValid())
		{
			CameraManagerMainDock->GetParentWindow()->Resize(FVector2D(400, 500));
		}

		UE_LOG(LogTemp,Warning,TEXT("Camera Manager Tab Toggled!"));
		SetupUIDelegates();
	}
}


TSharedRef<SDockTab> FCameraUIManagerImp::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	SAssignNew(CameraManagerMainDock, SDockTab)
	.TabRole(ETabRole::NomadTab)
	.OnTabClosed_Lambda([&](TSharedRef<class SDockTab> InParentTab)
	{
		if(CameraManagerWidget.IsValid())
		{
			CameraManagerWidget->Shutdown();
			CameraManagerWidget.Reset();
		}

		RemoveUIDelegates();
		
		// Remove the tab from the tab manager
	   FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(CameraManagerTabName);
	})
	[
		SAssignNew(CameraManagerWidget,SCameraManagerWidget)
	];
	
	return CameraManagerMainDock.ToSharedRef();
}


void FCameraUIManagerImp::ActivateInputProcessor()
{
	if (FSlateApplication::IsInitialized())
	{
		InputProcessor = MakeShared<FCMKeyInputPreProcessor>();
		InputProcessor->OnKeySelected.BindRaw(this, &FCameraUIManagerImp::HandleKeySelected);
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);
	}
}

void FCameraUIManagerImp::DeactivateInputProcessor()
{
	if (FSlateApplication::IsInitialized() && InputProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		InputProcessor.Reset();
	}
}


bool FCameraUIManagerImp::HandleKeySelected(const FKeyEvent& InKey) const
{
	return CameraLevelCommands->ProcessCommandBindings(InKey);
}


void FCameraUIManagerImp::InitCameraLevelCommands()
{
	CameraLevelCommands = MakeShareable(new FUICommandList());
	CameraLevelCommands->MapAction(FCMCameraLevelCommands::Get().CreateCamera,FExecuteAction::CreateRaw(this,&FCameraUIManagerImp::CreateNewCameraOnView));
	CameraLevelCommands->MapAction(FCMCameraLevelCommands::Get().NextCamera,FExecuteAction::CreateRaw(this,&FCameraUIManagerImp::OnNextCameraTriggered));
	CameraLevelCommands->MapAction(FCMCameraLevelCommands::Get().PreviousCamera,FExecuteAction::CreateRaw(this,&FCameraUIManagerImp::OnPreviousCameraTriggered));
	CameraLevelCommands->MapAction(FCMCameraLevelCommands::Get().EjectPilotMode,FExecuteAction::CreateRaw(this,&FCameraUIManagerImp::OnEjectPilotMode));
	CameraLevelCommands->MapAction(FCMCameraLevelCommands::Get().LockToggle,FExecuteAction::CreateRaw(this,&FCameraUIManagerImp::OnToggleLockState));
}

void FCameraUIManagerImp::OnNextCameraTriggered()
{
	GoToNextCamera(true);

	//Log ActiveViewport.IsValid() is valid
	if(ActiveViewport.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("ActiveViewport"));
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("ActiveViewport is not valid!"));
	
	}
		
}

void FCameraUIManagerImp::OnPreviousCameraTriggered()
{
	GoToNextCamera(false);
}

void FCameraUIManagerImp::GoToNextCamera(bool bInShouldNext)
{
	const auto EditorWorld = GEditor->GetEditorWorldContext().World();
	CHECK_AND_RETURN_WITH_ERROR(!EditorWorld, TEXT("EditorWorld not found!"));
	
	// Gather all cine cameras from CameraViewItemModels with the same order
	
	TArray<ACineCameraActor*> FoundCineCameraActors;

	if(CameraManagerWidget.IsValid() && CameraManagerWidget.Get())
	{
		FoundCineCameraActors = CameraManagerWidget->GetCineCameraActorsFromList();
	}
	else
	{
		FoundCineCameraActors = FCMCameraUtils::GetAllCineCameraActors();
	}
	
	CHECK_AND_RETURN_WITH_ERROR(FoundCineCameraActors.IsEmpty(), TEXT("No Cine Camera Actor Found!"));

	// Determine the active camera index
	int32 ActiveIndex = GetActiveCameraIndex(ActiveViewport.Pin(), FoundCineCameraActors);
	if (ActiveIndex >= 0)
	{
		LastCineCameraIndex = ActiveIndex;

		// Update the camera index based on the direction
		UpdateCameraIndex(bInShouldNext, FoundCineCameraActors.Num());
	}
	else if (LastCineCameraIndex >= FoundCineCameraActors.Num() || LastCineCameraIndex < 0)
	{
		LastCineCameraIndex = 0;
	}

	// Lock the viewport to the selected camera
	if (FoundCineCameraActors.IsValidIndex(LastCineCameraIndex) && IsValid(FoundCineCameraActors[LastCineCameraIndex]) && ActiveViewport.IsValid())
	{
		ActiveViewport.Pin()->OnActorLockToggleFromMenu(FoundCineCameraActors[LastCineCameraIndex]);
		
		if(CameraManagerWidget.IsValid() && CameraManagerWidget.Get())
		{
			CameraManagerWidget->HighlightPilotedCamera(FoundCineCameraActors[LastCineCameraIndex]);
		}
	}
}

void FCameraUIManagerImp::PilotTheCamera(ACineCameraActor* InCineCamera)
{
	if (!IsValid(InCineCamera) || !ActiveViewport.IsValid() || !ActiveViewport.Pin()->GetActiveViewport()) { return; }

	if (FLevelEditorViewportClient* ViewportClient = static_cast<FLevelEditorViewportClient*>(ActiveViewport.Pin()->GetActiveViewport()->GetClient()))
	{
		SetPilotStateChangedViaTool(true);
		
		if (ActiveViewport.Pin()->IsActorLocked(InCineCamera))
		{
			// Reset the settings
			ViewportClient->ViewFOV = ViewportClient->FOVAngle;

			ViewportClient->SetActorLock(nullptr);

			// remove roll and pitch from camera when unbinding from actors
			GEditor->RemovePerspectiveViewRotation(true, true, false);
		}
		else
		{
			ActiveViewport.Pin()->OnActorLockToggleFromMenu(InCineCamera);
		}
	}
}


void FCameraUIManagerImp::UpdateCameraIndex(bool bInShouldNext, int32 CameraCount)
{
	bInShouldNext ? ++LastCineCameraIndex : --LastCineCameraIndex;

	if (LastCineCameraIndex < 0)
	{
		LastCineCameraIndex = CameraCount - 1;
	}
	else if (LastCineCameraIndex >= CameraCount)
	{
		LastCineCameraIndex = 0;
	}
}


void FCameraUIManagerImp::OnEjectPilotMode()
{
	const auto EditorWorld = GEditor->GetEditorWorldContext().World();
	CHECK_AND_RETURN_WITH_ERROR(!EditorWorld, TEXT("EditorWorld not found!"));
	
	TArray<ACineCameraActor*> FoundCineCameraActors;
	if(CameraManagerWidget.IsValid() && CameraManagerWidget.Get())
	{
		FoundCineCameraActors = CameraManagerWidget->GetCineCameraActorsFromList();
	}
	else
	{
		FoundCineCameraActors = FCMCameraUtils::GetAllCineCameraActors();
	}
	
	CHECK_AND_RETURN_WITH_ERROR(FoundCineCameraActors.IsEmpty(), TEXT("No Cine Camera Actor Found!"));
	
	ACineCameraActor* PilotedCamera = nullptr;
	if (ActiveViewport.IsValid())
	{
		//If we change the pilot state via the tool, then we should not change the pilot state via the level viewport
		bIsPilotStateChangedViaTool = true;

		if (ActiveViewport.Pin()->GetLevelViewportClient().IsAnyActorLocked())
		{
			if(ActiveViewport.Pin()->GetLevelViewportClient().GetActiveActorLock().IsValid())
			{
				//If it is a cine camera actor
				if(ACineCameraActor* ActiveCamera = Cast<ACineCameraActor>(ActiveViewport.Pin()->GetLevelViewportClient().GetActiveActorLock().Get()))
				{
					PilotedCamera = ActiveCamera;
				}
			}

			if(ActiveViewport.IsValid())
			{
				ActiveViewport.Pin()->GetLevelViewportClient().ViewFOV = ActiveViewport.Pin()->GetLevelViewportClient().FOVAngle;

				ActiveViewport.Pin()->GetLevelViewportClient().SetActorLock(nullptr);
			}
			
			// remove roll and pitch from camera when unbinding from actors
			GEditor->RemovePerspectiveViewRotation(true, true, false);
		}
	}

	if(CameraManagerWidget.IsValid())
	{
		CameraManagerWidget->OnEjectPilotMode(PilotedCamera);
		
	}
}

void FCameraUIManagerImp::OnToggleLockState()
{
	const auto EditorWorld = GEditor->GetEditorWorldContext().World();
	CHECK_AND_RETURN_WITH_ERROR(!EditorWorld, TEXT("EditorWorld not found!"));

	TArray<ACineCameraActor*> FoundCineCameraActors;
	if(CameraManagerWidget.IsValid() && CameraManagerWidget.Get())
	{
		FoundCineCameraActors = CameraManagerWidget->GetCineCameraActorsFromList();
	}
	else
	{
		FoundCineCameraActors = FCMCameraUtils::GetAllCineCameraActors();
	}
	CHECK_AND_RETURN_WITH_ERROR(FoundCineCameraActors.IsEmpty(), TEXT("No Cine Camera Actor Found!"));
	
	if (ActiveViewport.IsValid())
	{
		if (ActiveViewport.Pin()->GetLevelViewportClient().IsAnyActorLocked())
		{
			if(ActiveViewport.Pin()->GetLevelViewportClient().GetActiveActorLock().IsValid())
			{

				//If it is a cine camera actor
				if(ACineCameraActor* ActiveCamera = Cast<ACineCameraActor>(ActiveViewport.Pin()->GetLevelViewportClient().GetActiveActorLock().Get()))
				{
					if(CameraManagerWidget.IsValid() && CameraManagerWidget.Get())
					{
						CameraManagerWidget->OnCameraLockStateChanged(ActiveCamera,!ActiveCamera->IsLockLocation());
					}
					else
					{
						
						if (IsValid(ActiveCamera))
						{
							ActiveCamera->SetLockLocation(!ActiveCamera->IsLockLocation());
						}
					}
				}
			}		
		}
	}
}

void FCameraUIManagerImp::CreateNewCameraOnView()
{
	if(ACineCameraActor* CreatedNewCineCameraActor = FCMCameraUtils::CreateCineCameraOnView())
	{
		const UCMCameraSettings* Config = GetDefault<UCMCameraSettings>();
		if(IsValid(Config) && Config->bAutoPilotCamera)
		{
			PilotTheCamera(CreatedNewCineCameraActor);
		}
	}
}


int32 FCameraUIManagerImp::GetActiveCameraIndex(const TSharedPtr<SLevelViewport>& ActiveViewport, const TArray<ACineCameraActor*>& CameraActors)
{
	int32 ActiveIndex = -1;
	if(!ActiveViewport.IsValid()){return ActiveIndex;}
	auto ActiveLock = ActiveViewport->GetLevelViewportClient().GetActiveActorLock();
	if (ActiveLock.IsValid())
	{
		if (ACineCameraActor* ActiveCamera = Cast<ACineCameraActor>(ActiveLock.Get()))
		{
			CameraActors.Find(ActiveCamera, ActiveIndex);
		}
	}
	return ActiveIndex;
}






















void SCameraManagerWidget::HandleItemToStringArray( const FCameraViewItemModelPtr& GroupOrStatNodePtr, TArray< FString >& out_SearchStrings) const
{
	// Add group or stat name.
	out_SearchStrings.Add( GroupOrStatNodePtr->CameraViewItemInfo->CameraPtr->GetActorLabel());
}

void SCameraManagerWidget::OnFilterTextChanged(const FText& InFilterText)
{
	*FilterText = InFilterText.ToString();
	ApplyFilter();
}

void SCameraManagerWidget::ApplyFilter()
{
	if (FilterText->Len() == 0)
	{
		FilteredCameraViewItemModels = CameraViewItemModels;
	}
	else
	{
		CollectionItemTextFilter->SetRawFilterText(FText::FromString(*FilterText));

		TArray<FCameraViewItemModelPtr> TempFilteredList;
		for (const FCameraViewItemModelPtr& Item : CameraViewItemModels)
		{
			if (CollectionItemTextFilter->PassesFilter(Item))
			{
				TempFilteredList.Add(Item);
			}
		}
		FilteredCameraViewItemModels = TempFilteredList;
	}

	if (TreeViewWidget.IsValid())
	{
		TreeViewWidget->RequestListRefresh();
	}
}

void SCameraManagerWidget::SetupDelegates()
{
	if (GEditor)
	{
		if (UCameraManagerSubsystem* CMSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			if(!CMSubsystem->OnPresetImported.IsBound())
			{
				CMSubsystem->OnPresetImported.BindRaw(this,&SCameraManagerWidget::OnPresetImported);
			}
		}
	}
}


void SCameraManagerWidget::Construct(const FArguments& InArgs)
{
	InitContextMenuCommands();
	RefreshCameraInfos();
	RefreshPresetNames();

	CollectionItemTextFilter = MakeShareable( new FCollectionItemTextFilter( FCollectionItemTextFilter::FItemToStringArray::CreateRaw( this, &SCameraManagerWidget::HandleItemToStringArray ) ) );
	FilterText = MakeShareable( new FString());
	
	ApplyFilter();
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs Args;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	Args.bAllowSearch = true;
	Args.bHideSelectionTip = true;
	Args.bShowOptions = false;
	Args.bShowPropertyMatrixButton  = false;

	DetailsWidget = PropertyModule.CreateDetailView(Args);
	DetailsWidget->SetVisibility(EVisibility::Visible);
	DetailsWidget->RegisterInstancedCustomPropertyLayout(AActor::StaticClass(),FOnGetDetailCustomizationInstance::CreateStatic(&ICMCineCameraDetailsCustomization::MakeInstance));
	DetailsWidget->RegisterInstancedCustomPropertyLayout(USceneComponent::StaticClass(),FOnGetDetailCustomizationInstance::CreateStatic(&ICMCineCameraDetailsCustomization::MakeInstance));
	TAttribute<EVisibility> InitialDetailsBoxVisibility = EVisibility::Visible;

	const UCMCameraSettings* Config = GetDefault<UCMCameraSettings>();

	if(IsValid(Config))
	{
		InitialDetailsBoxVisibility = Config->bShowDetailsPanel ? EVisibility::Visible : EVisibility::Collapsed;
	}

	const FText BlankText = FText::GetEmpty();
	
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(2.0f)	
			[
				SNew(SBox)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "SimpleButton")
					.OnClicked_Raw(this,&SCameraManagerWidget::OnAddCameraClicked)
					.ToolTipText(FText::FromName(TEXT("Create a camera at the current viewport's view")))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(0)
					.Content()
					[
						SNew(SImage)
						.ColorAndOpacity(FSlateColor::UseForeground())
						.Image(FCMToolStyle::GetCreatedToolSlateStyleSet()->GetBrush("CameraManager.AddCameraIcon"))
					]
				]
			]
			
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2, 0, 2, 0))
			[
				SNew(SSearchBox)
				.HintText( LOCTEXT( "FilterSearch", "Search..." ) )
				.ToolTipText( LOCTEXT("FilterSearchHint", "Type here to search"))
				.OnTextChanged(this, &SCameraManagerWidget::OnFilterTextChanged)
			]

			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(FMargin(2,0,1, 0))
			[
				SAssignNew(SettingsButton, SComboButton)
				.ComboButtonStyle( FAppStyle::Get(), "SimpleComboButtonWithIcon")
				.OnGetMenuContent( this, &SCameraManagerWidget::GetSettingsButtonContent)
				.HasDownArrow(false)
				.ButtonContent()
				[
					SNew(SImage)
					.ColorAndOpacity(FSlateColor::UseForeground())
					.Image( FAppStyle::Get().GetBrush("Icons.Settings"))
				]
			]
		]
		
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Vertical)
			.Style(FAppStyle::Get(), "FoliageEditMode.Splitter") 
			.ResizeMode(ESplitterResizeMode::FixedPosition)

			+ SSplitter::Slot()
			.Value_Lambda([&]()
			{
				return SplitterValue;
			})
			[
				SAssignNew(TreeViewWidget, STreeView<FCameraViewItemModelPtr>)
				.ItemHeight(40.0f)
				.TreeItemsSource(&FilteredCameraViewItemModels)
				.SelectionMode(ESelectionMode::Multi)
				.ClearSelectionOnClick(true)
				.OnGenerateRow(this, &SCameraManagerWidget::TreeViewGenerateRow)
				.OnGetChildren(this, &SCameraManagerWidget::TreeViewGetChildren)
				.EnableAnimatedScrolling(true)
				.OnSelectionChanged(this, &SCameraManagerWidget::OnPresetSelectionChanged)
				.OnMouseButtonDoubleClick(this, &SCameraManagerWidget::OnCameraItemDoubleClicked)
				.OnContextMenuOpening(this, &SCameraManagerWidget::ConstructCineCameraContextMenu)
				.HeaderRow
				(
					// Toggle Active //TODO Add ShortMode
					SNew(SHeaderRow)
					+ SHeaderRow::Column(CameraTreeTreeColumns::ColumnID_Pilot)
					.HeaderContentPadding(FMargin(2, 2, 2, 2))
					.DefaultLabel(BlankText)
					.FixedWidth(42.0f)
					.HAlignHeader(HAlign_Center)
					.HAlignCell(HAlign_Center)
					.VAlignCell(VAlign_Center)

					+ SHeaderRow::Column(CameraTreeTreeColumns::ColumnID_Camera)
					.HeaderContentPadding(FMargin(10, 1, 1, 1))
					.DefaultLabel(FText::FromName(CameraTreeTreeColumns::ColumnID_Camera))
					.FillWidth(2.4f)
					.HAlignHeader(HAlign_Left)
					.SortMode(this,&SCameraManagerWidget::GetIDSortMode, ECMQuerySortMode::Type::ByLabel)
					.OnSort(this,&SCameraManagerWidget::OnSortByChanged)

					+ SHeaderRow::Column(CameraTreeTreeColumns::ColumnID_Preset)
					.DefaultLabel(FText::FromName(CameraTreeTreeColumns::ColumnID_Preset))
					.FillWidth(1.6f)
					.HAlignHeader(HAlign_Left)
					.SortMode(this,&SCameraManagerWidget::GetIDSortMode, ECMQuerySortMode::Type::ByPresetName)
					.OnSort(this,&SCameraManagerWidget::OnSortByChanged)

					+ SHeaderRow::Column(CameraTreeTreeColumns::ColumnID_LockState)
					.DefaultLabel(FText::FromName(CameraTreeTreeColumns::ColumnID_LockState))
					.DefaultTooltip(FText::FromName("Lock Camera Movement"))
					.HeaderContentPadding(FMargin(2, 2, 2, 2))
					.FixedWidth(50)
					.HAlignHeader(HAlign_Center)
					.HAlignCell(HAlign_Center)
					.VAlignCell(VAlign_Center)
					.SortMode(this,&SCameraManagerWidget::GetIDSortMode, ECMQuerySortMode::Type::ByLockState)
					.OnSort(this,&SCameraManagerWidget::OnSortByChanged)
				)
				
			]

			+ SSplitter::Slot()
			.Value_Lambda([&]()
			{
				return 1.0f - SplitterValue;
			})
			[
				SAssignNew(DetailsBox,SVerticalBox)
				.Visibility(InitialDetailsBoxVisibility)
				
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(DetailTitleBox, SBorder)
					.Visibility(EVisibility::Collapsed)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(2.0f, 6.0f, 2.0f, -3.0f))
					[
						SNew(STextBlock)
						.Text(FText::FromName(TEXT("Details")))
						.Justification(ETextJustify::Center)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					]
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					DetailsWidget.ToSharedRef()
				]
			]
		]
	];
}

FReply SCameraManagerWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	return ToolMenuCommands->ProcessCommandBindings( InKeyEvent ) ? FReply::Handled() : FReply::Unhandled();
}


EColumnSortMode::Type SCameraManagerWidget::GetIDSortMode(ECMQuerySortMode::Type InColumn) const
{
	return (SortBy == InColumn) ? SortDirection : EColumnSortMode::None;
}

void SCameraManagerWidget::OnSortByChanged(const EColumnSortPriority::Type SortPriority, const FName& ColumnName, const EColumnSortMode::Type NewSortMode)
{
	if(ColumnName == CameraTreeTreeColumns::ColumnID_Camera)
	{
		if(SortBy == ECMQuerySortMode::ByLabel)
		{
			SortDirection = (SortDirection == EColumnSortMode::Descending) ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
		}
		SortBy = ECMQuerySortMode::ByLabel;
	}
	else if(ColumnName == CameraTreeTreeColumns::ColumnID_Preset)
	{
		if(SortBy == ECMQuerySortMode::ByPresetName)
		{
			SortDirection = (SortDirection == EColumnSortMode::Descending) ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
		}
		
		SortBy = ECMQuerySortMode::ByPresetName;
	}
	else if(ColumnName == CameraTreeTreeColumns::ColumnID_LockState)
	{
		if(SortBy == ECMQuerySortMode::ByLockState)
		{
			SortDirection = (SortDirection == EColumnSortMode::Descending) ? EColumnSortMode::Ascending : EColumnSortMode::Descending;
		}
		
		SortBy = ECMQuerySortMode::ByLockState;
	}
	
	ReOrderCameraList();
}

void SCameraManagerWidget::ReOrderCameraList()
{
	FilteredCameraViewItemModels.Sort([&](const FCameraViewItemModelPtr& A, const FCameraViewItemModelPtr& B)
	{
		switch(SortBy)
		{
			case ECMQuerySortMode::ByLabel:
				{
					FString LabelA = A->CameraViewItemInfo->CameraPtr->GetActorLabel();
					FString LabelB = B->CameraViewItemInfo->CameraPtr->GetActorLabel();
					return SortDirection == EColumnSortMode::Ascending ? FCMCameraUtils::AlphanumericCompare(LabelA, LabelB) : FCMCameraUtils::AlphanumericCompare(LabelB, LabelA);
				}
				case ECMQuerySortMode::ByPresetName:
				{
					FString PresetNameA = A->CameraViewItemInfo->PresetName.ToString();
					FString PresetNameB = B->CameraViewItemInfo->PresetName.ToString();
					return SortDirection == EColumnSortMode::Ascending ? FCMCameraUtils::AlphanumericCompare(PresetNameA, PresetNameB) : FCMCameraUtils::AlphanumericCompare(PresetNameB, PresetNameA);
				}
				case ECMQuerySortMode::ByLockState:
				{
					bool LockStateA = A->CameraViewItemInfo->CameraPtr->IsLockLocation();
					bool LockStateB = B->CameraViewItemInfo->CameraPtr->IsLockLocation();
					return SortDirection == EColumnSortMode::Ascending ? LockStateA < LockStateB : LockStateA > LockStateB;
				}
				default:
				{
					return false;
				}
			}
		});

	if(TreeViewWidget.IsValid()){ TreeViewWidget->RequestTreeRefresh(); }
}



void SCameraManagerWidget::LevelCameraSelectionChanged()
{
	if (!bIsItemSelectedViaIO && !CheckIsSelectionProcessing())
	{
		TArray<ACineCameraActor*> FoundCineCameraActors = FCMCameraUtils::GetSelectedCineCameraActors();

		if(FoundCineCameraActors.Num() > 0)
		{
			bIsItemSelectedViaLevel = true;
			
			TreeViewWidget->ClearSelection();
			
			//Select the camera in the tree view
			int32 CamLength = FoundCineCameraActors.Num();
			for (int32 Index = 0; Index < CamLength ; Index++)
			{
				if (FCameraViewItemModelPtr FoundModel = FindModelByCameraActor(FoundCineCameraActors[Index]))
				{
					TreeViewWidget->SetItemSelection(FoundModel, true);
				}
			}
			bIsItemSelectedViaLevel = false;
		}
	}
}


void SCameraManagerWidget::Shutdown()
{
	if (GEditor)
	{
		if (UCameraManagerSubsystem* CMSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			if(CMSubsystem->OnPresetImported.IsBound())
			{
				CMSubsystem->OnPresetImported.Unbind();
			}
		}
	}
	
	ShutdownSubWindow();
}



void SCameraManagerWidget::RefreshCameraInfos()
{
	if(!IsValid(GEditor)){return;}
	
	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!IsValid(EditorWorld))
	{
		UE_LOG(LogCameraManager, Error, TEXT("EditorWorld not found, unable to refresh camera infos!"));
		return;
	}
	
	CameraViewItemModels.Empty();
	
	TArray<AActor*> PilotedCameras = FCMCameraUtils::GetPilotedCameraActors();

	TArray<FName> LocalPresetNames;
	if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
	{
		LocalPresetNames = CameraManager->GetPresetNames();
	}
	
	for (TActorIterator<AActor> ActorItr(EditorWorld); ActorItr; ++ActorItr)
	{
		if (ACineCameraActor* CurrentCamera = Cast<ACineCameraActor>(*ActorItr))
		{
			CameraViewItemModels.Emplace(CreateNewItemFromCamera(LocalPresetNames, /*Check the pilot state */ PilotedCameras.Contains(CurrentCamera),CurrentCamera));
		}
	}
}

void SCameraManagerWidget::RefreshPresetNames()
{
	PresetNames.Empty();

	if (GEditor)
	{
		if (const UCameraManagerSubsystem* CMSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			const TArray<FName>& LocalPresetNames = CMSubsystem->GetPresetNames();

			// Reserve memory to avoid multiple reallocations
			PresetNames.Reserve(LocalPresetNames.Num() + 1);

			PresetNames.Emplace(MakeShared<FString>(TEXT("None")));

			for (const FName& CurrentPreset : LocalPresetNames)
			{
				PresetNames.Emplace(MakeShared<FString>(CurrentPreset.ToString()));
			}
		}
	}
}

void SCameraManagerWidget::OnPresetImported(FName InPresetName, bool bInIsOverridden)
{
	if (!bInIsOverridden)
	{
		// Add the new preset name to the list
		PresetNames.Emplace(MakeShared<FString>(InPresetName.ToString()));
	}
}

void SCameraManagerWidget::RefreshCameraPilotStates()
{
	if(!IsValid(GEditor)){return;}
	
	const UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	if (!IsValid(EditorWorld))
	{
		UE_LOG(LogCameraManager, Error, TEXT("EditorWorld not found, unable to refresh camera infos!"));
		return;
	}
	
	TArray<AActor*> PilotedCameras = FCMCameraUtils::GetPilotedCameraActors();
	
	for(int32 Index = CameraViewItemModels.Num() -1; Index >= 0; --Index)
	{
		FCameraViewItemModelPtr& Model = CameraViewItemModels[Index];
		if(Model.IsValid() && Model->CameraViewItemInfo.IsValid() && Model->CameraViewItemInfo->CameraPtr.IsValid())
		{
			CameraViewItemModels[Index]->CameraViewItemInfo->bPilotState = PilotedCameras.Contains(CameraViewItemModels[Index]->CameraViewItemInfo->CameraPtr.Get());
		}
		else
		{
			CameraViewItemModels.Remove(Model);
		}
	}
}

FCameraViewItemModelPtr SCameraManagerWidget::CreateNewItemFromCamera(const TArray<FName>& InPresetNames, bool InPilotState, ACineCameraActor* InCineCameraActor)
{
	if (!IsValid(InCineCameraActor)) { return {}; }

	const FCameraViewItemInfoPtr NewInfo = MakeShareable(new FCameraViewItemInfo(InCineCameraActor, FCMCameraUtils::GetPresetNameFromCineCamera(InPresetNames, InCineCameraActor),InPilotState));

	NewInfo->OnPilotButtonClicked.BindRaw(this, &SCameraManagerWidget::OnCameraPilotBtnClicked);

	NewInfo->OnLockStateChanged.BindRaw(this, &SCameraManagerWidget::OnCameraLockStateChanged);

	return MakeShareable(new FCameraViewItemModel(NewInfo));
}

void SCameraManagerWidget::AddNewlyAddedCineCameraToTheList(ACineCameraActor* InAddedCamera)
{
	if (!IsValid(InAddedCamera)) { return; }

	TArray<FName> LocalPresetNames;
	if (GEditor)
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			LocalPresetNames = CameraManager->GetPresetNames();
		}
	}

	CameraViewItemModels.Emplace(CreateNewItemFromCamera(LocalPresetNames,false, InAddedCamera));

	ApplyFilter();
}

void SCameraManagerWidget::RemoveDeletedCineCameraFromList(const ACineCameraActor* InDeletedCamera)
{
	if (CameraViewItemModels.IsEmpty()) { return; }
	
	for (int32 Index = CameraViewItemModels.Num() - 1; Index >= 0; --Index)
	{
		if (!CameraViewItemModels[Index].IsValid() || !CameraViewItemModels[Index]->CameraViewItemInfo.IsValid() || !
			CameraViewItemModels[Index]->CameraViewItemInfo->CameraPtr.IsValid() || CameraViewItemModels[Index]->
			CameraViewItemInfo->CameraPtr.Get() == InDeletedCamera)
		{
			CameraViewItemModels.RemoveAt(Index);

			ApplyFilter();
			return;
		}
	}
}



void SCameraManagerWidget::OnCameraPilotStateChanged()
{
	if(!FCameraUIManager::ManagerImpInstance->GetPilotStateChangedViaTool())
	{
		RequestUpdateCameraPilotStates();
	}

	FCameraUIManager::ManagerImpInstance->SetPilotStateChangedViaTool(false);
}

void SCameraManagerWidget::OnEjectPilotMode(const ACineCameraActor* InPilotedCamera)
{
	if(IsValid(InPilotedCamera))
	{
		for (FCameraViewItemModelPtr& CurrentModel : CameraViewItemModels)
		{
			if(CurrentModel->CameraViewItemInfo->CameraPtr.Get() == InPilotedCamera)
			{
				CurrentModel->CameraViewItemInfo->bPilotState = false;

				if(TreeViewWidget.IsValid())
				{
					TreeViewWidget->ClearHighlightedItems();
				}
				
				break;
			}
		}
	}
	
}

void SCameraManagerWidget::RequestUpdateCameraPilotStates()
{
	RefreshCameraPilotStates();

	if(TreeViewWidget.IsValid())
	{
		TreeViewWidget->RequestListRefresh();
	}
}


void SCameraManagerWidget::RequestCameraUIRegenerate()
{	
	RefreshCameraInfos();

	FilterText->Empty();

	ApplyFilter();
}

void SCameraManagerWidget::OnCameraPilotBtnClicked(TWeakObjectPtr<ACineCameraActor> InCameraPtr)
{
	if (!InCameraPtr.IsValid()) { return; }

	if(FCameraUIManager::ManagerImpInstance.IsValid())
	{
		FCameraUIManager::ManagerImpInstance->PilotTheCamera(InCameraPtr.Get());
	}

	TArray<AActor*> PilotedCameras = FCMCameraUtils::GetPilotedCameraActors();
	
	for (FCameraViewItemModelPtr CurrentModel : FilteredCameraViewItemModels)
	{
		CurrentModel->CameraViewItemInfo->bPilotState = PilotedCameras.Contains(CurrentModel->CameraViewItemInfo->CameraPtr.Get());
	}
}

void SCameraManagerWidget::OnCameraLockStateChanged(ACineCameraActor* InCineCameraActor, bool InNewState)
{
	if (!TreeViewWidget.IsValid()) { return; }

	if (!TreeViewWidget->GetSelectedItems().IsEmpty())
	{
		for (const TSharedPtr<FCameraViewItemModel>& ViewItemModel : TreeViewWidget->GetSelectedItems())
		{
			if (ViewItemModel.IsValid() && ViewItemModel->CameraViewItemInfo->CameraPtr.IsValid())
			{
				ViewItemModel->CameraViewItemInfo->CameraPtr->SetLockLocation(InNewState);
			}
			else
			{
				UE_LOG(LogCameraManager, Error, TEXT("CameraPtr is invalid!"));
			}
		}
	}
	else
	{
		if (IsValid(InCineCameraActor))
		{
			InCineCameraActor->SetLockLocation(InNewState);
		}
	}
}

void SCameraManagerWidget::OnPresetSelectionChanged(TSharedPtr<FCameraViewItemModel> SelectedItem,ESelectInfo::Type SelectInfo)
{
	if(bIsItemSelectedViaLevel){return;}
	
	if (!TreeViewWidget.IsValid()) { return; }

	TreeViewWidget->ClearHighlightedItems();

	TArray<FCameraViewItemModelPtr> ViewItemModels = TreeViewWidget->GetSelectedItems();

	int32 ItemLength = ViewItemModels.Num();
	const bool bAnyItemSelected = ItemLength > 0;

	if (bAnyItemSelected)
	{
		HandleWithItemSelectionTimer();

		//Select all selected camera actors in the editor
		if (const auto EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>())
		{
			EditorActorSubsystem->SelectNothing();

			if (!TreeViewWidget->GetSelectedItems().IsEmpty())
			{
				for (const TSharedPtr<FCameraViewItemModel>& ViewItemModel : ViewItemModels)
				{
					EditorActorSubsystem->SetActorSelectionState(ViewItemModel->CameraViewItemInfo->CameraPtr.Get(),true);
				}
			}
		}
	}

	if (DetailTitleBox.IsValid())
	{
		DetailTitleBox->SetVisibility(bAnyItemSelected ? EVisibility::Visible : EVisibility::Collapsed);
	}

	SplitterValue = bAnyItemSelected ? 0.5f : 0.99f;

	RefreshDetailsWidget();
}

void SCameraManagerWidget::OnCameraItemDoubleClicked(FCameraViewItemModelPtr InItem)
{
	// Check if there are any open viewports
	if (!GEditor || GEditor->GetAllViewportClients().Num() == 0)
	{
		// No open viewports, so return early
		return;
	}

	if (InItem->CameraViewItemInfo->CameraPtr.IsValid())
	{
		AActor* CameraActor = InItem->CameraViewItemInfo->CameraPtr.Get();

		if (const auto EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>())
		{
			EditorActorSubsystem->SelectNothing();
			EditorActorSubsystem->SetActorSelectionState(CameraActor, true);

			if (GUnrealEd)
			{
				GUnrealEd->Exec(CameraActor->GetWorld(), TEXT("CAMERA ALIGN ACTIVEVIEWPORTONLY"));
			}
		}
	}
}

void SCameraManagerWidget::UpdateCameraPresets(TArray<FCameraViewItemModelPtr> InCameraViewItemModels,bool InApplyChanges)
{
	const int32 TaskCount = InCameraViewItemModels.Num();
	FScopedSlowTask SlowTask(TaskCount, FText::FromString("Updating Camera Presets..."));
	SlowTask.MakeDialog();

	if (InApplyChanges)
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			for (const FCameraViewItemModelPtr CurrentItem : InCameraViewItemModels)
			{
				SlowTask.EnterProgressFrame(1, FText::FromString("Updating the camera..."));

				if (CurrentItem->CameraViewItemInfo->CameraPtr.IsValid())
				{
					CameraManager->LoadPresetToCamera(CurrentItem->CameraViewItemInfo->PresetName,CurrentItem->CameraViewItemInfo->CameraPtr.Get());
				}
			}
		}
	}
	else
	{
		for (const FCameraViewItemModelPtr CurrentItem : InCameraViewItemModels)
		{
			SlowTask.EnterProgressFrame(1, FText::FromString("Cleaning preset info in the camera..."));

			if (CurrentItem->CameraViewItemInfo->CameraPtr.IsValid())
			{
				CurrentItem->CameraViewItemInfo->PresetName = TEXT("");
				FCMCameraUtils::RemovePresetNameFromCineCamera(CurrentItem->CameraViewItemInfo->CameraPtr.Get());
			}
		}
	}
}

void SCameraManagerWidget::SpawnLoadPresetWindow()
{
	SetEnabled(false);
	
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FVector2D WindowSize = FVector2D(300.f, 300.f);

	const TSharedRef<SWindow> LoadWindow = SNew(SCMPresetLoadWindow)
		.Title(FText::FromName("Load Preset"))
		.Presets(GetPresetsWithNamesAndDesc())
		.OnPresetSelectedForLoad(this, &SCameraManagerWidget::OnPresetSelectedForLoad)
		.OnPresetDeleteRequested(this,&SCameraManagerWidget::OnPresetDeleteRequested);

	LoadWindow->MoveWindowTo(SlateApp.GetCursorPos() + FVector2D(-WindowSize.X / 2, -WindowSize.Y / 2));
	SubWindow = SlateApp.AddWindow(LoadWindow);
	if (SubWindow.IsValid())
	{
		SubWindow->Resize(FVector2D(WindowSize));
	}
}

void SCameraManagerWidget::SpawnSavePresetWindow()
{
	SetEnabled(false);
	
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FVector2D WindowSize = FVector2D(300.f, 300.f);

	const TSharedRef<SWindow> SaveWindow = SNew(SCMPresetSaveWindow)
		.Title(FText::FromName("Save Preset"))
		.Presets(GetPresetsWithNamesAndDesc())
		.OnPresetSaveRequested(this, &SCameraManagerWidget::OnPresetSaveRequested)
		.OnPresetDeleteRequested(this,&SCameraManagerWidget::OnPresetDeleteRequested);

	SaveWindow->MoveWindowTo(SlateApp.GetCursorPos() + FVector2D(-WindowSize.X / 2, -WindowSize.Y / 2));
	SubWindow = SlateApp.AddWindow(SaveWindow);
	if (SubWindow.IsValid())
	{
		SubWindow->Resize(FVector2D(WindowSize));
	}
}

void SCameraManagerWidget::RenameSelectedCamera() const
{
	if (!TreeViewWidget.IsValid()) { return; }

	TArray<FCameraViewItemModelPtr> SelectedItems = TreeViewWidget->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		//FSlateApplication::Find
		if(TreeViewWidget->WidgetFromItem(SelectedItems[0]).IsValid())
		{
			SelectedItems[0]->CameraViewItemInfo->OnRenameRequested.ExecuteIfBound();
		}
	}
}

bool SCameraManagerWidget::CanItemBeSaved() const
{
	if (!TreeViewWidget.IsValid()) { return false; }

	return TreeViewWidget->GetNumItemsSelected() == 1;
}

bool SCameraManagerWidget::CanItemBeLoad() const
{
	if (!TreeViewWidget.IsValid()) { return false; }

	return TreeViewWidget->GetNumItemsSelected() > 0;
}

bool SCameraManagerWidget::CanItemBeRenamed() const
{
	if (!TreeViewWidget.IsValid()) { return false; }

	return TreeViewWidget->GetNumItemsSelected() == 1;
}

bool SCameraManagerWidget::CanItemBeDeleted() const
{
	if (!TreeViewWidget.IsValid()) { return false; }

	return TreeViewWidget->GetNumItemsSelected() > 0;
}

bool SCameraManagerWidget::CanItemDuplicate() const
{
	if (!TreeViewWidget.IsValid()) { return false; }
	
	return TreeViewWidget->GetNumItemsSelected() > 0 && FLevelEditorActionCallbacks::Duplicate_CanExecute();
}

bool SCameraManagerWidget::CanItemBeFocused() const
{
	if (!TreeViewWidget.IsValid()) { return false; }

	return TreeViewWidget->GetNumItemsSelected() == 1;
}

void SCameraManagerWidget::DeleteSelectedCameras()
{
	const FScopedTransaction Transaction(FText::FromName(TEXT("Delete Elements")));
	
	TArray<FCameraViewItemModelPtr> SelectedItems = TreeViewWidget->GetSelectedItems();

	if(CameraViewItemModels.IsEmpty() || SelectedItems.IsEmpty()){return;}

	TreeViewWidget->ClearSelection();
	
	for (const auto& SelectedItem : SelectedItems)
	{
		// Check if the camera actor is valid
		if(ACineCameraActor* CameraActor = SelectedItem->CameraViewItemInfo->CameraPtr.Get())
		{
			// Destroy the camera actor
			CameraActor->Destroy();
		}

		CameraViewItemModels.Remove(SelectedItem);
	}

	ApplyFilter();
}

void SCameraManagerWidget::DuplicateSelectedCameras()
{
	TArray<AActor*> SelectedCameraActors;
	
	if(TreeViewWidget.IsValid())
	{
		TArray<FCameraViewItemModelPtr> SelectedItems = TreeViewWidget->GetSelectedItems();

		if(!SelectedItems.IsEmpty())
		{
			for(auto CurrentItem : SelectedItems)
			{
				if(CurrentItem->CameraViewItemInfo->CameraPtr.IsValid())
				{
					SelectedCameraActors.Add(CurrentItem->CameraViewItemInfo->CameraPtr.Get());
				}
			}
		}
	}
	
	if(TreeViewWidget.IsValid() && TreeViewWidget->GetNumItemsSelected() > 0)
	{
		if (UEditorActorSubsystem* EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>())
		{
			EditorActorSubsystem->DuplicateActors(SelectedCameraActors,GEditor->GetEditorWorldContext().World(),FVector(0.0f,0.0f,50.0f));

			OnDuplicateEnd();
		}
	}
}

void SCameraManagerWidget::OnDuplicateEnd()
{
	TArray<FName> LocalPresetNames;
	if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
	{
		LocalPresetNames = CameraManager->GetPresetNames();
	}

	if(!LocalPresetNames.IsEmpty())
	{
		for(auto CurrentModel : CameraViewItemModels)
		{
			if(CurrentModel.IsValid() && CurrentModel->CameraViewItemInfo.IsValid() && CurrentModel->CameraViewItemInfo->CameraPtr.IsValid())
			{
				CurrentModel->CameraViewItemInfo->PresetName = FCMCameraUtils::GetPresetNameFromCineCamera(LocalPresetNames, CurrentModel->CameraViewItemInfo->CameraPtr.Get());
			}
		}
	}
}

void SCameraManagerWidget::FocusSelectedCamera() const
{
	if(TreeViewWidget.IsValid() && TreeViewWidget->GetNumItemsSelected() == 1 && TreeViewWidget->GetSelectedItems()[0]->CameraViewItemInfo->CameraPtr.IsValid())
	{
		if (GUnrealEd)
		{
			GUnrealEd->Exec(TreeViewWidget->GetSelectedItems()[0]->CameraViewItemInfo->CameraPtr->GetWorld(), TEXT("CAMERA ALIGN ACTIVEVIEWPORTONLY"));
		}
	}
}

void SCameraManagerWidget::OnPresetSaveRequested(FName InCommittedPresetName, FName InDescription)
{
	auto ResetPresetWindow = [this]()
	{
		if (SubWindow.IsValid())
		{
			SubWindow->RequestDestroyWindow();
			SubWindow.Reset();
			SetEnabled(true);
		}
	};

	if (!InCommittedPresetName.IsNone())
	{
		TArray<FCameraViewItemModelPtr> SelectedCameraViewItemModels = TreeViewWidget->GetSelectedItems();

		if (GEditor && !SelectedCameraViewItemModels.IsEmpty())
		{
			if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
			{
				if (SelectedCameraViewItemModels[0]->CameraViewItemInfo->CameraPtr.IsValid())
				{
					SelectedCameraViewItemModels[0]->CameraViewItemInfo->PresetName = InCommittedPresetName;

					const bool IsPresetAvailable = CameraManager->IsPresetAvailable(InCommittedPresetName);

					if(!IsPresetAvailable)
					{
						PresetNames.Emplace(MakeShared<FString>(InCommittedPresetName.ToString()));
					}
					
					CameraManager->AddCineCameraAsPreset(InCommittedPresetName,InDescription,SelectedCameraViewItemModels[0]->CameraViewItemInfo->CameraPtr.Get(), IsPresetAvailable);

					FCMCameraUtils::InsertNewPresetNameToCineCamera(
						SelectedCameraViewItemModels[0]->CameraViewItemInfo->CameraPtr.Get(), InCommittedPresetName);

					const TArray<FCameraViewItemModelPtr> OtherCamerasWithSamePreset = GetOtherCamerasWithSamePreset(InCommittedPresetName, SelectedCameraViewItemModels[0]);

					if (OtherCamerasWithSamePreset.Num() > 0)
					{
						ResetPresetWindow();

						FString Message;
						if (OtherCamerasWithSamePreset.Num() > 1)
						{
							Message = FString::Printf(
								TEXT("There are %d other cameras with the same preset, do you want to update them?"),
								OtherCamerasWithSamePreset.Num());
						}
						else
						{
							Message = TEXT("There is one other camera with the same preset, do you want to update it?");
						}

						const EAppReturnType::Type UserAnswer = CMDebug::ShowMsgDialog(
							EAppMsgType::YesNo, Message, true);
						UpdateCameraPresets(OtherCamerasWithSamePreset, UserAnswer == EAppReturnType::Yes);
					}

					SelectedCameraViewItemModels[0]->CameraViewItemInfo->OnPresetChanged.ExecuteIfBound();
				}
			}
		}
	}

	ResetPresetWindow();
}

TArray<FName> SCameraManagerWidget::GetPresetsWithNames()
{
	if (GEditor)
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			return CameraManager->GetPresetNames();
		}
	}
	return {};
}

TArray<ACineCameraActor*> SCameraManagerWidget::GetCineCameraActorsFromList()
{
	if(FilteredCameraViewItemModels.IsEmpty()){return {};}

	TArray<ACineCameraActor*> FoundCineCameraActors;
	for(auto CurrentModel : FilteredCameraViewItemModels)
	{
		if(CurrentModel.IsValid() && CurrentModel->CameraViewItemInfo.IsValid() && CurrentModel->CameraViewItemInfo->CameraPtr.IsValid())
		{
			FoundCineCameraActors.Add(CurrentModel->CameraViewItemInfo->CameraPtr.Get());
		}
	}
	return FoundCineCameraActors;
}

TArray<PresetDescPair> SCameraManagerWidget::GetPresetsWithNamesAndDesc()
{
	TArray<PresetDescPair> LocalPresets;

	if (GEditor)
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			return CameraManager->GetPresetNamesWithDesc();
		}
	}
	return {};
}

FCameraViewItemModelPtr SCameraManagerWidget::FindModelByCameraActor(const ACineCameraActor* InCameraActor) const
{
	for (const FCameraViewItemModelPtr& Model : FilteredCameraViewItemModels)
	{
		if (Model->CameraViewItemInfo->CameraPtr.Get() == InCameraActor)
		{
			return Model;
		}
	}
	return {};
}

TArray<FCameraViewItemModelPtr> SCameraManagerWidget::GetOtherCamerasWithSamePreset(const FName& InPresetName,const FCameraViewItemModelPtr& InCameraActor)
{
	TArray<FCameraViewItemModelPtr> OtherCamerasWithSamePreset;

	for (const FCameraViewItemModelPtr& CurrentCameraViewItemModel : CameraViewItemModels)
	{
		if (CurrentCameraViewItemModel->CameraViewItemInfo->PresetName.IsEqual(InPresetName) &&
			CurrentCameraViewItemModel->CameraViewItemInfo->CameraPtr != InCameraActor->CameraViewItemInfo->CameraPtr)
		{
			OtherCamerasWithSamePreset.Add(CurrentCameraViewItemModel);
		}
	}
	return OtherCamerasWithSamePreset;
}

void SCameraManagerWidget::OnPresetSelectedForLoad(FName InSelectedPreset)
{
	if (GEditor && !InSelectedPreset.IsNone())
	{
		if (!TreeViewWidget.IsValid())
		{
			SetEnabled(true);
			return;
		}

		TArray<FCameraViewItemModelPtr> SelectedItems = TreeViewWidget->GetSelectedItems();

		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			for (const FCameraViewItemModelPtr CurrentItem : SelectedItems)
			{
				const FScopedTransaction Transaction(FText::FromName(TEXT("Preset Load")));
				
				if (CurrentItem->CameraViewItemInfo->CameraPtr.IsValid())
				{
					CameraManager->LoadPresetToCamera(InSelectedPreset,CurrentItem->CameraViewItemInfo->CameraPtr.Get());

					CurrentItem->CameraViewItemInfo->PresetName = InSelectedPreset;
				}
			}
		}
	}

	if (SubWindow.IsValid())
	{
		SubWindow->RequestDestroyWindow();
		SubWindow.Reset();
	}
	SetEnabled(true);
}

void SCameraManagerWidget::OnPresetSelectedForLoadInDropDown(FCameraViewItemInfoPtr InSelectedModelPtr)
{
	if(!InSelectedModelPtr.IsValid()){return;}

	//Remove the preset name from the camera 
	if(InSelectedModelPtr->PresetName.IsNone() || InSelectedModelPtr->PresetName.IsEqual("None"))
	{
		const FScopedTransaction Transaction(FText::FromName(TEXT("Preset Remove")));

		if (InSelectedModelPtr->CameraPtr.IsValid())
		{
			InSelectedModelPtr->PresetName = TEXT("");
			FCMCameraUtils::RemovePresetNameFromCineCamera(InSelectedModelPtr->CameraPtr.Get());
		}
	}
	else
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			const FScopedTransaction Transaction(FText::FromName(TEXT("Preset Load")));
			
			if (InSelectedModelPtr->CameraPtr.IsValid())
			{
				CameraManager->LoadPresetToCamera(InSelectedModelPtr->PresetName,InSelectedModelPtr->CameraPtr.Get());
			}
		}
	}
}

void SCameraManagerWidget::OnPresetDeleteRequested(FName InPresetToDelete) const
{
	if (GEditor && !InPresetToDelete.IsNone())
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			if(CameraManager->DeleteAPreset(InPresetToDelete))
			{
				FString PresetTag = CM_PRESET_TAG_TITLE + InPresetToDelete.ToString();

				for(auto CurrentModel : CameraViewItemModels)
				{
					if(CurrentModel->CameraViewItemInfo->PresetName.IsEqual(InPresetToDelete))
					{
						CurrentModel->CameraViewItemInfo->PresetName = TEXT("None");

						CurrentModel->CameraViewItemInfo->CameraPtr->Tags.RemoveSingle(*PresetTag);
						
						CurrentModel->CameraViewItemInfo->OnPresetChanged.ExecuteIfBound();
					}
				}
			}
		}
	}
	
	if(TreeViewWidget.IsValid()){TreeViewWidget->RequestListRefresh();}
}

void SCameraManagerWidget::InitContextMenuCommands()
{
	ToolMenuCommands = MakeShareable(new FUICommandList());
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().SavePreset,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::SpawnSavePresetWindow),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemBeSaved));
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().LoadPreset,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::SpawnLoadPresetWindow),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemBeLoad));
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().ImportPresets,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::RequestImportPresets));
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().ExportPresets,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::RequestExportPresets),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanExport));
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().AutoPilotCamera,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::OnAutoPilotClicked),FCanExecuteAction(),FIsActionChecked::CreateRaw(this,&SCameraManagerWidget::IsAutoPilotChecked));
	ToolMenuCommands->MapAction(FCMCameraContextMenuCommands::Get().ShowDetailsPanel,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::OnShowDetailsClicked),FCanExecuteAction(),FIsActionChecked::CreateRaw(this,&SCameraManagerWidget::IsShowDetailsChecked));
	ToolMenuCommands->MapAction(FGenericCommands::Get().Rename,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::RenameSelectedCamera),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemBeRenamed));
	ToolMenuCommands->MapAction(FGenericCommands::Get().Delete,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::DeleteSelectedCameras),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemBeDeleted));
	ToolMenuCommands->MapAction(FGenericCommands::Get().Duplicate,FExecuteAction::CreateRaw(this,&SCameraManagerWidget::DuplicateSelectedCameras),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemDuplicate));
	ToolMenuCommands->MapAction(FEditorViewportCommands::Get().FocusViewportToSelection,FExecuteAction::CreateStatic(&FLevelEditorActionCallbacks::ExecuteExecCommand, FString( TEXT("CAMERA ALIGN ACTIVEVIEWPORTONLY"))),FCanExecuteAction::CreateRaw(this,&SCameraManagerWidget::CanItemBeFocused));
}		


TSharedPtr<SWidget> SCameraManagerWidget::ConstructCineCameraContextMenu()
{
	FMenuBuilder MenuBuilder(true, ToolMenuCommands);

	MenuBuilder.BeginSection("CMSubsystem", LOCTEXT("CMCameraManagerPresetHeading", "PRESET"));
	{
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().SavePreset);
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().LoadPreset);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("CMSubsystem", LOCTEXT("CMCameraManagerEditHeading", "EDIT"));
	{
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename);
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
		MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
	};	
	MenuBuilder.EndSection();
	
	MenuBuilder.BeginSection("CMSubsystem", LOCTEXT("CMCameraManagerViewHeading", "VIEW"));
	{
		MenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().FocusViewportToSelection);
	}

	return MenuBuilder.MakeWidget();
}

void SCameraManagerWidget::HandleWithItemSelectionTimer()
{
	bIsItemSelectedViaIO = true;
	
	if(FAsyncDelayExecutor::IsTaskInProgress())
	{
		FAsyncDelayExecutor::ResetAndRestart([this]()
		{
			this->HandleOnDelayedElapsed();
		},0.3f);
	}
	// Use the AsyncDelayExecutor to log a message after 5 seconds
	FAsyncDelayExecutor::ExecuteFunctionAfterDelay([this]()
	{
		this->HandleOnDelayedElapsed();
	}, 0.3f);
}

void SCameraManagerWidget::HandleOnDelayedElapsed()
{
	bIsItemSelectedViaIO = false;
}

void SCameraManagerWidget::ShutdownSubWindow()
{
	if (SubWindow.IsValid())
	{
		SubWindow->RequestDestroyWindow();
		SubWindow.Reset();
	}
	if(ImportHandler.IsValid())
	{
		ImportHandler->RequestShutdown(ECMImportFailReason::UserCancelled);
	}
}

FReply SCameraManagerWidget::OnAddCameraClicked()
{
	if(ACineCameraActor* CreatedNewCineCameraActor = FCMCameraUtils::CreateCineCameraOnView())
	{
		const UCMCameraSettings* Config = GetDefault<UCMCameraSettings>();
		if(IsValid(Config) && Config->bAutoPilotCamera)
		{
			OnCameraPilotBtnClicked(CreatedNewCineCameraActor);
		}
	}
	return FReply::Handled();
}

TSharedRef<ITableRow> SCameraManagerWidget::TreeViewGenerateRow(FCameraViewItemModelPtr Item,const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SSCameraViewItemRow, OwnerTable, Item)
			.PresetData(&PresetNames)
			.InFilterText(FilterText)
			.OnPresetSelectedForLoadDropDown(this, &SCameraManagerWidget::OnPresetSelectedForLoadInDropDown);
}

void SCameraManagerWidget::RefreshDetailsWidget() const
{
	TArray<UObject*> SelectedCineCameraActors;

	for (const auto& PaletteItem : TreeViewWidget->GetSelectedItems())
	{
		SelectedCineCameraActors.Add(PaletteItem->CameraViewItemInfo->CameraPtr.Get());
	}

	constexpr bool bForceRefresh = true;
	DetailsWidget->SetObjects(SelectedCineCameraActors, bForceRefresh);
}


bool SCameraManagerWidget::CheckIsSelectionProcessing()
{
	// Get the current time
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

	// Calculate the difference between the current time and the last execution time
	const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - LastExecutionTime).count();
	
	// Update the last execution time
	LastExecutionTime = now;

	// If less than 0.2 seconds have passed since the last execution, return immediately
	return diff < 200;
}


TSharedRef<SWidget> SCameraManagerWidget::GetSettingsButtonContent() const
{
	// Menu should stay open on selection if filters are not being shown
	FMenuBuilder MenuBuilder(true, ToolMenuCommands);

	MenuBuilder.BeginSection("CMPresetSettings", LOCTEXT("CMCameraManagerPresetHeading", "PRESET ACTIONS"));
	{
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().ImportPresets);
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().ExportPresets);
	}
	MenuBuilder.EndSection();
	
	MenuBuilder.BeginSection("CMCamera Settings", LOCTEXT("CMCameraManagerPresetHeading", "CONFIGURATIONS"));
	{
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().AutoPilotCamera);
		MenuBuilder.AddMenuEntry(FCMCameraContextMenuCommands::Get().ShowDetailsPanel);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}



void SCameraManagerWidget::RequestImportPresets()
{
	ImportHandler = MakeUnique<FPResetImportHandler>(FOnPresetImportProcessCompleted::CreateRaw(this, &SCameraManagerWidget::OnPresetImportProcessCompleted));
	ImportHandler->StartImportProcess();
}


void SCameraManagerWidget::OnPresetImportProcessCompleted(ECMImportFailReason::Type InReason)
{
	if(InReason == ECMImportFailReason::None)
	{
		CMDebug::ShowNotifyInfo("Presets imported successfully.");
	}
	else if(InReason == ECMImportFailReason::UnknownError)
	{
		CMDebug::ShowNotifyError("Failed to import presets.");
	}
	else if(InReason == ECMImportFailReason::NoValidData)
	{
		CMDebug::ShowNotifyError("No valid data found in the file.");
	}

	ImportHandler.Reset();
}

void SCameraManagerWidget::RequestExportPresets() 
{
	SpawnExportWindow();
}

void SCameraManagerWidget::SpawnExportWindow()
{
	SetEnabled(false);
	
	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FVector2D WindowSize = FVector2D(500.f, 350.f);

	const TSharedRef<SCMPresetPipelineExportWindow> ExportWindow = SNew(SCMPresetPipelineExportWindow)
		.Title(FText::FromName("Export Presets"))
		.Presets(GetPresetsWithNamesAndDesc())
		.OnPresetExportRequested(this, &SCameraManagerWidget::OnExportRequested);

	ExportWindow->MoveWindowTo(SlateApp.GetCursorPos() + FVector2D(-WindowSize.X / 2, -WindowSize.Y / 2));
	SubWindow = SlateApp.AddWindow(ExportWindow);
	if (SubWindow.IsValid())
	{
		SubWindow->Resize(FVector2D(WindowSize));
	}
	ExportWindow->InitialSetup();
}

void SCameraManagerWidget::OnExportRequested(const TArray<FName>& InSelectedPresets, const FString& InFilePath)
{
	if(!InSelectedPresets.IsEmpty() && !InFilePath.IsEmpty())
	{
		if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			CameraManager->ExportPresets(InSelectedPresets,InFilePath);
		}
	}
	
	if (SubWindow.IsValid())
	{
		SubWindow->RequestDestroyWindow();
		SubWindow.Reset();
	}
	
	SetEnabled(true);
}

bool SCameraManagerWidget::CanExport() const
{
	if (const UCameraManagerSubsystem* CameraManager = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
	{
		return CameraManager->IsAnyPresetAvailable();
	}
	return false;
}

void SCameraManagerWidget::OnShowDetailsClicked() const
{
	UCMCameraSettings* Config = GetMutableDefault<UCMCameraSettings>();

	if(IsValid(Config))
	{
		Config->bShowDetailsPanel = !Config->bShowDetailsPanel;

		DetailsBox->SetVisibility(Config->bShowDetailsPanel ? EVisibility::Visible : EVisibility::Collapsed);
		
		Config->SavePluginConfig();
	}
}

bool SCameraManagerWidget::IsShowDetailsChecked() const
{
	const UCMCameraSettings* Config = GetDefault<UCMCameraSettings>();

	if(IsValid(Config))
	{
		return Config->bShowDetailsPanel;
	}
	return false;
}

void SCameraManagerWidget::OnAutoPilotClicked() const 
{
	UCMCameraSettings* Config = GetMutableDefault<UCMCameraSettings>();
	if(IsValid(Config))
	{
		Config->bAutoPilotCamera = !Config->bAutoPilotCamera;
		Config->SavePluginConfig();
	}
}

bool SCameraManagerWidget::IsAutoPilotChecked() const
{
	const UCMCameraSettings* Config = GetDefault<UCMCameraSettings>();

	if(IsValid(Config))
	{
		return Config->bAutoPilotCamera;
	}
	return false;
}

void SCameraManagerWidget::TreeViewGetChildren(FCameraViewItemModelPtr Item,TArray<FCameraViewItemModelPtr>& OutChildren)
{
	// No Children
}







void SCameraManagerWidget::HighlightPilotedCamera(const ACineCameraActor* InCameraActor)
{
	CHECK_AND_RETURN_WITH_ERROR(!TreeViewWidget.IsValid(), TEXT("TreeViewWidget is not valid!"));

	TreeViewWidget->ClearSelection();
	TreeViewWidget->ClearHighlightedItems();

	for(auto CurrentModel : CameraViewItemModels)
	{
		if(CurrentModel->CameraViewItemInfo->CameraPtr.Get() == InCameraActor)
		{
			TreeViewWidget->SetItemHighlighted(CurrentModel, true);
			TreeViewWidget->RequestNavigateToItem(CurrentModel);
			break;
		}
	}	
}

#undef LOCTEXT_NAMESPACE
