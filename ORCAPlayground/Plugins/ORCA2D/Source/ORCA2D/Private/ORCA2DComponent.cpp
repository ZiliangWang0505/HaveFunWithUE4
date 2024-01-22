// Fill out your copyright notice in the Description page of Project Settings.


#include "ORCA2DComponent.h"
#include "DynamicObstacle2D.h"
#include "StaticObstacle2D.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

float UORCA2DComponent::RVO_EPSILON = 0.00001f;
FVector UORCA2DComponent::DrawDebugOffset = FVector(0.0f, 0.0f, 10.0f);

// Sets default values for this component's properties
UORCA2DComponent::UORCA2DComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UORCA2DComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called for compute AvoidanceVelocity
FVector2D UORCA2DComponent::ComputeAvoidanceVelocity(float DeltaTime, const FVector2D& PreferredVelocity)
{
	FVector2D AvoidanceVelocity = PreferredVelocity;

  if (const ADynamicObstacle2D* Self = Cast<ADynamicObstacle2D>(GetOwner()))
  {
    ORCALines.Empty();

    const float InvTimeHorizonObst = 1.0f / TimeHorizonObst;

    /* Create obstacle ORCA lines. */
    TArray<AActor*> OutStaticObstacleActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticObstacle2D::StaticClass(), OutStaticObstacleActors);
    for (const AActor* Actor : OutStaticObstacleActors)
   {
      const AStaticObstacle2D* Obstacle1 = Cast<AStaticObstacle2D>(Actor);
      const AStaticObstacle2D* Obstacle2 = Obstacle1->Next;

      const FVector2D RelativePosition1 = Obstacle1->Point - FVector2D(Self->GetActorLocation());
      const FVector2D RelativePosition2 = Obstacle2->Point - FVector2D(Self->GetActorLocation());

      /* Check if velocity obstacle of obstacle is already taken care of by
       * previously constructed obstacle ORCA lines. */
      bool bAlreadyCovered = false;

      for (int j = 0; j < ORCALines.Num(); ++j) {
        if (((InvTimeHorizonObst * RelativePosition1 - ORCALines[j].Point) ^
                ORCALines[j].Direction) -
                    InvTimeHorizonObst * Self->GetDynamicObstacleRadius() >=
                -UORCA2DComponent::RVO_EPSILON &&
            ((InvTimeHorizonObst * RelativePosition2 - ORCALines[j].Point) ^
                ORCALines[j].Direction) -
                    InvTimeHorizonObst * Self->GetDynamicObstacleRadius() >=
                -UORCA2DComponent::RVO_EPSILON) {
          bAlreadyCovered = true;
          break;
        }
      }

      if (bAlreadyCovered) {
        continue;
      }

      /* Not yet covered. Check for collisions. */
      const float DistSq1 = RelativePosition1.SizeSquared();
      const float DistSq2 = RelativePosition2.SizeSquared();

      const float RadiusSq = Self->GetDynamicObstacleRadius() * Self->GetDynamicObstacleRadius();

      const FVector2D ObstacleVector = Obstacle2->Point - Obstacle1->Point;
      const float S =
          (-RelativePosition1 | ObstacleVector) / ObstacleVector.SizeSquared();
      const float DistSqLine = (-RelativePosition1 - S * ObstacleVector).SizeSquared();

      FLine Line;

      if (S < 0.0f && DistSq1 <= RadiusSq) {
        /* Collision with left vertex. Ignore if non-convex. */
        if (Obstacle1->bConvex) {
          Line.Point = FVector2D(0.0f, 0.0f);
          Line.Direction =
              (FVector2D(-RelativePosition1.Y, RelativePosition1.X)).GetSafeNormal();
          DrawDebugORCALine(Line, FColor::Purple);
          ORCALines.Add(Line);
        }

        continue;
      }

      if (S > 1.0f && DistSq2 <= RadiusSq) {
        /* Collision with right vertex. Ignore if non-convex or if it will be
         * taken care of by neighoring obstace */
        if (Obstacle2->bConvex &&
            (RelativePosition2 ^ Obstacle2->Direction) >= 0.0f) {
          Line.Point = FVector2D(0.0f, 0.0f);
          Line.Direction =
              (FVector2D(-RelativePosition2.Y, RelativePosition2.X)).GetSafeNormal();
          DrawDebugORCALine(Line, FColor::Purple);
          ORCALines.Add(Line);
        }

        continue;
      }

      if (S >= 0.0f && S <= 1.0f && DistSqLine <= RadiusSq) {
        /* Collision with obstacle segment. */
        Line.Point = FVector2D(0.0f, 0.0f);
        Line.Direction = -Obstacle1->Direction;
        DrawDebugORCALine(Line, FColor::Purple);
        ORCALines.Add(Line);
        continue;
      }

      /* No collision. Compute legs. When obliquely viewed, both legs can come
       * from a single vertex. Legs extend cut-off line when nonconvex vertex. */
      FVector2D LeftLegDirection;
      FVector2D RightLegDirection;

      if (S < 0.0f && DistSqLine <= RadiusSq) {
        /* Obstacle viewed obliquely so that left vertex defines velocity
         * obstacle. */
        if (!Obstacle1->bConvex) {
          /* Ignore obstacle. */
          continue;
        }

        Obstacle2 = Obstacle1;

        const float Leg1 = FMath::Sqrt(DistSq1 - RadiusSq);
        LeftLegDirection =
            FVector2D(
                RelativePosition1.X * Leg1 - RelativePosition1.Y * Self->GetDynamicObstacleRadius(),
                RelativePosition1.X * Self->GetDynamicObstacleRadius() + RelativePosition1.Y * Leg1) /
            DistSq1;
        RightLegDirection =
            FVector2D(
                RelativePosition1.X * Leg1 + RelativePosition1.Y * Self->GetDynamicObstacleRadius(),
                -RelativePosition1.X * Self->GetDynamicObstacleRadius() + RelativePosition1.Y * Leg1) /
            DistSq1;
      } else if (S > 1.0f && DistSqLine <= RadiusSq) {
        /* Obstacle viewed obliquely so that right vertex defines velocity
         * obstacle. */
        if (!Obstacle2->bConvex) {
          /* Ignore obstacle. */
          continue;
        }

        Obstacle1 = Obstacle2;

        const float Leg2 = FMath::Sqrt(DistSq2 - RadiusSq);
        LeftLegDirection =
            FVector2D(
                RelativePosition2.X * Leg2 - RelativePosition2.Y * Self->GetDynamicObstacleRadius(),
                RelativePosition2.X * Self->GetDynamicObstacleRadius() + RelativePosition2.Y * Leg2) /
            DistSq2;
        RightLegDirection =
            FVector2D(
                RelativePosition2.X * Leg2 + RelativePosition2.Y * Self->GetDynamicObstacleRadius(),
                -RelativePosition2.X * Self->GetDynamicObstacleRadius() + RelativePosition2.Y * Leg2) /
            DistSq2;
      } else {
        /* Usual situation. */
        if (Obstacle1->bConvex) {
          const float Leg1 = FMath::Sqrt(DistSq1 - RadiusSq);
          LeftLegDirection = FVector2D(RelativePosition1.X * Leg1 -
                                         RelativePosition1.Y * Self->GetDynamicObstacleRadius(),
                                     RelativePosition1.X * Self->GetDynamicObstacleRadius() +
                                         RelativePosition1.Y * Leg1) /
                             DistSq1;
        } else {
          /* Left vertex non-convex; left leg extends cut-off line. */
          LeftLegDirection = -Obstacle1->Direction;
        }

        if (Obstacle2->bConvex) {
          const float Leg2 = FMath::Sqrt(DistSq2 - RadiusSq);
          RightLegDirection = FVector2D(RelativePosition2.X * Leg2 +
                                          RelativePosition2.Y * Self->GetDynamicObstacleRadius(),
                                      -RelativePosition2.X * Self->GetDynamicObstacleRadius() +
                                          RelativePosition2.Y * Leg2) /
                              DistSq2;
        } else {
          /* Right vertex non-convex; right leg extends cut-off line. */
          RightLegDirection = Obstacle1->Direction;
        }
      }

      /* Legs can never point into neighboring edge when convex vertex, take
       * cutoff-line of neighboring edge instead. If velocity projected on
       * "foreign" leg, no constraint is added. */
      const AStaticObstacle2D *const LeftNeighbor = Obstacle1->Previous;

      bool bLeftLegForeign = false;
      bool bRightLegForeign = false;

      if (Obstacle1->bConvex &&
          (LeftLegDirection ^ -LeftNeighbor->Direction) >= 0.0f) {
        /* Left leg points into obstacle. */
        LeftLegDirection = -LeftNeighbor->Direction;
        bLeftLegForeign = true;
      }

      if (Obstacle2->bConvex &&
          (RightLegDirection ^ Obstacle2->Direction) <= 0.0f) {
        /* Right leg points into obstacle. */
        RightLegDirection = Obstacle2->Direction;
        bRightLegForeign = true;
      }

      /* Compute cut-off centers. */
      const FVector2D LeftCutoff =
          InvTimeHorizonObst * (Obstacle1->Point - FVector2D(Self->GetActorLocation()));
      const FVector2D RightCutoff =
          InvTimeHorizonObst * (Obstacle2->Point - FVector2D(Self->GetActorLocation()));
      const FVector2D CutoffVector = RightCutoff - LeftCutoff;

      /* Project current velocity on velocity obstacle. */

      /* Check if current velocity is projected on cutoff circles. */
      const float T =
          Obstacle1 == Obstacle2
              ? 0.5f
              : (Self->GetDynamicObstacleVelocity2D() - LeftCutoff) | CutoffVector / CutoffVector.SizeSquared();
      const float TLeft = (Self->GetDynamicObstacleVelocity2D() - LeftCutoff) | LeftLegDirection;
      const float TRight = (Self->GetDynamicObstacleVelocity2D() - RightCutoff) | RightLegDirection;

      if ((T < 0.0f && TLeft < 0.0f) ||
          (Obstacle1 == Obstacle2 && TLeft < 0.0f && TRight < 0.0f)) {
        /* Project on left cut-off circle. */
        const FVector2D UnitW = (Self->GetDynamicObstacleVelocity2D() - LeftCutoff).GetSafeNormal();
        DrawDebugW(Self->GetDynamicObstacleVelocity2D(), LeftCutoff, FColor::Turquoise);

        Line.Direction = FVector2D(UnitW.Y, -UnitW.X);
        Line.Point = LeftCutoff + Self->GetDynamicObstacleRadius() * InvTimeHorizonObst * UnitW;
        DrawDebugVOBodyForStaticObstacle(LeftCutoff, RightCutoff, LeftLegDirection, RightLegDirection, Self->GetDynamicObstacleRadius());
        DrawDebugORCALine(Line, FColor::Blue);
        ORCALines.Add(Line);
        continue;
      }

      if (T > 1.0f && TRight < 0.0f) {
        /* Project on right cut-off circle. */
        const FVector2D UnitW = (Self->GetDynamicObstacleVelocity2D() - RightCutoff).GetSafeNormal();
        DrawDebugW(Self->GetDynamicObstacleVelocity2D(), LeftCutoff, FColor::Turquoise);

        Line.Direction = FVector2D(UnitW.Y, -UnitW.X);
        Line.Point = RightCutoff + Self->GetDynamicObstacleRadius() * InvTimeHorizonObst * UnitW;
        DrawDebugVOBodyForStaticObstacle(LeftCutoff, RightCutoff, LeftLegDirection, RightLegDirection, Self->GetDynamicObstacleRadius());
        DrawDebugORCALine(Line, FColor::Blue);
        ORCALines.Add(Line);
        continue;
      }

      /* Project on left leg, right leg, or cut-off line, whichever is closest to
       * velocity. */
      const float DistSqCutoff =
          (T < 0.0f || T > 1.0f || Obstacle1 == Obstacle2)
              ? TNumericLimits<float>::Max()
              : (Self->GetDynamicObstacleVelocity2D() - (LeftCutoff + T * CutoffVector)).SizeSquared();
      const float DistSqLeft =
          TLeft < 0.0f
              ? TNumericLimits<float>::Max()
              : (Self->GetDynamicObstacleVelocity2D() - (LeftCutoff + TLeft * LeftLegDirection)).SizeSquared();
      const float DistSqRight =
          TRight < 0.0f
              ? TNumericLimits<float>::Max()
              : (Self->GetDynamicObstacleVelocity2D() - (RightCutoff + TRight * RightLegDirection)).SizeSquared();

      if (DistSqCutoff <= DistSqLeft && DistSqCutoff <= DistSqRight) {
        /* Project on cut-off line. */
        Line.Direction = -Obstacle1->Direction;
        Line.Point =
            LeftCutoff + Self->GetDynamicObstacleRadius() * InvTimeHorizonObst *
                             FVector2D(-Line.Direction.Y, Line.Direction.X);
        DrawDebugVOBodyForStaticObstacle(LeftCutoff, RightCutoff, LeftLegDirection, RightLegDirection, Self->GetDynamicObstacleRadius());
        DrawDebugORCALine(Line, FColor::Blue);
        ORCALines.Add(Line);
        continue;
      }

      if (DistSqLeft <= DistSqRight) {
        /* Project on left leg. */
        if (bLeftLegForeign) {
          continue;
        }

        Line.Direction = LeftLegDirection;
        Line.Point =
            LeftCutoff + Self->GetDynamicObstacleRadius() * InvTimeHorizonObst *
                             FVector2D(-Line.Direction.Y, Line.Direction.X);
        DrawDebugVOBodyForStaticObstacle(LeftCutoff, RightCutoff, LeftLegDirection, RightLegDirection, Self->GetDynamicObstacleRadius());
        DrawDebugORCALine(Line, FColor::Blue);
        ORCALines.Add(Line);
        continue;
      }

      /* Project on right leg. */
      if (bRightLegForeign) {
        continue;
      }

      Line.Direction = -RightLegDirection;
      Line.Point =
          RightCutoff + Self->GetDynamicObstacleRadius() * InvTimeHorizonObst *
                            FVector2D(-Line.Direction.Y, Line.Direction.X);
      DrawDebugVOBodyForStaticObstacle(LeftCutoff, RightCutoff, LeftLegDirection, RightLegDirection, Self->GetDynamicObstacleRadius());
      DrawDebugORCALine(Line, FColor::Blue);
      ORCALines.Add(Line);
    }
  
    const int NumObstLines = ORCALines.Num();

    const float InvTimeHorizon = 1.0f / TimeHorizon;

    /* Create agent ORCA lines. */
    TArray<AActor*> OutDynamicObstacleActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADynamicObstacle2D::StaticClass(), OutDynamicObstacleActors);
    for (const AActor* Actor : OutDynamicObstacleActors)
    {
      if (IsValid(Actor) && Actor!= GetOwner())
      {
        if (const ADynamicObstacle2D* Other = Cast<ADynamicObstacle2D>(Actor))
        {
          const FVector2D RelativePosition = FVector2D(Other->GetActorLocation() - Self->GetActorLocation());
          const FVector2D RelativeVelocity = FVector2D(Self->GetDynamicObstacleVelocity() - Other->GetDynamicObstacleVelocity());
          const float DistSq = RelativePosition.SizeSquared();
          const float CombinedRadius = Self->GetDynamicObstacleRadius() + Other->GetDynamicObstacleRadius();
          const float CombinedRadiusSq = CombinedRadius * CombinedRadius;

          DrawDebugVOBodyForDynamicObstacle(RelativePosition, CombinedRadius);
          
          FLine Line;
          FVector2D U;

          if (DistSq > CombinedRadiusSq) {
            /* No collision. */
            const FVector2D W = RelativeVelocity - InvTimeHorizon * RelativePosition;
            /* Vector from cutoff center to relative velocity. */
            const float WLengthSq = W.SizeSquared();

            const float DotProduct = W | RelativePosition;

            if (DotProduct < 0.0f &&
                DotProduct * DotProduct > CombinedRadiusSq * WLengthSq) {
              /* Project on cut-off circle. */
              DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Cyan);
              const float WLength = FMath::Sqrt(WLengthSq);
              const FVector2D UnitW = W / WLength;

              Line.Direction = FVector2D(UnitW.Y, -UnitW.X);
              U = (CombinedRadius * InvTimeHorizon - WLength) * UnitW;
            } else {
              /* Project on legs. */
              DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Yellow);

              const float Leg = FMath::Sqrt(DistSq - CombinedRadiusSq);

              if ((RelativePosition ^ W) > 0.0f) {
                /* Project on left leg. */
                Line.Direction = FVector2D(RelativePosition.X * Leg -
                                             RelativePosition.Y * CombinedRadius,
                                         RelativePosition.X * CombinedRadius +
                                             RelativePosition.Y * Leg) /
                                 DistSq;
              } else {
                /* Project on right leg. */
                Line.Direction = -FVector2D(RelativePosition.X * Leg +
                                              RelativePosition.Y * CombinedRadius,
                                          -RelativePosition.X * CombinedRadius +
                                              RelativePosition.Y * Leg) /
                                 DistSq;
              }

              U = (RelativeVelocity | Line.Direction) * Line.Direction -
                  RelativeVelocity;
            }
          } else {
            /* Collision. Project on cut-off circle of time timeStep. */
            const float InvTimeStep = 1.0f / DeltaTime;

            /* Vector from cutoff center to relative velocity. */
            const FVector2D W = RelativeVelocity - InvTimeStep * RelativePosition;
            DrawDebugW(RelativeVelocity, InvTimeHorizon * RelativePosition, FColor::Orange);

            const float WLength = W.Size();
            const FVector2D UnitW = W / WLength;

            Line.Direction = FVector2D(UnitW.Y, -UnitW.X);
            U = (CombinedRadius * InvTimeStep - WLength) * UnitW;
          }

          Line.Point = Self->GetDynamicObstacleVelocity2D() + 0.5f * U;
          DrawDebugORCALine(Line, FColor::Red);
          ORCALines.Add(Line);
        }
      }
    }
    
    const int LineFail =
       UORCA2DComponent::LinearProgram2(ORCALines, Self->GetDynamicObstacleMaxSpeed(), PreferredVelocity, bDirectionOpt, AvoidanceVelocity);

    if (LineFail < ORCALines.Num()) {
      UORCA2DComponent::LinearProgram3(ORCALines, NumObstLines, LineFail, Self->GetDynamicObstacleMaxSpeed(), AvoidanceVelocity);
    }
  }
  
  DrawDebugAvoidanceVelocity(PreferredVelocity, AvoidanceVelocity);
	return AvoidanceVelocity;
}


bool UORCA2DComponent::LinearProgram1(const TArray<FLine>& Lines, int LineNo,
                    float Radius, const FVector2D& OptVelocity, bool bDirectionOpt,
                    FVector2D& Result)
{
  /* NOLINT(runtime/references) */
  const float DotProduct = Lines[LineNo].Point | Lines[LineNo].Direction;
  const float Discriminant =
      DotProduct * DotProduct + Radius * Radius - Lines[LineNo].Point.SizeSquared();

  if (Discriminant < 0.0f) {
    /* Max speed circle fully invalidates line lineNo. */
    return false;
  }

  const float SqrtDiscriminant = FMath::Sqrt(Discriminant);
  float TLeft = -DotProduct - SqrtDiscriminant;
  float TRight = -DotProduct + SqrtDiscriminant;

  for (int i = 0; i < LineNo; ++i) {
    const float Denominator = Lines[LineNo].Direction ^ Lines[i].Direction;
    const float Numerator =
        Lines[i].Direction | (Lines[LineNo].Point - Lines[i].Point);

    if (FMath::Abs(Denominator) <= UORCA2DComponent::RVO_EPSILON) {
      /* Lines lineNo and i are (almost) parallel. */
      if (Numerator < 0.0f) {
        return false;
      }

      continue;
    }

    const float T = Numerator / Denominator;

    if (Denominator >= 0.0f) {
      /* Line i bounds line lineNo on the right. */
      TRight = FMath::Min(TRight, T);
    } else {
      /* Line i bounds line lineNo on the left. */
      TLeft = FMath::Max(TLeft, T);
    }

    if (TLeft > TRight) {
      return false;
    }
  }

  if (bDirectionOpt) {
    /* Optimize direction. */
    if ((OptVelocity | Lines[LineNo].Direction) > 0.0f) {
      /* Take right extreme. */
      Result = Lines[LineNo].Point + TRight * Lines[LineNo].Direction;
    } else {
      /* Take left extreme. */
      Result = Lines[LineNo].Point + TLeft * Lines[LineNo].Direction;
    }
  } else {
    /* Optimize closest point. */
    const float T =
        Lines[LineNo].Direction | (OptVelocity - Lines[LineNo].Point);

    if (T < TLeft) {
      Result = Lines[LineNo].Point + TLeft * Lines[LineNo].Direction;
    } else if (T > TRight) {
      Result = Lines[LineNo].Point + TRight * Lines[LineNo].Direction;
    } else {
      Result = Lines[LineNo].Point + T * Lines[LineNo].Direction;
    }
  }

  return true;
}


int UORCA2DComponent::LinearProgram2(const TArray<FLine>& Lines, float Radius,
                           const FVector2D& OptVelocity, bool bDirectionOpt,
                           FVector2D& Result)
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

  for (int i = 0; i < Lines.Num(); ++i) {
    if ((Lines[i].Direction ^ (Lines[i].Point - Result)) > 0.0f) {
      /* Result does not satisfy constraint i. Compute new optimal result. */
      const FVector2D TempResult = Result;

      if (!UORCA2DComponent::LinearProgram1(Lines, i, Radius, OptVelocity, bDirectionOpt,
                          Result))
      {
        Result = TempResult;

        return i;
      }
    }
  }

  return Lines.Num();
}

void UORCA2DComponent::LinearProgram3(const TArray<FLine>& Lines, int NumObstLines,
                    int BeginLine, float Radius,
                    FVector2D& Result)
{
  /* NOLINT(runtime/references) */
  float Distance = 0.0f;

  for (int i = BeginLine; i < Lines.Num(); ++i) {
    if ((Lines[i].Direction ^ (Lines[i].Point - Result)) > Distance) {
      /* Result does not satisfy constraint of line i. */
      TArray<FLine> ProjLines;
      for (int LineIndex = 0; LineIndex < NumObstLines; ++LineIndex)
      {
        ProjLines.Add(Lines[LineIndex]);
      }

      for (int j = NumObstLines; j < i; ++j) {
        FLine Line;

        const float Determinant = Lines[i].Direction ^ Lines[j].Direction;

        if (FMath::Abs(Determinant) <= UORCA2DComponent::RVO_EPSILON) {
          /* Line i and line j are parallel. */
          if ((Lines[i].Direction ^ Lines[j].Direction) > 0.0f) {
            /* Line i and line j point in the same direction. */
            continue;
          }

          /* Line i and line j point in opposite direction. */
          Line.Point = 0.5f * (Lines[i].Point + Lines[j].Point);
        } else {
          Line.Point = Lines[i].Point + ((Lines[j].Direction ^
                                             Lines[i].Point - Lines[j].Point) /
                                         Determinant) *
                                            Lines[i].Direction;
        }

        Line.Direction = (Lines[j].Direction - Lines[i].Direction).GetSafeNormal();
        ProjLines.Add(Line);
      }

      const FVector2D TempResult = Result;

      if (UORCA2DComponent::LinearProgram2(
              ProjLines, Radius,
              FVector2D(-Lines[i].Direction.Y, Lines[i].Direction.X), true,
              Result) < ProjLines.Num()) {
        /* This should in principle not happen. The result is by definition
         * already in the feasible region of this linear program. If it fails,
         * it is due to small floating point error, and the current result is
         * kept. */
        Result = TempResult;
      }

      Distance = Lines[i].Direction ^ (Lines[i].Point - Result);
    }
  }
}

void UORCA2DComponent::DrawDebugORCALine(const FLine& Line, FColor Color) const
{
  if (bDrawDebug)
  {
    const FVector LineLocation = FVector(Line.Point, 0.0f) + GetOwner()->GetActorLocation() + UORCA2DComponent::DrawDebugOffset;
    const FVector LineDirection = FVector(Line.Direction, 0.0f);
    DrawDebugDirectionalArrow(GetWorld(), LineLocation - 100.0f * LineDirection, LineLocation + 100.0f * LineDirection, 50.0f, Color, false, -1.0f, 0, 10.0f);
  }
}

void UORCA2DComponent::DrawDebugVOBodyForStaticObstacle(const FVector2D& LeftCutoff, const FVector2D& RightCutoff, const FVector2D& LeftLegDirection, const FVector2D& RightLegDirection, const float CombinedRadius) const
{
  if (bDrawDebug)
  {
    const FVector ActorLocation = GetOwner()->GetActorLocation();
    const float CombinedRadiusInvTimeHorizonObst = CombinedRadius / TimeHorizonObst;
    
    const FVector LeftCutoffCircleCenter = ActorLocation + FVector(LeftCutoff, 0.0f);
    DrawDebugCylinder(GetWorld(), LeftCutoffCircleCenter, LeftCutoffCircleCenter + UORCA2DComponent::DrawDebugOffset, CombinedRadiusInvTimeHorizonObst, 16, FColor::Silver, false, -1.0f, 0, 3.0f);
    const FVector LeftLegStart = ActorLocation + FVector(LeftCutoff + CombinedRadiusInvTimeHorizonObst * FVector2D(-LeftLegDirection.Y, LeftLegDirection.X), 0.0f);
    const FVector LeftLegEnd = LeftLegStart + 500.0 * FVector(LeftLegDirection, 0.0f);
    DrawDebugDirectionalArrow(GetWorld(),  LeftLegStart, LeftLegEnd, 50.0f, FColor::Silver, false, -1.0f, 0, 10.0f);

    const FVector RightCutoffCircleCenter = ActorLocation + FVector(RightCutoff, 0.0f);
    DrawDebugCylinder(GetWorld(), RightCutoffCircleCenter, RightCutoffCircleCenter + UORCA2DComponent::DrawDebugOffset, CombinedRadiusInvTimeHorizonObst, 16, FColor::Silver, false, -1.0f, 0, 3.0f);
    const FVector RightLegStart = ActorLocation + FVector(RightCutoff + CombinedRadiusInvTimeHorizonObst * FVector2D(RightLegDirection.Y, -RightLegDirection.X), 0.0f);
    const FVector RightLegEnd = RightLegStart + 500.0 * FVector(RightLegDirection, 0.0f);
    DrawDebugDirectionalArrow(GetWorld(),  RightLegStart, RightLegEnd, 50.0f, FColor::Silver, false, -1.0f, 0, 10.0f);

    const FVector CutoffVector = (RightCutoffCircleCenter - LeftCutoffCircleCenter).GetSafeNormal2D();
    const FVector RightCutoffInCircle = RightCutoffCircleCenter + CombinedRadiusInvTimeHorizonObst * FVector(CutoffVector.Y, -CutoffVector.X, 0.0f);
    const FVector LeftCutoffInCircle = LeftCutoffCircleCenter + CombinedRadiusInvTimeHorizonObst * FVector(CutoffVector.Y, -CutoffVector.X, 0.0f);
    DrawDebugLine(GetWorld(),  LeftCutoffInCircle, RightCutoffInCircle, FColor::Silver, false, -1.0f, 0, 10.0f);
  }
}

void UORCA2DComponent::DrawDebugVOBodyForDynamicObstacle(const FVector2D& RelativePosition, const float CombinedRadius) const
{
  if (bDrawDebug)
  {
    const FVector ActorLocation = GetOwner()->GetActorLocation();
    const FVector2D RelativePositioInvTimeHorizonn = RelativePosition / TimeHorizon;
    const float CombinedRadiusInvTimeHorizon = CombinedRadius / TimeHorizon;

    const FVector CombineBodyStart = ActorLocation + FVector(RelativePosition, 0.0f) + UORCA2DComponent::DrawDebugOffset;
    const FVector CombineBodyEnd = ActorLocation + FVector(RelativePosition, 0.0f) + UORCA2DComponent::DrawDebugOffset + FVector(0.0f, 0.0f, 100.0f);
    DrawDebugCylinder(GetWorld(), CombineBodyStart, CombineBodyEnd, CombinedRadius, 16, FColor::Silver, false, -1.0f, 0, 3.0f);

    const FVector CombineBodyTimeInvHorizonnStart = ActorLocation + FVector(RelativePositioInvTimeHorizonn, 0.0f) + UORCA2DComponent::DrawDebugOffset;
    const FVector CombineBodyTimeInvHorizonnEnd = ActorLocation + FVector(RelativePositioInvTimeHorizonn, 0.0f) + UORCA2DComponent::DrawDebugOffset + FVector(0.0f, 0.0f, 100.0f);
    DrawDebugCylinder(GetWorld(), CombineBodyTimeInvHorizonnStart, CombineBodyTimeInvHorizonnEnd, CombinedRadiusInvTimeHorizon, 16, FColor::Silver, false, -1.0f, 0, 3.0f);
    
    const float Leg = FMath::Sqrt(RelativePosition.SizeSquared() - FMath::Square(CombinedRadius));
    
    const FVector2D LeftLegDirection = FVector2D(RelativePosition.X * Leg -
                                   RelativePosition.Y * CombinedRadius,
                               RelativePosition.X * CombinedRadius +
                                   RelativePosition.Y * Leg) /
                       RelativePosition.SizeSquared();
    const FVector LeftLegStart = ActorLocation + FVector(RelativePositioInvTimeHorizonn + CombinedRadiusInvTimeHorizon * FVector2D(-LeftLegDirection.Y, LeftLegDirection.X), 0.0f);
    const FVector LeftLegEnd = LeftLegStart + 500.0 * FVector(LeftLegDirection, 0.0f);
    DrawDebugDirectionalArrow(GetWorld(),  LeftLegStart, LeftLegEnd, 50.0f, FColor::Silver, false, -1.0f, 0, 10.0f);

    const FVector2D  RightLegDirection = -FVector2D(RelativePosition.X * Leg +
                                    RelativePosition.Y * CombinedRadius,
                                -RelativePosition.X * CombinedRadius +
                                    RelativePosition.Y * Leg) /
                       RelativePosition.SizeSquared();
    const FVector RightLegStart = ActorLocation + FVector(RelativePositioInvTimeHorizonn + CombinedRadiusInvTimeHorizon * FVector2D(-RightLegDirection.Y, RightLegDirection.X), 0.0f);
    const FVector RightLegEnd = RightLegStart - 500.0 * FVector(RightLegDirection, 0.0f);
    DrawDebugDirectionalArrow(GetWorld(),  RightLegStart, RightLegEnd, 50.0f, FColor::Silver, false, -1.0f, 0, 10.0f);
  }
}

void UORCA2DComponent::DrawDebugW(const FVector2D& RelativeVelocity, const FVector2D& InvTimeRelativePosition, FColor Color) const
{
  if (bDrawDebug)
  {
    const FVector WLineStart = FVector(InvTimeRelativePosition, 0.0f) + GetOwner()->GetActorLocation() + UORCA2DComponent::DrawDebugOffset;
    const FVector WLineEnd = FVector(RelativeVelocity, 0.0f) + GetOwner()->GetActorLocation() + UORCA2DComponent::DrawDebugOffset;
    DrawDebugDirectionalArrow(GetWorld(), WLineStart, WLineEnd, 50.0f, Color, false, -1.0f, 0, 10.0f);
  }
}

void UORCA2DComponent::DrawDebugAvoidanceVelocity(const FVector2D& PreferredVelocity, const FVector2D& AvoidanceVelocity) const
{
  if (bDrawDebug)
  {
    const float Height = 2.0f * GetOwner()->GetSimpleCollisionHalfHeight();
    const float BaseZOffset = Height / UORCA2DComponent::DrawDebugOffset.Z;
    const FVector PreferredVelocityStart = GetOwner()->GetActorLocation() + (BaseZOffset + 1.0f) * UORCA2DComponent::DrawDebugOffset;
    const FVector PreferredVelocityEnd = FVector(PreferredVelocity, 0.0f) + GetOwner()->GetActorLocation() + (BaseZOffset + 1.0f) * UORCA2DComponent::DrawDebugOffset;
    DrawDebugDirectionalArrow(GetWorld(), PreferredVelocityStart, PreferredVelocityEnd, 50.0f, FColor::Black, false, -1.0f, 0, 10.0f);

    const FVector AvoidanceVelocityStart = GetOwner()->GetActorLocation() + (BaseZOffset + 2.0f) * UORCA2DComponent::DrawDebugOffset;
    const FVector AvoidanceVelocityEnd = FVector(AvoidanceVelocity, 0.0f) + GetOwner()->GetActorLocation() + (BaseZOffset + 2.0f) * UORCA2DComponent::DrawDebugOffset;
    DrawDebugDirectionalArrow(GetWorld(), AvoidanceVelocityStart, AvoidanceVelocityEnd, 50.0f, FColor::Green, false, -1.0f, 0, 10.0f);
  }
}