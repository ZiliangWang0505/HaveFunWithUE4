// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DynamicObstacle2D.generated.h"

UCLASS()
class ORCA2D_API ADynamicObstacle2D : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ADynamicObstacle2D();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set Destination
	FORCEINLINE void SetDestination(const FVector& NewDestination)
	{
		Destination = NewDestination;
	}

	// Set DesiredSpeed
	FORCEINLINE void SetDesiredSpeed(float NewDesiredSpeed)
	{
		DesiredSpeed = FMath::Min(MaxSpeed, NewDesiredSpeed);
	}

	// Get Preferred Velocity
	FORCEINLINE FVector GetPreferredVelocity() const
	{
		return PreferredVelocity;
	}
	
	// Get DynamicObstacle Velocity
	FORCEINLINE FVector GetDynamicObstacleVelocity() const
	{
		return CurrentVelocity;
	}

	// Get DynamicObstacle Velocity 2D
	FORCEINLINE FVector2D GetDynamicObstacleVelocity2D() const
	{
		return FVector2D(CurrentVelocity);
	}

	// Get DynamicObstacle Max Speed
	FORCEINLINE float GetDynamicObstacleMaxSpeed() const
	{
		return MaxSpeed;
	}

	// Get DynamicObstacle Radius
	FORCEINLINE float GetDynamicObstacleRadius() const
	{
		return GetSimpleCollisionRadius();
	}

	// Set DynamicObstacle Radius
	void SetDynamicObstacleRadius(float NewRadius);

	// Set DrawDebug
	void SetDrawDebug(bool bNewDrawDebug);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category="Dynamic Obstacle Config")
	float MaxSpeed = 1000.0f;

private:
	FVector Destination;
	float DesiredSpeed;
	FVector CurrentVelocity;
	FVector PreferredVelocity;

};
