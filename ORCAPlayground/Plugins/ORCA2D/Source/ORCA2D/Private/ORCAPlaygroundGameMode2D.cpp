// Copyright Epic Games, Inc. All Rights Reserved.


#include "ORCAPlaygroundGameMode2D.h"
#include "ORCA2D/Public/DynamicObstacle2D.h"
#include "ORCA2D/Public/StaticObstacle2D.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

void AORCAPlaygroundGameMode2D::StartPlay()
{
	Super::StartPlay();

	// create DynamicObstacle2D in world
	for (const FDynamicObstacle2DConfig& DynamicObstacle2DConfig : DynamicObstacle2DConfigs)
	{
		CreateDynamicObstacle2D(DynamicObstacle2DConfig);
	}

	// create StaticObstacle2D in world
	for (const FStaticObstacle2DConfig& StaticObstacle2DConfig : StaticObstacle2DConfigs)
	{
		CreateStaticObstacle2D(StaticObstacle2DConfig);
	}

	// adjust visual representation for StaticObstacle2D
	TArray<AActor*> OutStaticObstacleActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticObstacle2D::StaticClass(), OutStaticObstacleActors);
	for (AActor* Actor : OutStaticObstacleActors)
	{
		if (AStaticObstacle2D* StaticObstacle2D = Cast<AStaticObstacle2D>(Actor))
		{
			StaticObstacle2D->AdjustVisualRepresentation(); 
		}
	}
}

void AORCAPlaygroundGameMode2D::CreateDynamicObstacle2D(const FDynamicObstacle2DConfig& DynamicObstacle2DConfig)
{
	ADynamicObstacle2D* DynamicObstacle2D = GetWorld()->SpawnActor<ADynamicObstacle2D>(DefaultDynamicObstacle2DClass, FVector(DynamicObstacle2DConfig.StartLocation, 0.0f), FRotator());
	if (IsValid(DynamicObstacle2D))
	{
		DynamicObstacle2D->SetDestination(FVector(DynamicObstacle2DConfig.EndLocation, 0.0f));
		DynamicObstacle2D->SetDesiredSpeed(DynamicObstacle2DConfig.DesiredSpeed);
		DynamicObstacle2D->SetDynamicObstacleRadius(DynamicObstacle2DConfig.Radius);
		DynamicObstacle2D->SetDrawDebug(DynamicObstacle2DConfig.bDrawDebug);
	}
}

void AORCAPlaygroundGameMode2D::CreateStaticObstacle2D(const FStaticObstacle2DConfig& StaticObstacle2DConfig)
{
	const TArray<FVector2D>& Vertices = StaticObstacle2DConfig.Points;
	AStaticObstacle2D* FirstStaticObstacle2D = nullptr;
	AStaticObstacle2D* PrevStaticObstacle2D = nullptr;
	for (int i = 0; i < Vertices.Num(); ++i)
	{
		AStaticObstacle2D* StaticObstacle2D = GetWorld()->SpawnActor<AStaticObstacle2D>(DefaultStaticObstacle2DClass, FVector(Vertices[i], 0.0f), FRotator());
		if (IsValid(StaticObstacle2D))
		{
			StaticObstacle2D->Point = Vertices[i];

			if (i != 0) {
				StaticObstacle2D->Previous = PrevStaticObstacle2D;
				StaticObstacle2D->Previous->Next = StaticObstacle2D;
			}
			else
			{
				FirstStaticObstacle2D = StaticObstacle2D;
			}

			if (i == Vertices.Num() - 1) {
				StaticObstacle2D->Next = FirstStaticObstacle2D;
				StaticObstacle2D->Next->Previous = StaticObstacle2D;
			}

			StaticObstacle2D->Direction = (
				Vertices[(i == Vertices.Num() - 1 ? 0 : i + 1)] - Vertices[i]).GetSafeNormal();

			if (Vertices.Num() == 2) {
				StaticObstacle2D->bConvex = true;
			} else {
				StaticObstacle2D->bConvex =
					AStaticObstacle2D::LeftOf(Vertices[i == 0 ? Vertices.Num() - 1 : i - 1],
						   Vertices[i],
						   Vertices[i == Vertices.Num() - 1 ? 0 : i + 1]) >= 0.0f;
			}
			PrevStaticObstacle2D = StaticObstacle2D;
		}
	}
}