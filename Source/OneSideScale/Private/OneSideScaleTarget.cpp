#include "OneSideScaleTarget.h"

#include "Components/StaticMeshComponent.h"
#include "DynamicMeshActor.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Editor.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "LevelUtils.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h"

bool OneSideScale::GetLocalBounds(AActor* Actor, FBox& OutBounds)
{
    OutBounds = FBox(ForceInit);
    if (!IsValid(Actor) || !Actor->IsEditable() || Actor->IsLockLocation() ||
        !Actor->GetRootComponent() || Actor->GetAttachParentActor() || FLevelUtils::IsLevelLocked(Actor))
    {
        return false;
    }
    if (const AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
    {
        const UStaticMeshComponent* Component = MeshActor->GetStaticMeshComponent();
        if (Component != Actor->GetRootComponent() || !Component->GetStaticMesh()) { return false; }
        OutBounds = Component->GetStaticMesh()->GetBoundingBox();
    }
    else if (const ADynamicMeshActor* DynamicActor = Cast<ADynamicMeshActor>(Actor))
    {
        const UDynamicMeshComponent* Component = DynamicActor->GetDynamicMeshComponent();
        if (Component != Actor->GetRootComponent()) { return false; }
        Component->ProcessMesh([&OutBounds](const UE::Geometry::FDynamicMesh3& Mesh)
        {
            if (Mesh.VertexCount() > 0)
            {
                const UE::Geometry::FAxisAlignedBox3d Box = Mesh.GetBounds();
                OutBounds = FBox(Box.Min, Box.Max);
            }
        });
    }
    else { return false; }
    const FVector Size = OutBounds.GetSize();
    const FVector Scale = Actor->GetActorScale3D().GetAbs();
    return OutBounds.IsValid && !Size.ContainsNaN() && !Scale.ContainsNaN() &&
        Size.GetMin() > UE_SMALL_NUMBER && Scale.GetMin() > UE_SMALL_NUMBER;
}

FVector OneSideScale::FaceCenter(const FBox& Bounds, int32 Face)
{
    FVector Point = Bounds.GetCenter();
    Point[Face / 2] = (Face % 2) ? Bounds.Max[Face / 2] : Bounds.Min[Face / 2];
    return Point;
}

FTransform OneSideScale::ScaleFromFace(const FTransform& Start, const FBox& Bounds,
    int32 Face, double OutwardDistance, double GridSize)
{
    check(Face >= 0 && Face < 6);
    const int32 Axis = Face / 2;
    const double LocalLength = Bounds.GetSize()[Axis];
    FVector Scale = Start.GetScale3D();
    if (GridSize > UE_SMALL_NUMBER)
    {
        OutwardDistance = FMath::GridSnap(OutwardDistance, GridSize);
    }
    // Retain mirrored transforms, but never allow a handle to cross its anchor plane.
    const double NewLength = FMath::Max(0.01, LocalLength * FMath::Abs(Scale[Axis]) + OutwardDistance);
    Scale[Axis] = FMath::Sign(Scale[Axis]) * NewLength / LocalLength;
    const FVector Anchor = FaceCenter(Bounds, Face ^ 1);
    FTransform Result(Start.GetRotation(), FVector::ZeroVector, Scale);
    // Every point on the opposite face has this same axis coordinate, so the whole plane stays fixed.
    Result.SetTranslation(Start.TransformPosition(Anchor) - Result.TransformVector(Anchor));
    return Result;
}

FOneSideScaleDrag::FOneSideScaleDrag() = default;
FOneSideScaleDrag::~FOneSideScaleDrag() { End(true); }

bool FOneSideScaleDrag::Begin(AActor* Actor, int32 Face)
{
    if (IsActive() || Face < 0 || Face >= 6 || !OneSideScale::GetLocalBounds(Actor, Bounds))
    {
        return false;
    }
    Target = Actor;
    ActiveFace = Face;
    StartTransform = Actor->GetActorTransform();
    Transaction = MakeUnique<FScopedTransaction>(NSLOCTEXT("OneSideScale", "Transaction", "One-sided scale"));
    Actor->Modify();
    Actor->GetRootComponent()->Modify();
    return true;
}

void FOneSideScaleDrag::Update(double OutwardDistance, double GridSize)
{
    if (IsActive() && Target.IsValid())
    {
        Target->SetActorTransform(OneSideScale::ScaleFromFace(StartTransform, Bounds, ActiveFace,
            OutwardDistance, GridSize), false, nullptr, ETeleportType::TeleportPhysics);
        Target->PostEditMove(false);
        GEditor->RedrawLevelEditingViewports();
    }
}

void FOneSideScaleDrag::End(bool bCancel)
{
    if (!IsActive()) { return; }
    if (Target.IsValid())
    {
        const bool bUnchanged = Target->GetActorTransform().Equals(StartTransform);
        if (bCancel) { Target->SetActorTransform(StartTransform, false, nullptr, ETeleportType::TeleportPhysics); }
        Target->PostEditMove(true);
        // Our ITF drag bypasses the native gizmo's completion path. Refresh its cached pivot
        // from the selected actor's final transform and existing pivot offset before returning control.
        if (GUnrealEd && Target->IsSelected()) { GUnrealEd->UpdatePivotLocationForSelection(); }
        bCancel |= bUnchanged;
    }
    if (bCancel) { Transaction->Cancel(); }
    Transaction.Reset();
    Target.Reset();
    if (GEditor) { GEditor->RedrawLevelEditingViewports(); }
}
