// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ORCA3DComponent.generated.h"

struct FLine3D
{
	/**
	* @brief Constructs a directed line instance.
	*/
	FLine3D() {}

	/**
	 * @brief The direction of the directed line.
	 */
	FVector Direction;

	/**
	 * @brief A point on the directed line.
	 */
	FVector Point;
};

struct FPlane3D
{
	/**
	  * @brief Constructs a plane.
	  */
	FPlane3D() {}

	/**
	 * @brief A point on the plane.
	 */
	FVector Point;

	/**
	 * @brief The normal to the plane.
	 */
	FVector Normal;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ORCA3D_API UORCA3DComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class ADynamicObstacle3D;

public:	
	// Sets default values for this component's properties
	UORCA3DComponent();

	// Called for compute AvoidanceVelocity
	FVector ComputeAvoidanceVelocity(float DeltaTime, const FVector& PreferredVelocity);

public:	
	static float RVO3D_EPSILON;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	* @brief     Solves a one-dimensional linear program on a specified line
	 *            subject to linear constraints defined by planes and a spherical
	 *            constraint.
	 * @param[in] planes       Planes defining the linear constraints.
	 * @param[in] planeNo      The plane on which the line lies.
	 * @param[in] line         The line on which the one-dimensional linear program
	 *                         is solved.
	 * @param[in] radius       The radius of the spherical constraint.
	 * @param[in] optVelocity  The optimization velocity.
	 * @param[in] directionOpt True if the direction should be optimized.
	 * @param[in] result       A reference to the result of the linear program.
	 * @return True if successful.
	 */
	static bool LinearProgram1(const TArray<FPlane3D>& Planes, int PlaneNo,
						const FLine3D& Line, float Radius, const FVector& OptVelocity,
						bool bDirectionOpt,
						FVector& Result);

	/**
	 * @brief      Solves a two-dimensional linear program on a specified plane
	 *             subject to linear constraints defined by planes and a spherical
	 *             constraint.
	 * @param[in]  planes       Planes defining the linear constraints.
	 * @param[in]  planeNo      The plane on which the two-dimensional linear
	 *                          program is solved.
	 * @param[in]  radius       The radius of the spherical constraint.
	 * @param[in]  optVelocity  The optimization velocity.
	 * @param[in]  directionOpt True if the direction should be optimized.
	 * @param[out] result       A reference to the result of the linear program.
	 * @return     True if successful.
	 */
	static bool LinearProgram2(const TArray<FPlane3D>& Planes, int PlaneNo,
						float Radius, const FVector& OptVelocity, bool bDirectionOpt,
						FVector& Result);

	/**
	 * @brief      Solves a three-dimensional linear program subject to linear
	 *             constraints defined by planes and a spherical constraint.
	 * @param[in]  planes       Planes defining the linear constraints.
	 * @param[in]  radius       The radius of the spherical constraint.
	 * @param[in]  optVelocity  The optimization velocity.
	 * @param[in]  directionOpt True if the direction should be optimized.
	 * @param[out] result       A reference to the result of the linear program.
	 * @return     The number of the plane it fails on, and the number of planes if
	 *             successful.
	 */
	static int LinearProgram3(const TArray<FPlane3D>& Planes, float Radius,
							   const FVector& OptVelocity, bool bDirectionOpt,
							   FVector& Result);

	/**
	 * @brief      Solves a four-dimensional linear program subject to linear
	 *             constraints defined by planes and a spherical constraint.
	 * @param[in]  planes     Planes defining the linear constraints.
	 * @param[in]  beginPlane The plane on which the three-dimensional linear
	 *                        program failed.
	 * @param[in]  radius     The radius of the spherical constraint.
	 * @param[out] result     A reference to the result of the linear program.
	 */
	static void LinearProgram4(const TArray<FPlane3D>& Planes, int BeginPlane,
						float Radius,
						FVector& Result);

protected:
	UPROPERTY(EditAnywhere, Category="ORCA Config")
	float TimeHorizon = 2.0f;

	UPROPERTY(EditAnywhere, Category="ORCA Config")
	bool bDrawDebug = false;
	
	TArray<FPlane3D> ORCAPlanes;

private:
	void DrawDebugORCAPlane(const FPlane3D& Plane, FColor Color) const;
	void DrawDebugW(const FVector& RelativeVelocity, const FVector& InvTimeRelativePosition, FColor Color) const;
	void DrawDebugAvoidanceVelocity(const FVector& PreferredVelocity, const FVector& AvoidanceVelocity) const;
	
};
