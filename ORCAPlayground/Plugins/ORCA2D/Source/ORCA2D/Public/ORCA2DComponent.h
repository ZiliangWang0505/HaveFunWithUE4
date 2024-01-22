// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ORCA2DComponent.generated.h"

struct FLine
{
	/**
	* @brief Constructs a directed line instance.
	*/
	FLine() {}

	/**
	 * @brief The direction of the directed line.
	 */
	FVector2D Direction;

	/**
	 * @brief A point on the directed line.
	 */
	FVector2D Point;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ORCA2D_API UORCA2DComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class ADynamicObstacle2D;
	
public:	
	// Sets default values for this component's properties
	UORCA2DComponent();
	
	// Called for compute AvoidanceVelocity
	FVector2D ComputeAvoidanceVelocity(float DeltaTime, const FVector2D& PreferredVelocity);

	static float RVO_EPSILON;
	static FVector DrawDebugOffset;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	 * @relates        Agent
	 * @brief          Solves a one-dimensional linear program on a specified line
	 *                 subject to linear constraints defined by lines and a circular
	 *                 constraint.
	 * @param[in]      lines        Lines defining the linear constraints.
	 * @param[in]      lineNo       The specified line constraint.
	 * @param[in]      radius       The radius of the circular constraint.
	 * @param[in]      optVelocity  The optimization velocity.
	 * @param[in]      directionOpt True if the direction should be optimized.
	 * @param[in, out] result       A reference to the result of the linear program.
	 * @return         True if successful.
	 */
	static bool LinearProgram1(const TArray<FLine>& Lines, int LineNo,
					float Radius, const FVector2D& OptVelocity, bool bDirectionOpt,
					FVector2D& Result);

	/**
	 * @relates        Agent
	 * @brief          Solves a two-dimensional linear program subject to linear
	 *                 constraints defined by lines and a circular constraint.
	 * @param[in]      lines        Lines defining the linear constraints.
	 * @param[in]      radius       The radius of the circular constraint.
	 * @param[in]      optVelocity  The optimization velocity.
	 * @param[in]      directionOpt True if the direction should be optimized.
	 * @param[in, out] result       A reference to the result of the linear program.
	 * @return         The number of the line it fails on, and the number of lines
	 *                 if successful.
	 */
	static int LinearProgram2(const TArray<FLine>& Lines, float Radius,
							   const FVector2D& OptVelocity, bool bDirectionOpt,
							   FVector2D& Result);

	/**
	 * @relates        Agent
	 * @brief          Solves a two-dimensional linear program subject to linear
	 *                 constraints defined by lines and a circular constraint.
	 * @param[in]      lines        Lines defining the linear constraints.
	 * @param[in]      numObstLines Count of obstacle lines.
	 * @param[in]      beginLine    The line on which the 2-d linear program failed.
	 * @param[in]      radius       The radius of the circular constraint.
	 * @param[in, out] result       A reference to the result of the linear program.
	 */
	static void LinearProgram3(const TArray<FLine>& Lines, int NumObstLines,
						int BeginLine, float Radius,
						FVector2D& Result);

protected:
	UPROPERTY(EditAnywhere, Category="ORCA Config")
	float TimeHorizon = 2.0f;

	UPROPERTY(EditAnywhere, Category="ORCA Config")
	float TimeHorizonObst = 2.0f;
	
	UPROPERTY(EditAnywhere, Category="ORCA Config")
	bool bDirectionOpt = false;

	UPROPERTY(EditAnywhere, Category="ORCA Config")
	bool bDrawDebug = false;
	
	TArray<FLine> ORCALines;
	
private:
	void DrawDebugORCALine(const FLine& Line, FColor Color) const;
	void DrawDebugVOBodyForStaticObstacle(const FVector2D& LeftCutoff, const FVector2D& RightCutoff, const FVector2D& LeftLegDirection, const FVector2D& RightLegDirection, const float CombinedRadius) const;
	void DrawDebugVOBodyForDynamicObstacle(const FVector2D& RelativePosition, const float CombinedRadius) const;
	void DrawDebugW(const FVector2D& RelativeVelocity, const FVector2D& InvTimeRelativePosition, FColor Color) const;
	void DrawDebugAvoidanceVelocity(const FVector2D& PreferredVelocity, const FVector2D& AvoidanceVelocity) const;
};
