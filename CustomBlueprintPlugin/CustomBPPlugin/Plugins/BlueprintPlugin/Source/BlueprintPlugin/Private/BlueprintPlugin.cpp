// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlueprintPlugin.h"
#include "UObject/Class.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "MultiBox/MultiBoxBuilder.h"
#include "AssetEditorManager.h"
#include "SSCSEditor.h"
#include "Camera/CameraComponent.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "FBlueprintPluginModule"

void FBlueprintPluginModule::CustomBlueprintOperations(TArray<FAssetData> SelectedAssets) {
	UE_LOG(LogTemp, Log, TEXT("Custom Blueprint Operations"));
	for (FAssetData& Asset : SelectedAssets) {
		UObject* Object = Asset.GetAsset();
		UBlueprint* BP = Cast<UBlueprint>(Object);

		FAssetEditorManager::Get().OpenEditorForAsset(BP);
		IAssetEditorInstance* AssetEditor = FAssetEditorManager::Get().FindEditorForAsset(BP, true);
		FBlueprintEditor* BlueprintEditorPtr = (FBlueprintEditor*)AssetEditor;
		TSharedPtr<SSCSEditor> Editor = BlueprintEditorPtr->GetSCSEditor();

		UClass* NewCameraComponentClass = UCameraComponent::StaticClass();
		UActorComponent* NewCameraComponent = Editor->AddNewComponent(NewCameraComponentClass, NULL);
		FSCSEditorTreeNodePtrType NewCameraNodePtr = Editor->GetNodeFromActorComponent(NewCameraComponent);

		NewCameraNodePtr->OnCompleteRename(FText::FromString("ChaseCamera"));
		FKismetEditorUtilities::CompileBlueprint(BP);
		FAssetEditorManager::Get().CloseAllEditorsForAsset(BP);
	}
}

void FBlueprintPluginModule::CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets) {

	bool allIsCarBP = true;
	for (FAssetData& Asset : SelectedAssets) {
		allIsCarBP &= (SelectedAssets[0].AssetClass == "Blueprint");
	}
	if (allIsCarBP)
	{
		MenuBuilder.AddMenuEntry
		(
			LOCTEXT("Custom Blueprint Operations", "Custom Blueprint Operations"),
			LOCTEXT("Custom Blueprint Operations", "Custom Blueprint Operations"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FBlueprintPluginModule::CustomBlueprintOperations, SelectedAssets))
		);
	}
}


TSharedRef<FExtender> FBlueprintPluginModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets) {
	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FBlueprintPluginModule::CreateContentBrowserAssetMenu, SelectedAssets));
	return Extender;
}

void FBlueprintPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!IsRunningCommandlet() && !IsRunningGame() && FSlateApplication::IsInitialized())
	{
		// Register content browser hook
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FBlueprintPluginModule::OnExtendContentBrowserAssetSelectionMenu));
	}
}

void FBlueprintPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintPluginModule, BlueprintPlugin)