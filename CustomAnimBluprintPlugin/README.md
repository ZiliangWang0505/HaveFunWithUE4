### 动画蓝图插件
对于动画蓝图的一系列操作的实现方法。
#### 1.UE4中使用C++创建动画蓝图
创建动画蓝图最重要的是需要提供 "ParentClass" 和 "Skeleton"。

```C++
const FString ABPPathName = FPaths::Combine(FPaths::GetPath(BPPathName), FString::Printf(TEXT("ABP_%s"), *BPName));
USkeleton* TargetSkeleton = SkeletalMeshComponent->SkeletalMesh->Skeleton;
UClass* ParentClassToUse = UVehicleAnimInstance::StaticClass();
UAnimBlueprint* CustomAnimBlueprint = CreateCustomAnimationBlueprint(TargetSkeleton, ParentClassToUse, ABPPathName);
```

在 "CreateCustomAnimationBlueprint" 函数中， 利用"FKismetEditorUtilities::CreateBlueprint" 函数创建新的动画蓝图，同时修改动画蓝图的 "Skeleton"。

```C++
Package = CreatePackage(nullptr, *AnimBlueprintPath);
UObject* InParent = Cast<UObject>(Package);
// 创建新的动画蓝图
UAnimBlueprint* NewBP = CastChecked<UAnimBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClassToUse, InParent, *FPaths::GetBaseFilename(AnimBlueprintPath), BPTYPE_Normal, UAnimBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), FName("ContentBrowserNewAsset")));
	
// 修改动画蓝图的 "Skeleton"
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
```

#### 2.UE4中使用C++为动画蓝图中的 "AnimGraph" 添加动画节点
添加动画节点主要利用 "FGraphNodeCreator"， 同时可以指点节点的位置：
```C++
FGraphNodeCreator<UAnimGraphNode_WheelHandler> WheelHandlerNodeCreator(*AnimGraph);
UAnimGraphNode_WheelHandler*  WheelHandlerNode = WheelHandlerNodeCreator.CreateNode();
WheelHandlerNode->NodePosX = -400;
WheelHandlerNodeCreator.Finalize();
```

#### 3.UE4中使用C++对动画节点添加连接
添加动画节点之间的连接主要通过 "UEdGraphSchema" 类型中的 "CreateAutomaticConversionNodeAndConnections" 对节点的引脚进行连接：

```C++
UEdGraph* AnimGraph = NewBP->FunctionGraphs[0];
const UEdGraphSchema* Schema = AnimGraph->GetSchema();
UEdGraphPin* ResultPin = AnimGraph->Nodes[0]->Pins[0];   // 获取需要连接的引脚

UEdGraphPin* ComponentPosePin = WheelHandlerNode->Pins[4];  // 获取需要连接的引脚

Schema->CreateAutomaticConversionNodeAndConnections(ResultPin, ComponentPosePin);  // 对引脚进行连接
```

#### 4.UE4中为骨骼网格体指定动画蓝图

利用 "USkeletalMeshComponent" 中的 "SetAnimClass" 函数可以重新为 "SkeletonMeshComponent" 重新指定动画蓝图。
```C++
SkeletalMeshComponent->SetAnimClass(CustomAnimBlueprint->GeneratedClass);
```
### 实际效果
选中蓝图资源右键 ->Asset Aciton -> Custom Anim Blueprint Operations。

![使用插件](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomAnimBluprintPlugin/Images/CustomAnimBlueprintPlugin_1.png?raw=true)

执行之后可以看到在同级目录中生成了新的动画蓝图。

![动画蓝图](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomAnimBluprintPlugin/Images/CustomAnimBlueprintPlugin_2.png?raw=true)

打开动画蓝图可以看到代码中添加的 "Wheel Handler" 节点，同时还有对应的连接。"Component To Local" 节点由函数 "CreateAutomaticConversionNodeAndConnections" 自动生成。

![动画蓝图AnimGraph](https://github.com/ZiliangWang0505/HaveFunWithUE4/blob/master/CustomAnimBluprintPlugin/Images/CustomAnimBlueprintPlugin_3.png?raw=true)

### 总结
UE4中各种对于图类型数据的操作基本类似。
首先需要获取正在使用的编辑器指针：
```C++
IAssetEditorInstance* AssetEditor = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(Asset, true);
```
之后将获取到的 抽象类编辑器指针 转化为 实际使用类型对应的编辑器指针，通过实际使用的编辑器指针就可以获取该资源的 Component、Graph等资源：
```C++
TSharedPtr<SSCSEditor> SCSEditor = BlueprintEditorPtr->GetSCSEditor();
FSCSEditorTreeNodePtrType SceneRootNode = SCSEditor->GetSceneRootNode();
UActorComponent* ComponentTemplate = SceneRootNode->GetComponentTemplate();
```
对于图模型的编辑，主要在于创建节点找到引脚之后利用 "UEdGraphSchema" 类来进行具体的业务逻辑编辑工作：
```C++
FGraphNodeCreator<UAnimGraphNode_WheelHandler> WheelHandlerNodeCreator(*AnimGraph);
UAnimGraphNode_WheelHandler*  WheelHandlerNode = WheelHandlerNodeCreator.CreateNode();
WheelHandlerNodeCreator.Finalize();
UEdGraphPin* ComponentPosePin = WheelHandlerNode->Pins[4];
```
图模型的获取也相对比较容易：
```C++
UEdGraph* AnimGraph = NewBP->FunctionGraphs[0];
const UEdGraphSchema* Schema = AnimGraph->GetSchema();
```
