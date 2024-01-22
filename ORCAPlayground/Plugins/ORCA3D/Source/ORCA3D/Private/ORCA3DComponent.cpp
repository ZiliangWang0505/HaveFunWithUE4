// Fill out your copyright notice in the Description page of Project Settings.


#include "ORCA3DComponent.h"
#include "DynamicObstacle3D.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

float UORCA3DComponent::RVO3D_EPSILON = 0.00001f;

// Sets default values for this component's properties
UORCA3DComponent::UORCA3DComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UORCA3DComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called for compute AvoidanceVelocity
FVector UORCA3DComponent::ComputeAvoidanceVelocity(float DeltaTime, const FVector& PreferredVelocity)
{
	FVector AvoidanceVelocity = PreferredVelocity;
  if (const ADynamicObstacle3D* Self = Cast<ADynamicObstacle3D>(GetOwner()))
  {
    ORCAPlanes.Empty();
    const float InvTimeHorizon = 1.0f / TimeHorizon;

    /* Create agent ORCA planes. */
    TArray<AActor*> OutDynamicObstacleActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicObstacle3D::StaticClass(), OutDynamicObstacleActors);
    for (const AActor* Actor : OutDynamicObstacleActors)
    {
      if (IsValid(Actor) && Actor!= GetOwner())
      {
        if (const ADynamicObstacle3D* Other = Cast<ADynamicObstacle3D>(Actor))
        {
          const FVector RelativePosition = Other->GetActorLocation() - Self->GetActorLocation();
          const FVector RelativeVelocity = Self->GetDynamicObstacleVelocity() - Other->GetDynamicObstacleVelocity();
          const float DistSq = RelativePosition.SizeSquared();
          const float CombinedRadius = Self->GetDynamicObstacleRadius() + Other->GetDynamicObstacleRadius();
          const float CombinedRadiusSq = CombinedRadius * CombinedRadius;

          FPlane3D Plane;
          FVector U;

          if (DistSq > CombinedRadiusSq) {
            /* No collision. */
            const FVector W = RelativeVelocity - InvTimeHorizon * RelativePosition;
            /* Vector from cutoff center to relative velocity. */
            const float WLengthSq = W.SizeSquared();

            const float DotProduct = W | RelativePosition;

            if (DotProduct < 0.0f &&
                DotProduct * DotProduct > CombinedRadiusSq * WLengthSq) {
              /* Project on cut-off circle. */
              DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Cyan);
              const float WLength = FMath::Sqrt(WLengthSq);
              const FVector UnitW = W / WLength;

              Plane.Normal = UnitW;
              U = (CombinedRadius * InvTimeHorizon - WLength) * UnitW;
                } else {
                  /* Project on cone. */
                  DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Yellow);
                  const float A = DistSq;
                  const float B = RelativePosition | RelativeVelocity;
                  const float C = RelativeVelocity.SizeSquared() -
                                  (RelativePosition ^ RelativeVelocity).SizeSquared() /
                                      (DistSq - CombinedRadiusSq);
                  const float T = (B + FMath::Sqrt(B * B - A * C)) / A;
                  const FVector WW = RelativeVelocity - T * RelativePosition;
                  const float WWLength = WW.Size();
                  const FVector UnitWW = WW / WWLength;

                  Plane.Normal = UnitWW;
                  U = (CombinedRadius * T - WWLength) * UnitWW;
                }
          } else {
            /* Collision. */
            DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Orange);
            const float InvTimeStep = 1.0f / DeltaTime;
            const FVector W = RelativeVelocity - InvTimeStep * RelativePosition;
            const float WLength = W.Size();
            const FVector UnitW = W / WLength;

            Plane.Normal = UnitW;
            U = (CombinedRadius * InvTimeStep - WLength) * UnitW;
          }

          Plane.Point = Self->GetDynamicObstacleVelocity() + 0.5f * U;
          DrawDebugORCAPlane(Plane, FColor::Red);
          ORCAPlanes.Add(Plane);
        }

        const int PlaneFail = LinearProgram3(
            ORCAPlanes, Self->GetDynamicObstacleMaxSpeed(), PreferredVelocity, false, AvoidanceVelocity);

        if (PlaneFail < ORCAPlanes.Num()) {
          LinearProgram4(ORCAPlanes, PlaneFail, Self->GetDynamicObstacleMaxSpeed(), AvoidanceVelocity);
        }
      }
    }
  }
  
  DrawDebugAvoidanceVelocity(PreferredVelocity, AvoidanceVelocity);
	return AvoidanceVelocity;
}

bool UORCA3DComponent::LinearProgram1(const TArray<FPlane3D>& Planes, int PlaneNo,
                    const FLine3D& Line, float Radius, const FVector& OptVelocity,
                    bool bDirectionOpt,
                    FVector& Result)
{
  /* NOLINT(runtime/references) */
  const float DotProduct = Line.Point | Line.Direction;
  const float Discriminant =
      DotProduct * DotProduct + Radius * Radius - Line.Point.SizeSquared();

  if (Discriminant < 0.0f) {
    /* Max speed sphere fully invalidates line. */
    return false;
  }

  const float SqrtDiscriminant = FMath::Sqrt(Discriminant);
  float TLeft = -DotProduct - SqrtDiscriminant;
  float TRight = -DotProduct + SqrtDiscriminant;

  for (int i = 0; i < PlaneNo; ++i) {
    const float Numerator = (Planes[i].Point - Line.Point) | Planes[i].Normal;
    const float Denominator = Line.Direction | Planes[i].Normal;

    if (Denominator * Denominator <= RVO3D_EPSILON) {
      /* Lines line is (almost) parallel to plane i. */
      if (Numerator > 0.0f) {
        return false;
      }

      continue;
    }

    const float T = Numerator / Denominator;

    if (Denominator >= 0.0f) {
      /* Plane i bounds line on the left. */
      TLeft = FMath::Max(TLeft, T);
    } else {
      /* Plane i bounds line on the right. */
      TRight = FMath::Min(TRight, T);
    }

    if (TLeft > TRight) {
      return false;
    }
  }

  if (bDirectionOpt) {
    /* Optimize direction. */
    if ((OptVelocity | Line.Direction) > 0.0f) {
      /* Take right extreme. */
      Result = Line.Point + TRight * Line.Direction;
    } else {
      /* Take left extreme. */
      Result = Line.Point + TLeft * Line.Direction;
    }
  } else {
    /* Optimize closest point. */
    const float T = Line.Direction | (OptVelocity - Line.Point);

    if (T < TLeft) {
      Result = Line.Point + TLeft * Line.Direction;
    } else if (T > TRight) {
      Result = Line.Point + TRight * Line.Direction;
    } else {
      Result = Line.Point + T * Line.Direction;
    }
  }

  return true;
}

bool UORCA3DComponent::LinearProgram2(const TArray<FPlane3D>& Planes, int PlaneNo,
                    float Radius, const FVector& OptVelocity, bool bDirectionOpt,
                    FVector& Result)
{
  /* NOLINT(runtime/references) */
  const float PlaneDist = Planes[PlaneNo].Point | Planes[PlaneNo].Normal;
  const float PlaneDistSq = PlaneDist * PlaneDist;
  const float RadiusSq = Radius * Radius;

  if (PlaneDistSq > RadiusSq) {
    /* Max speed sphere fully invalidates plane PlaneNo. */
    return false;
  }

  const float PlaneRadiusSq = RadiusSq - PlaneDistSq;

  const FVector PlaneCenter = PlaneDist * Planes[PlaneNo].Normal;

  if (bDirectionOpt) {
    /* Project direction OptVelocity on plane PlaneNo. */
    const FVector PlaneOptVelocity =
        OptVelocity -
        (OptVelocity | Planes[PlaneNo].Normal) * Planes[PlaneNo].Normal;
    const float PlaneOptVelocityLengthSq = PlaneOptVelocity.SizeSquared();

    if (PlaneOptVelocityLengthSq <= RVO3D_EPSILON) {
      Result = PlaneCenter;
    } else {
      Result =
          PlaneCenter + FMath::Sqrt(PlaneRadiusSq / PlaneOptVelocityLengthSq) *
                            PlaneOptVelocity;
    }
  } else {
    /* Project point OptVelocity on plane PlaneNo. */
    Result = OptVelocity +
             ((Planes[PlaneNo].Point - OptVelocity) | Planes[PlaneNo].Normal) *
                 Planes[PlaneNo].Normal;

    /* If outside planeCircle, project on planeCircle. */
    if (Result.SizeSquared() > RadiusSq) {
      const FVector PlaneResult = Result - PlaneCenter;
      const float PlaneResultLengthSq = PlaneResult.SizeSquared();
      Result = PlaneCenter +
               FMath::Sqrt(PlaneRadiusSq / PlaneResultLengthSq) * PlaneResult;
    }
  }

  for (int i = 0; i < PlaneNo; ++i) {
    if ((Planes[i].Normal | (Planes[i].Point - Result)) > 0.0f) {
      /* Result does not satisfy constraint i. Compute new optimal Result.
       * Compute intersection line of plane i and plane PlaneNo.
       */
      FVector CrossProduct = Planes[i].Normal ^ Planes[PlaneNo].Normal;

      if (CrossProduct.SizeSquared() <= RVO3D_EPSILON) {
        /* Planes PlaneNo and i are (almost) parallel, and plane i fully
         * invalidates plane PlaneNo.
         */
        return false;
      }

      FLine3D Line;
      Line.Direction = CrossProduct.GetSafeNormal();
      const FVector LineNormal = Line.Direction ^ Planes[PlaneNo].Normal;
      Line.Point =
          Planes[PlaneNo].Point +
          ((((Planes[i].Point - Planes[PlaneNo].Point) | Planes[i].Normal)) /
           (LineNormal | Planes[i].Normal)) *
              LineNormal;

      if (!LinearProgram1(Planes, i, Line, Radius, OptVelocity, bDirectionOpt,
                          Result)) {
        return false;
      }
    }
  }

  return true;
}

int UORCA3DComponent::LinearProgram3(const TArray<FPlane3D>& Planes, float Radius,
                           const FVector& OptVelocity, bool bDirectionOpt,
                           FVector& Result)
{
  /* NOLINT(runtime/references) */
  if (bDirectionOpt) {
    /* Optimize direction. Note that the optimization velocity is of unit length
     * in this case.
     */
    Result = OptVelocity * Radius;
  } else if (OptVelocity.SizeSquared() > Radius * Radius) {
    /* Optimize closest point and outside circle. */
    Result = OptVelocity.GetSafeNormal() * Radius;
  } else {
    /* Optimize closest point and inside circle. */
    Result = OptVelocity;
  }

  for (int i = 0; i < Planes.Num(); ++i) {
    if ((Planes[i].Normal | (Planes[i].Point - Result)) > 0.0f) {
      /* Result does not satisfy constraint i. Compute new optimal Result. */
      const FVector TempResult = Result;

      if (!LinearProgram2(Planes, i, Radius, OptVelocity, bDirectionOpt,
                          Result)) {
        Result = TempResult;
        return i;
      }
    }
  }

  return Planes.Num();
}

void UORCA3DComponent::LinearProgram4(const TArray<FPlane3D>& Planes, int BeginPlane,
                    float Radius,
                    FVector& Result)
{
  /* NOLINT(runtime/references) */
  float Distance = 0.0f;

  for (int i = BeginPlane; i < Planes.Num(); ++i) {
    if ((Planes[i].Normal | (Planes[i].Point - Result)) > Distance) {
      /* Result does not satisfy constraint of plane i. */
     TArray<FPlane3D> ProjPlanes;

      for (int j = 0; j < i; ++j) {
        FPlane3D Plane;

        const FVector CrossProduct = Planes[j].Normal ^ Planes[i].Normal;

        if (CrossProduct.SizeSquared() <= RVO3D_EPSILON) {
          /* Plane i and plane j are (almost) parallel. */
          if ((Planes[i].Normal | Planes[j].Normal) > 0.0f) {
            /* Plane i and plane j point in the same direction. */
            continue;
          }

          /* Plane i and plane j point in opposite direction. */
          Plane.Point = 0.5f * (Planes[i].Point + Planes[j].Point);
        } else {
          /* Plane.Point is point on line of intersection between plane i and
           * plane j.
           */
          const FVector LineNormal = CrossProduct ^ Planes[i].Normal;
          Plane.Point =
              Planes[i].Point +
              (((Planes[j].Point - Planes[i].Point) | Planes[j].Normal) /
               (LineNormal | Planes[j].Normal)) *
                  LineNormal;
        }

        Plane.Normal = (Planes[j].Normal - Planes[i].Normal).GetSafeNormal();
        ProjPlanes.Add(Plane);
      }

      const FVector TempResult = Result;

      if (LinearProgram3(ProjPlanes, Radius, Planes[i].Normal, true, Result) <
          ProjPlanes.Num()) {
        /* This should in principle not happen. The Result is by definition
         * already in the feasible region of this linear program. If it fails,
         * it is due to small floating point error, and the current Result is
         * kept.
         */
        Result = TempResult;
      }

      Distance = Planes[i].Normal | (Planes[i].Point - Result);
    }
  }
}

void UORCA3DComponent::DrawDebugORCAPlane(const FPlane3D& Plane, FColor Color) const
{
  if (bDrawDebug)
  {
    const FVector PlaneCenter = Plane.Point + GetOwner()->GetActorLocation();
    const FRotator PlaneRotation = Plane.Normal.Rotation();
    DrawDebugBox(GetWorld(), PlaneCenter, FVector(10.0f, 100.0f, 100.0f), PlaneRotation.Quaternion(), Color, false, -1.0f, 0, 10.0f);
  }
}

void UORCA3DComponent::DrawDebugW(const FVector& RelativeVelocity, const FVector& InvTimeRelativePosition, FColor Color) const
{
  if (bDrawDebug)
  {
    const FVector WLineStart = GetOwner()->GetActorLocation() + InvTimeRelativePosition;
    const FVector WLineEnd =  GetOwner()->GetActorLocation() + RelativeVelocity;
    DrawDebugDirectionalArrow(GetWorld(), WLineStart, WLineEnd, 50.0f, Color, false, -1.0f, 0, 10.0f);
  }
}

void UORCA3DComponent::DrawDebugAvoidanceVelocity(const FVector& PreferredVelocity, const FVector& AvoidanceVelocity) const
{
  if (bDrawDebug)
  {
    const FVector PreferredVelocityStart = GetOwner()->GetActorLocation();
    const FVector PreferredVelocityEnd = GetOwner()->GetActorLocation() + PreferredVelocity;
    DrawDebugDirectionalArrow(GetWorld(), PreferredVelocityStart, PreferredVelocityEnd, 50.0f, FColor::Black, false, -1.0f, 0, 10.0f);

    const FVector AvoidanceVelocityStart = GetOwner()->GetActorLocation();
    const FVector AvoidanceVelocityEnd =  GetOwner()->GetActorLocation() + AvoidanceVelocity;
    DrawDebugDirectionalArrow(GetWorld(), AvoidanceVelocityStart, AvoidanceVelocityEnd, 50.0f, FColor::Green, false, -1.0f, 0, 10.0f);
  }
}