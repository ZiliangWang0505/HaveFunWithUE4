// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ORCAPlaygroundGameMode2D.generated.h"

class ADynamicObstacle2D;
class AStaticObstacle2D;

USTRUCT(BlueprintType)
struct FDynamicObstacle2DConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector2D StartLocation = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector2D EndLocation = FVector2D::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	float DesiredSpeed = 0.0f;

	UPROPERTY(EditAnywhere)
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;
};

USTRUCT(BlueprintType)
struct FStaticObstacle2DConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FVector2D> Points;
};

/**
 * 
 */
UCLASS()
class ORCA2D_API AORCAPlaygroundGameMode2D : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Transitions to calls BeginPlay on actors. */
	virtual void StartPlay() override;

	void CreateDynamicObstacle2D(const FDynamicObstacle2DConfig& DynamicObstacle2DConfig);

	void CreateStaticObstacle2D(const FStaticObstacle2DConfig& StaticObstacle2DConfig);

public:
	UPROPERTY(EditAnywhere, Category=Game)
	TArray<FDynamicObstacle2DConfig> DynamicObstacle2DConfigs;

	UPROPERTY(EditAnywhere, Category=Game)
	TSubclassOf<ADynamicObstacle2D> DefaultDynamicObstacle2DClass = nullptr;

	UPROPERTY(EditAnywhere, Category=Game)
	TArray<FStaticObstacle2DConfig> StaticObstacle2DConfigs;

	UPROPERTY(EditAnywhere, Category=Game)
	TSubclassOf<AStaticObstacle2D> DefaultStaticObstacle2DClass = nullptr;
	
};
