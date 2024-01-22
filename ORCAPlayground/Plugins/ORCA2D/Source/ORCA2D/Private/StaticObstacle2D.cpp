// Fill out your copyright notice in the Description page of Project Settings.


#include "StaticObstacle2D.h"

// Sets default values
AStaticObstacle2D::AStaticObstacle2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	Direction = FVector2D::ZeroVector;
	Point = FVector2D::ZeroVector;
	Next = nullptr;
	Previous = nullptr;
	bConvex = false;
}

// Called when the game starts or when spawned
void AStaticObstacle2D::BeginPlay()
{
	Super::BeginPlay();
}

void AStaticObstacle2D::AdjustVisualRepresentation()
{
	if (IsValid(Next))
	{
		const FVector2D MidPoint = (Point + Next->Point) / 2.0f;
		const FVector NewLocation = FVector(MidPoint, 0.0f);
		SetActorLocation(NewLocation);

		const FVector NewForward = FVector(Direction, 0.0f);
		SetActorRotation(NewForward.Rotation());

		const float Length = (Point - Next->Point).Size();
		SetActorScale3D(FVector(Length / 100.0f, 1.0f, 1.0f));
	}
}

float AStaticObstacle2D::LeftOf(const FVector2D& Vector1, const FVector2D& Vector2, const FVector2D& Vector3)
{
	return (Vector1 - Vector3) ^ (Vector2 - Vector3);
}


