// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicObstacle2D.h"
#include "ORCA2DComponent.h"

// Sets default values
ADynamicObstacle2D::ADynamicObstacle2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Destination = FVector::ZeroVector;
	DesiredSpeed = 0.0f;
	CurrentVelocity = FVector::ZeroVector;
	PreferredVelocity = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ADynamicObstacle2D::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADynamicObstacle2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrentActorLocation = GetActorLocation();
	if (CurrentActorLocation.Equals(Destination, 1.0f))
	{
		PreferredVelocity = FVector::ZeroVector;
	}
	else
	{
		PreferredVelocity = DesiredSpeed * (Destination - CurrentActorLocation).GetSafeNormal2D();
	}

	if (UORCA2DComponent* ORCA2DComponent = FindComponentByClass<UORCA2DComponent>())
	{
		const FVector2D AvoidanceVelocity = ORCA2DComponent->ComputeAvoidanceVelocity(DeltaTime, FVector2D(PreferredVelocity));
		CurrentVelocity = FVector(AvoidanceVelocity, 0.0f);
	}
	else
	{
		CurrentVelocity = PreferredVelocity;
	}

	// Update Location of DynamicObstacle2D by CurrentVelocity
	const FVector NewActorLocation = CurrentActorLocation + CurrentVelocity * DeltaTime;
	SetActorLocation(NewActorLocation);
	const FVector NewActorForwardVector = CurrentVelocity.GetSafeNormal2D();
	if (!NewActorForwardVector.IsNearlyZero())
	{
		SetActorRotation(NewActorForwardVector.Rotation());
	}
}

void ADynamicObstacle2D::SetDynamicObstacleRadius(float NewRadius)
{
	const float DynamicObstacleRadius = GetDynamicObstacleRadius();
	check(DynamicObstacleRadius != 0.0f)
	check(NewRadius != 0.0f)
	const float Scale = NewRadius / DynamicObstacleRadius;
	SetActorScale3D(FVector(Scale, Scale, 1.0f));
}

void ADynamicObstacle2D::SetDrawDebug(bool bNewDrawDebug)
{
	if (UORCA2DComponent* ORCA2DComponent = FindComponentByClass<UORCA2DComponent>())
	{
		ORCA2DComponent->bDrawDebug = bNewDrawDebug;
	}
}
