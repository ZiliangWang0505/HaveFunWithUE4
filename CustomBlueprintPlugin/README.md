# 蓝图资源插件
创建一个Editor插件，执行对蓝图资源的自定义操作。
## 1.创建项目
启动 **虚幻编辑器**。**项目浏览器** 启动时，点击  **新建项目（NewProject）** 选项卡，选择 **C++** 选项卡，然后选择**基础代码（Basic Code）** 模板。可以启用 **初学者内容包（Starter Content）**，为此项目选择首选的 **保存位置（Save Location）** 和 **名称（Name）**，然后点击 **创建项目（Create Project）**。
![创建项目](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_1.png?raw=true)
## 2.创建插件
在编辑器中，选择 **编辑（Edit）**-> **插件（Plugin）**，打开 **插件编辑界面**。
![插件编辑界面](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_2-1.png?raw=true)
点击 **新建插件（New Plugin）** 按钮，选择要创建的插件类型为 **Blank**，并设置基本参数。
![新建插件界面](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_2-2.png?raw=true)
## 3.插件文件结构
插件生成后打开项目工程，可以看到UE4为插件生成了四个文件：BlueprintPlugin.uplugin、BlueprintPlugin.Build.cs、BlueprintPlugin.h、BlueprintPlugin.cpp。
![项目工程](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_3-1.png?raw=true)

###  BlueprintPlugin.uplugin
```
{
	"FileVersion": 3,
	"Version": 1,
	"VersionName": "1.0",
	"FriendlyName": "BlueprintPlugin",
	"Description": "custom blueprint plugin",
	"Category": "Other",
	"CreatedBy": "Ziliang Wang",
	"CreatedByURL": "",
	"DocsURL": "",
	"MarketplaceURL": "",
	"SupportURL": "",
	"CanContainContent": true,
	"IsBetaVersion": false,
	"Installed": false,
	"Modules": [
		{
			"Name": "BlueprintPlugin",
			"Type": "Runtime",
			"LoadingPhase": "Default"
		}
	]
}
```
 BlueprintPlugin.uplugin为 **插件描述文件**，插件描述文件详细说明可以参考UE4官方文档 [插件|虚幻引擎文档](https://docs.unrealengine.com/zh-CN/Programming/Plugins/index.html) 。这里将插件的类型 `"Type"` 由 `"Runtime"` 修改为 `"Editor"`。`"LoadingPhase"` 由 `"Default"` 修改为 `"PreDefault"`。

### BlueprintPlugin.Build.cs
```C#
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintPlugin : ModuleRules
{
	public BlueprintPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
```
BlueprintPlugin.Build.cs为插件模块构建文件，关于模块的介绍可以参考UE4官方文档 [模块|虚幻引擎文档](https://docs.unrealengine.com/zh-CN/Programming/BuildTools/UnrealBuildTool/ModuleFiles/index.html)。

### BlueprintPlugin.h
```C++
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBlueprintPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
```
BlueprintPlugin.h 插件C++头文件。

### BlueprintPlugin.cpp
```C++
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlueprintPlugin.h"

#define LOCTEXT_NAMESPACE "FBlueprintPluginModule"

void FBlueprintPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FBlueprintPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlueprintPluginModule, BlueprintPlugin)
```
BlueprintPlugin.cpp 插件C++源文件。

## 4.编辑插件
### 1.创建Menu

在 BlueprintPlugin.h 添加三个函数，主要用于构建插件的UI并执行相应的逻辑。函数`OnExtendContentBrowserAssetSelectionMenu`和`CreateContentBrowserAssetMenu`为UI响应函数。函数`CustomBlueprintOperations`为自定义的蓝图操作。

BlueprintPlugin.h 
```C++
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AssetData.h"

class FBlueprintPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CustomBlueprintOperations(TArray<FAssetData> SelectedAssets);
	void CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
};
```
BlueprintPlugin.cpp
```C++
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "BlueprintPlugin.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "MultiBox/MultiBoxBuilder.h"


#define LOCTEXT_NAMESPACE "FBlueprintPluginModule"

void FBlueprintPluginModule::CustomBlueprintOperations(TArray<FAssetData> SelectedAssets) {
	UE_LOG(LogTemp, Log, TEXT("Custom Blueprint Operations"));
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
```
编译并运行项目，在 **ContentBrowser** 中选择 **Blueprint** 资源文件，右键并选择 **Asset Actions**，在弹出的菜单栏中可以看到刚才定义的插件 **Custom Blueprint Operations**。
![插件Menu](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_4-1.png?raw=true)

### 2.自定义蓝图操作
在上一步中，已经完成了插件UI部分的构建，接下来完成针对蓝图资源的自定义操作的代码。在开始之前，首先修改`BlueprintPlugin.Build.cs`文件，将需要的模块加入插件的构建过程。

```C#
PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "UnrealEd",
                "Kismet",
				// ... add private dependencies that you statically link with here ...	
            }
            );
```

按如下方式修改`CustomBlueprintOperations`函数。针对蓝图资源的修改需要通过代码打开 **蓝图编辑器**。这里的代码首先将资源加载为蓝图类，之后用代码调用 **蓝图编辑器** 打开对应的蓝图资源，并给该蓝图类添加了一个Camera组件。
```C++
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
```
修改代码后，再次编译并运行项目，在 **ContentBrowser** 中选择 **Blueprint** 资源文件，右键并选择 **Asset Actions**，在弹出的菜单栏中可以看到刚才定义的插件 **Custom Blueprint Operations**。之后双击打开刚才操作的蓝图资源，可以看到该蓝图类已经添加了对应的Camera组件。
![添加Camera组件](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomBlueprintPlugin/Images/BlueprintPlugin_4-3.png?raw=true)

## 5.总结
最终，整个工程的代码如下。
###  BlueprintPlugin.uplugin
```
{
	"FileVersion": 3,
	"Version": 1,
	"VersionName": "1.0",
	"FriendlyName": "BlueprintPlugin",
	"Description": "custom blueprint plugin",
	"Category": "Other",
	"CreatedBy": "Ziliang Wang",
	"CreatedByURL": "",
	"DocsURL": "",
	"MarketplaceURL": "",
	"SupportURL": "",
	"CanContainContent": true,
	"IsBetaVersion": false,
	"Installed": false,
	"Modules": [
		{
			"Name": "BlueprintPlugin",
			"Type": "Editor",
			"LoadingPhase": "PreDefault"
		}
	]
}
```

### BlueprintPlugin.Build.cs
```C#
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BlueprintPlugin : ModuleRules
{
	public BlueprintPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
                "UnrealEd",
                "Kismet",
				// ... add private dependencies that you statically link with here ...	
            }
            );
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
```
### BlueprintPlugin.h
```C++
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "AssetData.h"

class FBlueprintPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CustomBlueprintOperations(TArray<FAssetData> SelectedAssets);
	void CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
};
```
### BlueprintPlugin.cpp
```C++
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
```