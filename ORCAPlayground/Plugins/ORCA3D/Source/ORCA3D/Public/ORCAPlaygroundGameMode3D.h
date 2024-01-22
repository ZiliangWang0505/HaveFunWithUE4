// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ORCAPlaygroundGameMode3D.generated.h"

class ADynamicObstacle3D;

USTRUCT(BlueprintType)
struct FDynamicObstacle3DConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector EndLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	float DesiredSpeed = 0.0f;

	UPROPERTY(EditAnywhere)
	float Radius = 100.0f;

	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;
};

/**
 * 
 */
UCLASS()
class ORCA3D_API AORCAPlaygroundGameMode3D : public AGameModeBase
{
	GENERATED_BODY()

public:
	/** Transitions to calls BeginPlay on actors. */
	virtual void StartPlay() override;

	void CreateDynamicObstacle3D(const FDynamicObstacle3DConfig& DynamicObstacle3DConfig);

public:
	UPROPERTY(EditAnywhere, Category=Game)
	TArray<FDynamicObstacle3DConfig> DynamicObstacle3DConfigs;

	UPROPERTY(EditAnywhere, Category=Game)
	TSubclassOf<ADynamicObstacle3D> DefaultDynamicObstacle3DClass = nullptr;
	
};
