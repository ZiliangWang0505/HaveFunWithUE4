// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicObstacle3D.h"
#include "ORCA3DComponent.h"

// Sets default values
ADynamicObstacle3D::ADynamicObstacle3D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Destination = FVector::ZeroVector;
	DesiredSpeed = 0.0f;
	CurrentVelocity = FVector::ZeroVector;
	PreferredVelocity = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ADynamicObstacle3D::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADynamicObstacle3D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const FVector CurrentActorLocation = GetActorLocation();
	if (CurrentActorLocation.Equals(Destination, 1.0f))
	{
		PreferredVelocity = FVector::ZeroVector;
	}
	else
	{
		PreferredVelocity = DesiredSpeed * (Destination - CurrentActorLocation).GetSafeNormal();
	}

	if (UORCA3DComponent* ORCA3DComponent = FindComponentByClass<UORCA3DComponent>())
	{
		const FVector AvoidanceVelocity = ORCA3DComponent->ComputeAvoidanceVelocity(DeltaTime, PreferredVelocity);
		CurrentVelocity = AvoidanceVelocity;
	}
	else
	{
		CurrentVelocity = PreferredVelocity;
	}
	
	// Update Location of DynamicObstacle3D by CurrentVelocity
	const FVector NewActorLocation = CurrentActorLocation + CurrentVelocity * DeltaTime;
	SetActorLocation(NewActorLocation);
	const FVector NewActorForwardVector = CurrentVelocity.GetSafeNormal();
	if (!NewActorForwardVector.IsNearlyZero())
	{
		SetActorRotation(NewActorForwardVector.Rotation());
	}
}

void ADynamicObstacle3D::SetDynamicObstacleRadius(float NewRadius)
{
	const float DynamicObstacleRadius = GetDynamicObstacleRadius();
	check(DynamicObstacleRadius != 0.0f)
	check(NewRadius != 0.0f)
	const float Scale = NewRadius / DynamicObstacleRadius;
	SetActorScale3D(FVector(Scale, Scale, Scale));
}

void ADynamicObstacle3D::SetDrawDebug(bool bNewDrawDebug)
{
	if (UORCA3DComponent* ORCA3DComponent = FindComponentByClass<UORCA3DComponent>())
	{
		ORCA3DComponent->bDrawDebug = bNewDrawDebug;
	}
}

