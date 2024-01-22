// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StaticObstacle2D.generated.h"

UCLASS()
class ORCA2D_API AStaticObstacle2D : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStaticObstacle2D();

	void AdjustVisualRepresentation();
	
	static float LeftOf(const FVector2D& Vector1, const FVector2D& Vector2, const FVector2D& Vector3);

public:
	FVector2D Direction;
	FVector2D Point;
	AStaticObstacle2D* Next;
	AStaticObstacle2D* Previous;
	bool bConvex;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

};
