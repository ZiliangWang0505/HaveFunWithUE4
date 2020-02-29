// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Slate/Public/Framework/MultiBox/MultiBoxBuilder.h"
#include "AssetData.h"
#include "Engine/SkeletalMesh.h"

class FAnimBluprintPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CustomAnimBlueprintOperations(TArray<FAssetData> SelectedAssets);
	UAnimBlueprint* CreateCustomAnimationBlueprint(USkeleton* TargetSkeleton, UClass* ParentClassToUse, const FString& AnimBlueprintPath);
	void CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
};
