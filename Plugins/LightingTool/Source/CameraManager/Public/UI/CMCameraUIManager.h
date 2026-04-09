// Copyright 2024 Leartes Studios. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STreeView.h"
#include "CMCameraViewSlate.h"
#include "CMMisc.h"
#include "CMPresetSelectionWindow.h"
#include "UnrealEdMisc.h"
#include "CMCameraDefinitions.h"
#include <chrono>

#include "Misc/ITransaction.h"
#include "Misc/TextFilter.h"

class SComboButton;
class SLevelViewport;
class SCameraManagerWidget;
class SCMPresetSaveWindow;
class SCMPresetLoadWindow;
class ACineCameraActor;

DECLARE_DELEGATE_OneParam(FOnSelectionDelay, TWeakObjectPtr<ACineCameraActor>)

namespace ECMQuerySortMode
{
    enum Type
    {
        ByLabel,
        ByPresetName,
        ByLockState
    };
}


class FCameraUIManagerImp : public TSharedFromThis<FCameraUIManagerImp>
{
public:
    void Initialize();
    void Shutdown();
    
    void ToggleCameraManagerTab();

    void SetupManagerDelegates();
    void RemoveManagerDelegates() const;

    void SetupUIDelegates();
    void RemoveUIDelegates() const;
    
    void SetPilotStateChangedViaTool(bool InState);
    bool GetPilotStateChangedViaTool() const;


private:
    void OnLevelActorAdded(AActor* InAddedActor) const;
    void OnLevelActorDeleted(AActor* InDeletedActor) const;
    void OnTransactionStateChanged(const FTransactionContext& TransactionContext, ETransactionStateEventType TransactionState) const;
    void OnActorSelectionChanged(const TArray<UObject*>& InActors, bool bIsSelected) const;
    void OnMapChanged(UWorld* World, EMapChangeType MapChangeType) const;
    
    TWeakPtr<SLevelViewport> ActiveViewport;
    TWeakObjectPtr<AActor> ActiveActorLock;
    FTSTicker::FDelegateHandle TickHandle;
    bool OnTick(float DeltaTime);

    void OnActiveViewportChanged(TSharedPtr<IAssetViewport> OldViewport, TSharedPtr<IAssetViewport> NewViewport);
    void CheckForViewportCameraPilotChange();

    
    TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
    
    //UI Refs
    TSharedPtr<SDockTab> CameraManagerMainDock;
    
    TSharedPtr<SCameraManagerWidget> CameraManagerWidget;


    void InitCameraLevelCommands();


    bool bIsPilotStateChangedViaTool = false;


//Logic
    void CreateNewCameraOnView();
    void OnNextCameraTriggered();
    void OnPreviousCameraTriggered();
    void OnEjectPilotMode();
    void OnToggleLockState();
    void GoToNextCamera(bool bInShouldNext);
    void UpdateCameraIndex(bool bInShouldNext, int32 CameraCount);
    static int32 GetActiveCameraIndex(const TSharedPtr<SLevelViewport>& ActiveViewport, const TArray<ACineCameraActor*>& CameraActors);

public:
    void PilotTheCamera(ACineCameraActor* InCineCamera);

private:
    
#pragma region  InputPreProcessor
    
    void ActivateInputProcessor();
    void DeactivateInputProcessor();
    
    TSharedPtr<class FCMKeyInputPreProcessor> InputProcessor;
	
protected:
    bool HandleKeySelected(const FKeyEvent& InKey) const;
private:
    
    int32 LastCineCameraIndex = 0;

#pragma endregion  InputPreProcessor
    
    TSharedPtr<class FUICommandList> CameraLevelCommands;
    
};

class FCameraUIManager
{
public:
    static void Initialize();
    static void Shutdown();

    static TSharedPtr<FCameraUIManagerImp> ManagerImpInstance;
};

class SCameraManagerWidget : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCameraManagerWidget) {}
    SLATE_END_ARGS()
    
    void Construct(const FArguments& InArgs);

    virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;

    EColumnSortMode::Type GetIDSortMode(ECMQuerySortMode::Type InColumn) const;
    void OnSortByChanged(EColumnSortPriority::Type SortPriority, const FName& ColumnName,EColumnSortMode::Type NewSortMode);
    void ReOrderCameraList();
    
    void AddNewlyAddedCineCameraToTheList(ACineCameraActor* InAddedCamera);
    void RemoveDeletedCineCameraFromList(const ACineCameraActor* InDeletedCamera);
    void OnDuplicateEnd();
    void OnCameraPilotStateChanged();
    void OnEjectPilotMode(const ACineCameraActor* InPilotedCamera);
    void HighlightPilotedCamera(const ACineCameraActor* InCameraActor);
    TArray<ACineCameraActor*> GetCineCameraActorsFromList();
    void OnCameraLockStateChanged(ACineCameraActor* InCineCameraActor, bool InNewState);
    void RequestUpdateCameraPilotStates();
    void RequestCameraUIRegenerate();
    void LevelCameraSelectionChanged();
    void Shutdown();
private:
    
    std::chrono::steady_clock::time_point LastExecutionTime = std::chrono::steady_clock::now();
    TSharedPtr<STreeView<FCameraViewItemModelPtr>> TreeViewWidget;
    TSharedPtr<class IDetailsView> DetailsWidget;
    TSharedPtr<class SBorder> DetailTitleBox;
    TSharedPtr<SComboButton> SettingsButton;
    TSharedPtr<SPanel> DetailsBox;
    TSharedPtr<class FUICommandList> ToolMenuCommands;
    TArray<FCameraViewItemModelPtr> CameraViewItemModels;
    TSharedPtr<SWindow> SubWindow;

    float SplitterValue = 1.0f;

    bool bIsItemSelectedViaLevel = false;
    bool bIsItemSelectedViaIO = false;

    TArray<TSharedPtr<FString>> PresetNames;
    
    //Recreate CameraViewItemModels but not refresh the UI (for that please use the TreeViewWidget->RequestTreeRefresh())
    void RefreshCameraInfos();
    void RefreshPresetNames();DECLARE_DELEGATE_TwoParams(FOnPresetImported, FName /*PresetName*/, bool /*bIsOverridden*/)
    void OnPresetImported(FName InPresetName, bool bInIsOverridden);
    void RefreshCameraPilotStates();
    FCameraViewItemModelPtr CreateNewItemFromCamera(const TArray<FName>& InPresetNames, bool InPilotState, ACineCameraActor* InCineCameraActor);
    void OnCameraPilotBtnClicked(TWeakObjectPtr<ACineCameraActor> InCameraPtr);
    void OnPresetSelectionChanged(TSharedPtr<FCameraViewItemModel> SelectedItem, ESelectInfo::Type SelectInfo);
    void OnCameraItemDoubleClicked(FCameraViewItemModelPtr InItem);
    void UpdateCameraPresets(TArray<FCameraViewItemModelPtr> InCameraViewItemModels, bool InApplyChanges);
    void SpawnLoadPresetWindow();
    void SpawnSavePresetWindow();
    void RenameSelectedCamera() const;
    bool CanItemBeSaved() const;
    bool CanItemBeLoad() const;
    bool CanItemBeRenamed() const;
    bool CanItemBeDeleted() const;
    bool CanItemDuplicate() const;
    bool CanItemBeFocused() const;
    void DeleteSelectedCameras();
    void DuplicateSelectedCameras();
    void FocusSelectedCamera() const;
    void OnPresetSaveRequested(FName InCommittedPresetName, FName InDescription);
    static TArray<FName> GetPresetsWithNames();
    static TArray<PresetDescPair> GetPresetsWithNamesAndDesc();
    FCameraViewItemModelPtr FindModelByCameraActor(const ACineCameraActor* InCameraActor) const;
    TArray<FCameraViewItemModelPtr> GetOtherCamerasWithSamePreset(const FName& InPresetName, const FCameraViewItemModelPtr& InCameraActor);
    void OnPresetSelectedForLoad(FName InSelectedPreset);
    void OnPresetSelectedForLoadInDropDown(FCameraViewItemInfoPtr InSelectedModelPtr);
    void OnPresetDeleteRequested(FName InPresetToDelete) const;
    void InitContextMenuCommands();
    TSharedPtr<SWidget> ConstructCineCameraContextMenu();
    void HandleWithItemSelectionTimer();
    void HandleOnDelayedElapsed();
    void RefreshDetailsWidget() const;
    bool CheckIsSelectionProcessing();
    void ShutdownSubWindow();
    FReply OnAddCameraClicked();
    TSharedRef<SWidget> GetSettingsButtonContent() const;

    void RequestImportPresets();
    TUniquePtr<class FPResetImportHandler> ImportHandler;
    void OnPresetImportProcessCompleted(ECMImportFailReason::Type InReason);
    
    void RequestExportPresets();
    void SpawnExportWindow();

    void OnExportRequested(const TArray<FName>& InSelectedPresets, const FString& InFilePath);
    FORCEINLINE bool CanExport() const;

    void OnShowDetailsClicked() const;
    FORCEINLINE bool IsShowDetailsChecked() const;

    void OnAutoPilotClicked() const;
    FORCEINLINE bool IsAutoPilotChecked() const;
    
    TSharedRef<ITableRow> TreeViewGenerateRow(FCameraViewItemModelPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
    void TreeViewGetChildren(FCameraViewItemModelPtr Item, TArray<FCameraViewItemModelPtr>& OutChildren);

    ECMQuerySortMode::Type SortBy = ECMQuerySortMode::ByLabel;
    
    EColumnSortMode::Type SortDirection = EColumnSortMode::Type::Descending;
    
    typedef TTextFilter<const FCameraViewItemModelPtr&> FCollectionItemTextFilter;
    TSharedPtr<FCollectionItemTextFilter> CollectionItemTextFilter;
    
    TArray<FCameraViewItemModelPtr> FilteredCameraViewItemModels;
    void HandleItemToStringArray( const FCameraViewItemModelPtr& GroupOrStatNodePtr, TArray< FString >& out_SearchStrings ) const;

    TSharedPtr<FString> FilterText;
    void OnFilterTextChanged(const FText& InFilterText);
    void ApplyFilter();
    
    void SetupDelegates();
        
};


