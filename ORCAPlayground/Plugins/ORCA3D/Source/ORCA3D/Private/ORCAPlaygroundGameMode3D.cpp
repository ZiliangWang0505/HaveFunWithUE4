// Fill out your copyright notice in the Description page of Project Settings.


#include "ORCAPlaygroundGameMode3D.h"
#include "ORCA3D/Public/DynamicObstacle3D.h"

void AORCAPlaygroundGameMode3D::StartPlay()
{
	Super::StartPlay();

	// create DynamicObstacle3D in world
	for (const FDynamicObstacle3DConfig& DynamicObstacle3DConfig : DynamicObstacle3DConfigs)
	{
		CreateDynamicObstacle3D(DynamicObstacle3DConfig);
	}
}

void AORCAPlaygroundGameMode3D::CreateDynamicObstacle3D(const FDynamicObstacle3DConfig& DynamicObstacle3DConfig)
{
	ADynamicObstacle3D* DynamicObstacle3D = GetWorld()->SpawnActor<ADynamicObstacle3D>(DefaultDynamicObstacle3DClass, DynamicObstacle3DConfig.StartLocation, FRotator());
	if (IsValid(DynamicObstacle3D))
	{
		DynamicObstacle3D->SetDestination(DynamicObstacle3DConfig.EndLocation);
		DynamicObstacle3D->SetDesiredSpeed(DynamicObstacle3DConfig.DesiredSpeed);
		DynamicObstacle3D->SetDynamicObstacleRadius(DynamicObstacle3DConfig.Radius);
		DynamicObstacle3D->SetDrawDebug(DynamicObstacle3DConfig.bDrawDebug);
	}
}