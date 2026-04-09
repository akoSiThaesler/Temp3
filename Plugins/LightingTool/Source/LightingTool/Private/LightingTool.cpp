// Copyright 2024 Leartes Studios. All Rights Reserved.


#include "LightingTool.h"
#include "CameraManager.h"
#include "CMSubsystem.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"
#include "LTToolAssetData.h"
#include "LTToolMenuCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LTToolStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SComboButton.h"

#define LOCTEXT_NAMESPACE "FLightingToolModule"

FName FLightingToolModule::ALToolTabID;
FName FLightingToolModule::LTToolTabID;
FName FLightingToolModule::HDRIToolTabID;
FName FLightingToolModule::CMToolTabID;
FName FLightingToolModule::LRToolTabID;

void FLightingToolModule::StartupModule()
{
	FLTToolStyle::InitializeToolStyle();
	FLTToolMenuCommands::Register();
	InitToolMenuCommands();
	SetupPluginToolbarEntry();
}

void FLightingToolModule::ShutdownModule()
{
	FLTToolStyle::ShutDownStyle();
}

void FLightingToolModule::InitToolMenuCommands()
{
	ToolMenuCommands = MakeShareable(new FUICommandList());
	
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().OpenAutoLightMapTool,FExecuteAction::CreateRaw(this,&FLightingToolModule::ToggleAutoLightMapWindow));
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().OpenLightsTool,FExecuteAction::CreateRaw(this,&FLightingToolModule::ToggleLightsToolWindow));
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().OpenCameraManagerTool,FExecuteAction::CreateRaw(this,&FLightingToolModule::ToggleCameraManager));
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().OpenHDRIManagerTool,FExecuteAction::CreateRaw(this,&FLightingToolModule::ToggleHDRIManagerWindow));
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().OpenLightRenderSettingsTool,FExecuteAction::CreateRaw(this,&FLightingToolModule::ToggleLightRenderSettingsTolWindow));
	ToolMenuCommands->MapAction(FLTToolMenuCommands::Get().LaunchHelp,FExecuteAction::CreateStatic(&FLightingToolModule::LaunchHelp));
}

void FLightingToolModule::SetupPluginToolbarEntry()
{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    	TSharedPtr<FExtender> Extenders = LevelEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders();
    	
    	const TSharedPtr<FExtender> ODToolExtender = MakeShareable(new FExtender);
    	ODToolExtender->AddToolBarExtension("Play", EExtensionHook::After, NULL, FToolBarExtensionDelegate::CreateRaw(this, &FLightingToolModule::AddToolbarExtension));
    	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ODToolExtender);
}

TSharedRef<SWidget> FLightingToolModule::CreateToolMenu() const
{
	FMenuBuilder MenuBuilder(true, ToolMenuCommands);
	MenuBuilder.BeginSection("Tools");
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().OpenLightsTool.ToSharedRef());
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().OpenAutoLightMapTool.ToSharedRef());
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().OpenCameraManagerTool.ToSharedRef());
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().OpenHDRIManagerTool.ToSharedRef());
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().OpenLightRenderSettingsTool.ToSharedRef());
	MenuBuilder.AddSeparator();
	MenuBuilder.AddMenuEntry(FLTToolMenuCommands::Get().LaunchHelp.ToSharedRef());
	MenuBuilder.EndSection();
	//MenuBuilder.AddSubMenu()
	return MenuBuilder.MakeWidget();	
}

void FLightingToolModule::AddToolbarExtension(FToolBarBuilder& ToolBarBuilder)
{
	const auto UnitsWidgetLambda = [this]() -> TSharedRef<SWidget> {
		return
		SNew(SComboButton)
		.OnGetMenuContent(FOnGetContent::CreateRaw(this, &FLightingToolModule::CreateToolMenu))
		.ComboButtonStyle(FAppStyle::Get(),"SimpleComboButton")
		.ButtonContent()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(0,0,2,0)
			[
				SNew(SBox)
				.WidthOverride(24)
				.HeightOverride(24)
				[
					SNew(SImage)
					.Image(FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.ToolbarIcon").GetIcon())
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(0)
			[
				SNew(SBox)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.TextStyle(FAppStyle::Get(), "Menu.Label")
				.Text(LOCTEXT("LightinTool", "Ultimate Lighting Tool"))
				]
			]
		];
	};
	
	ToolBarBuilder.AddWidget(UnitsWidgetLambda());
}

void FLightingToolModule::ToggleAutoLightMapWindow()
{
	if (!GEditor){return;}
	
	if(	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
	{
		if(EditorUtilitySubsystem->DoesTabExist(ALToolTabID))
		{
			EditorUtilitySubsystem->CloseTabByID(ALToolTabID);
		}
		else
		{
			if(UObject* ToolWindowObject = LTToolAssetData::LightMapToolWindowPath.TryLoad())
			{
				if(const auto ToolWindowBlueprint = Cast<UEditorUtilityWidgetBlueprint>(ToolWindowObject))
				{
					EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(ToolWindowBlueprint,ALToolTabID);

					//Set Tab Icon
					const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
					const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
					if (const TSharedPtr<SDockTab> FoundTab = LevelEditorTabManager->FindExistingLiveTab(ALToolTabID))
					{
						FoundTab->SetTabIcon(FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightMapIcon").GetIcon());
					}
				}
			}
		}
	}
}

void FLightingToolModule::ToggleLightsToolWindow()
{
	if (!GEditor){return;}

	if(	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
	{
		if(EditorUtilitySubsystem->DoesTabExist(LTToolTabID))
		{
			EditorUtilitySubsystem->CloseTabByID(LTToolTabID);
		}
		else
		{
			if(UObject* ToolWindowObject = LTToolAssetData::LightsToolToolWindowPath.TryLoad())
			{
				if(const auto ToolWindowBlueprint = Cast<UEditorUtilityWidgetBlueprint>(ToolWindowObject))
				{
					EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(ToolWindowBlueprint,LTToolTabID);

					//Set Tab Icon
					const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
					const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
					if (const TSharedPtr<SDockTab> FoundTab = LevelEditorTabManager->FindExistingLiveTab(LTToolTabID))
					{
						FoundTab->SetTabIcon(FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightsToolIcon").GetIcon());
					}
				}
			}
		}
	}
}

void FLightingToolModule::ToggleHDRIManagerWindow()
{
	if(	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
	{
		if(EditorUtilitySubsystem->DoesTabExist(HDRIToolTabID))
		{
			EditorUtilitySubsystem->CloseTabByID(HDRIToolTabID);
		}
		else
		{
			if(UObject* ToolWindowObject = LTToolAssetData::HDRIManagerToolWindowPath.TryLoad())
			{
				if(const auto ToolWindowBlueprint = Cast<UEditorUtilityWidgetBlueprint>(ToolWindowObject))
				{
					EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(ToolWindowBlueprint,HDRIToolTabID);

					//Set Tab Icon
					const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
					const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
					if (const TSharedPtr<SDockTab> FoundTab = LevelEditorTabManager->FindExistingLiveTab(HDRIToolTabID))
					{
						FoundTab->SetTabIcon(FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.HDRIIcon").GetIcon());
					}
				}
			}
		}
	}
}


void FLightingToolModule::ToggleCameraManager()
{	
	if(FCameraManagerModule* CameraManager = &FModuleManager::LoadModuleChecked<FCameraManagerModule>(TEXT("CameraManager")))
	{
		if(UCameraManagerSubsystem* CameraManagerSubsystem = GEditor->GetEditorSubsystem<UCameraManagerSubsystem>())
		{
			CameraManagerSubsystem->ToggleCameraManagerToolTab();
		}
	}
}

void FLightingToolModule::ToggleLightRenderSettingsTolWindow()
{
	if(	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>())
	{
		if(EditorUtilitySubsystem->DoesTabExist(LRToolTabID))
		{
			EditorUtilitySubsystem->CloseTabByID(LRToolTabID);
		}
		else
		{
			if(UObject* ToolWindowObject = LTToolAssetData::LightRenderSettingsToolWindowPath.TryLoad())
			{
				if(const auto ToolWindowBlueprint = Cast<UEditorUtilityWidgetBlueprint>(ToolWindowObject))
				{
					EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID(ToolWindowBlueprint,LRToolTabID);

					//Set Tab Icon
					const FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
					const TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
					if (const TSharedPtr<SDockTab> FoundTab = LevelEditorTabManager->FindExistingLiveTab(LRToolTabID))
					{
						FoundTab->SetTabIcon(FSlateIcon(FLTToolStyle::GetToolStyleName(),"LightingTool.LightRenderSettingsIcon").GetIcon());
					}
				}
			}
		}
	}
}

void FLightingToolModule::LaunchHelp()
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("LightingTool"));
	if (Plugin.IsValid())
	{
		const FPluginDescriptor &Descriptor = Plugin->GetDescriptor();
		FPlatformProcess::LaunchURL(*Descriptor.DocsURL,nullptr,nullptr);
	}
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLightingToolModule, LightingTool)