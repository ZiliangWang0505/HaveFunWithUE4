// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AnimBluprintPlugin.h"
#include "UObject/Class.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "Slate/Public/Framework/Application/SlateApplication.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "SSCSEditor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "FileHelpers.h"
#include "Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/Classes/EdGraph/EdGraph.h"
#include "VehicleAnimInstance.h"
#include "AnimGraphNode_WheelHandler.h"

#define LOCTEXT_NAMESPACE "FAnimBluprintPluginModule"

void FAnimBluprintPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!IsRunningCommandlet() && !IsRunningGame() && FSlateApplication::IsInitialized())
	{
		// Register content browser hook
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBAssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBAssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FAnimBluprintPluginModule::OnExtendContentBrowserAssetSelectionMenu));
	}
}

void FAnimBluprintPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

UAnimBlueprint* FAnimBluprintPluginModule::CreateCustomAnimationBlueprint(USkeleton* TargetSkeleton, UClass* ParentClassToUse, const FString& AnimBlueprintPath) {
	TArray<UPackage*> PackagesToSave;

	UPackage* Package = FindPackage(nullptr, *AnimBlueprintPath);
	if (Package)
	{
		UAnimBlueprint* NewBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBlueprintPath);
		return NewBP;
	}

	Package = CreatePackage(nullptr, *AnimBlueprintPath);
	UObject* InParent = Cast<UObject>(Package);
	UAnimBlueprint* NewBP = CastChecked<UAnimBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClassToUse, InParent, *FPaths::GetBaseFilename(AnimBlueprintPath), BPTYPE_Normal, UAnimBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), FName("ContentBrowserNewAsset")));
	
	NewBP->TargetSkeleton = TargetSkeleton;
	// Because the BP itself didn't have the skeleton set when the initial compile occured, it's not set on the generated classes either
	if (UAnimBlueprintGeneratedClass* TypedNewClass = Cast<UAnimBlueprintGeneratedClass>(NewBP->GeneratedClass))
	{
		TypedNewClass->TargetSkeleton = TargetSkeleton;
	}
	if (UAnimBlueprintGeneratedClass* TypedNewClass_SKEL = Cast<UAnimBlueprintGeneratedClass>(NewBP->SkeletonGeneratedClass))
	{
		TypedNewClass_SKEL->TargetSkeleton = TargetSkeleton;
	}

	UEdGraph* AnimGraph = NewBP->FunctionGraphs[0];
	const UEdGraphSchema* Schema = AnimGraph->GetSchema();
	UEdGraphPin* ResultPin = AnimGraph->Nodes[0]->Pins[0];

	FGraphNodeCreator<UAnimGraphNode_WheelHandler> WheelHandlerNodeCreator(*AnimGraph);
	UAnimGraphNode_WheelHandler*  WheelHandlerNode = WheelHandlerNodeCreator.CreateNode();
	WheelHandlerNode->NodePosX = -400;
	WheelHandlerNodeCreator.Finalize();
	UEdGraphPin* ComponentPosePin = WheelHandlerNode->Pins[4];

	Schema->CreateAutomaticConversionNodeAndConnections(ResultPin, ComponentPosePin);

	FKismetEditorUtilities::CompileBlueprint(NewBP);
	Package->MarkPackageDirty();
	PackagesToSave.Add(Package);
	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, true);
	return NewBP;
}

void FAnimBluprintPluginModule::CustomAnimBlueprintOperations(TArray<FAssetData> SelectedAssets) {
	UE_LOG(LogTemp, Log, TEXT("Custom Anim Blueprint Operations"));
	for (FAssetData& Asset : SelectedAssets) {
		UObject* Object = Asset.GetAsset();
		UBlueprint* BP = Cast<UBlueprint>(Object);

		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(BP);
		IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(BP, true);
		FBlueprintEditor* BlueprintEditorPtr = (FBlueprintEditor*)AssetEditor;
		TSharedPtr<SSCSEditor> SCSEditor = BlueprintEditorPtr->GetSCSEditor();
		FSCSEditorTreeNodePtrType SceneRootNode = SCSEditor->GetSceneRootNode();
		UActorComponent* ComponentTemplate = SceneRootNode->GetComponentTemplate();
		USkeletalMeshComponent* SkeletalMeshComponent = (USkeletalMeshComponent*)ComponentTemplate;
		if (SkeletalMeshComponent)
		{
			const FString BPName = BP->GetName();
			const FString BPPathName = BP->GetPathName();

			const FString ABPPathName = FPaths::Combine(FPaths::GetPath(BPPathName), FString::Printf(TEXT("ABP_%s"), *BPName));
			USkeleton* TargetSkeleton = SkeletalMeshComponent->SkeletalMesh->Skeleton;
			UClass* ParentClassToUse = UVehicleAnimInstance::StaticClass();
			UAnimBlueprint* CustomAnimBlueprint = CreateCustomAnimationBlueprint(TargetSkeleton, ParentClassToUse, ABPPathName);

			SkeletalMeshComponent->SetAnimClass(CustomAnimBlueprint->GeneratedClass);
		}
		FKismetEditorUtilities::CompileBlueprint(BP);
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->CloseAllEditorsForAsset(BP);
	}
}

void FAnimBluprintPluginModule::CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets) {

	bool allIsCarBP = true;
	for (FAssetData& Asset : SelectedAssets) {
		allIsCarBP &= (Asset.AssetClass == "Blueprint");
	}
	if (allIsCarBP)
	{
		MenuBuilder.AddMenuEntry
		(
			LOCTEXT("Custom Anim Blueprint Operations", "Custom Anim Blueprint Operations"),
			LOCTEXT("Custom Anim Blueprint Operations", "Custom Anim Blueprint Operations"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FAnimBluprintPluginModule::CustomAnimBlueprintOperations, SelectedAssets))
		);
	}
}

TSharedRef<FExtender> FAnimBluprintPluginModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets) {
	TSharedRef<FExtender> Extender(new FExtender());
	Extender->AddMenuExtension(
		"AssetContextAdvancedActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FAnimBluprintPluginModule::CreateContentBrowserAssetMenu, SelectedAssets));
	return Extender;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAnimBluprintPluginModule, AnimBluprintPlugin)