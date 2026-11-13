#include "OneSideScaleGizmo.h"

#include "BaseBehaviors/ClickDragBehavior.h"
#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseGizmos/GizmoRenderingUtil.h"
#include "BaseGizmos/GizmoViewContext.h"
#include "ContextObjectStore.h"
#include "Editor.h"
#include "Engine/Engine.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"
#include "InteractiveGizmoManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "OneSideScaleEdMode.h"
#include "OneSideScaleSettings.h"
#include "PrimitiveDrawingUtils.h"
#include "SceneManagement.h"
#include "Settings/LevelEditorViewportSettings.h"

UInteractiveGizmo* UOneSideScaleGizmoBuilder::BuildGizmo(const FToolBuilderState& SceneState) const
{
    return NewObject<UOneSideScaleGizmo>(SceneState.GizmoManager);
}

void UOneSideScaleGizmo::Setup()
{
    Super::Setup();
    // Transient instances reuse the editor's opaque, unlit material without changing assets.
    const FLinearColor Colors[] = {FLinearColor(1, 0.12f, 0.12f), FLinearColor(0.15f, 1, 0.15f),
        FLinearColor(0.15f, 0.4f, 1), FLinearColor::Yellow};
    for (const FLinearColor& Color : Colors)
    {
        UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(GEngine->ArrowMaterial, this);
        Material->SetVectorParameterValue(TEXT("GizmoColor"), Color);
        HandleMaterials.Add(Material);
    }
    UClickDragInputBehavior* Behavior = NewObject<UClickDragInputBehavior>(this);
    Behavior->Initialize(this);
    Behavior->ModifierCheckFunc = [](const FInputDeviceState& Input) { return !Input.bAltKeyDown; };
    AddInputBehavior(Behavior);
    ULocalMouseHoverBehavior* Hover = NewObject<ULocalMouseHoverBehavior>(this);
    Hover->Initialize();
    Hover->BeginHitTestFunc = [this](const FInputDeviceRay& Ray) { return CanBeginClickDragSequence(Ray); };
    Hover->OnBeginHoverFunc = [this](const FInputDeviceRay& Ray) { UpdateHover(Ray); };
    Hover->OnUpdateHoverFunc = [this](const FInputDeviceRay& Ray) { UpdateHover(Ray); return HoverFace != INDEX_NONE; };
    Hover->OnEndHoverFunc = [this]() { HoverFace = INDEX_NONE; GEditor->RedrawLevelEditingViewports(); };
    AddInputBehavior(Hover);
}

void UOneSideScaleGizmo::Shutdown()
{
    CancelDrag();
    Super::Shutdown();
}

AActor* UOneSideScaleGizmo::GetTarget() const
{
    if (!UOneSideScaleEdMode::IsToolActive() || GEditor->GetSelectedActorCount() != 1) { return nullptr; }
    AActor* Actor = Cast<AActor>(GEditor->GetSelectedActors()->GetSelectedObject(0));
    FBox Bounds;
    return OneSideScale::GetLocalBounds(Actor, Bounds) ? Actor : nullptr;
}

void UOneSideScaleGizmo::Tick(float DeltaTime)
{
    if (IsDragging() && GetTarget() != DragActor.Get()) { CancelDrag(); }
}

double UOneSideScaleGizmo::HandleRadius(const FVector& Position) const
{
    if (const UGizmoViewContext* View = GetGizmoManager()->GetContextObjectStore()->FindContext<UGizmoViewContext>())
    {
        return FMath::Max(0.01, UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale<double>(View, Position, true) *
            GetDefault<UOneSideScaleSettings>()->HandleHalfSizePixels);
    }
    FViewCameraState Camera;
    GetGizmoManager()->GetContextQueriesAPI()->GetCurrentViewState(Camera);
    const double Width = Camera.bIsOrthographic ? Camera.OrthoWorldCoordinateWidth :
        2.0 * FMath::Abs(FVector::DotProduct(Position - Camera.Position, Camera.Forward())) *
        FMath::Tan(FMath::DegreesToRadians(Camera.HorizontalFOVDegrees * 0.5));
    return FMath::Max(0.01, Width * 0.007);
}

void UOneSideScaleGizmo::Render(IToolsContextRenderAPI* RenderAPI)
{
    AActor* Actor = GetTarget();
    FBox Bounds;
    if (!Actor || !OneSideScale::GetLocalBounds(Actor, Bounds)) { return; }
    const FTransform Transform = Actor->GetActorTransform();
    FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();
    DrawWireBox(PDI, Transform.ToMatrixWithScale(), Bounds, FLinearColor(0.65f, 0.65f, 0.65f), SDPG_Foreground);
    for (int32 Face = 0; Face < 6; ++Face)
    {
        const FVector Center = Transform.TransformPosition(OneSideScale::FaceCenter(Bounds, Face));
        const double Radius = UE::GizmoRenderingUtil::CalculateLocalPixelToWorldScale(RenderAPI->GetSceneView(), Center) *
            GetDefault<UOneSideScaleSettings>()->HandleHalfSizePixels;
        const int32 MaterialIndex = ActiveFace == Face || HoverFace == Face ? 3 : Face / 2;
        DrawSphere(PDI, Center, FRotator::ZeroRotator, FVector(FMath::Max(0.01, Radius)),
            24, 16, HandleMaterials[MaterialIndex]->GetRenderProxy(), SDPG_Foreground);
    }
}

void UOneSideScaleGizmo::UpdateHover(const FInputDeviceRay& Ray)
{
    double Depth;
    const int32 Face = HitFace(Ray, Depth);
    if (HoverFace != Face)
    {
        HoverFace = Face;
        GEditor->RedrawLevelEditingViewports();
    }
}

int32 UOneSideScaleGizmo::HitFace(const FInputDeviceRay& Ray, double& OutDepth) const
{
    AActor* Actor = GetTarget();
    FBox Bounds;
    OutDepth = TNumericLimits<double>::Max();
    if (!Actor || !OneSideScale::GetLocalBounds(Actor, Bounds)) { return INDEX_NONE; }
    int32 BestFace = INDEX_NONE;
    for (int32 Face = 0; Face < 6; ++Face)
    {
        const FVector Center = Actor->GetActorTransform().TransformPosition(OneSideScale::FaceCenter(Bounds, Face));
        const double Depth = FVector::DotProduct(Center - Ray.WorldRay.Origin, Ray.WorldRay.Direction);
        // A small screen-space margin helps acquisition; nearest sphere surface wins overlaps.
        const double Radius = HandleRadius(Center) * 1.25;
        const double DistanceSquared = FVector::DistSquared(Center, Ray.WorldRay.PointAt(Depth));
        if (DistanceSquared > Radius * Radius) { continue; }
        const double HalfChord = FMath::Sqrt(FMath::Max(0.0, Radius * Radius - DistanceSquared));
        if (Depth + HalfChord < 0.0) { continue; }
        const double HitDepth = FMath::Max(0.0, Depth - HalfChord);
        if (HitDepth < OutDepth)
        {
            OutDepth = HitDepth;
            BestFace = Face;
        }
    }
    return BestFace;
}

FInputRayHit UOneSideScaleGizmo::CanBeginClickDragSequence(const FInputDeviceRay& PressPos)
{
    double Depth;
    return HitFace(PressPos, Depth) != INDEX_NONE ? FInputRayHit(Depth) : FInputRayHit();
}

bool UOneSideScaleGizmo::AxisParameter(const FRay& Ray, double& OutParameter) const
{
    const double Dot = FVector::DotProduct(AxisDirection, Ray.Direction);
    const double Denominator = 1.0 - Dot * Dot;
    // A face viewed exactly end-on has no projected drag axis; avoid an unstable jump.
    if (Denominator < 0.0001) { return false; }
    const FVector Offset = Ray.Origin - AxisOrigin;
    OutParameter = (FVector::DotProduct(Offset, AxisDirection) - Dot * FVector::DotProduct(Offset, Ray.Direction)) / Denominator;
    return true;
}

void UOneSideScaleGizmo::OnClickPress(const FInputDeviceRay& PressPos)
{
    double Depth;
    const int32 Face = HitFace(PressPos, Depth);
    AActor* Actor = GetTarget();
    if (Face == INDEX_NONE || !Actor) { return; }
    const FTransform Transform = Actor->GetActorTransform();
    AxisDirection = Transform.GetUnitAxis(static_cast<EAxis::Type>(Face / 2 + 1)) *
        ((Face % 2) ? 1.0 : -1.0) * FMath::Sign(Transform.GetScale3D()[Face / 2]);
    FBox Bounds;
    OneSideScale::GetLocalBounds(Actor, Bounds);
    AxisOrigin = Transform.TransformPosition(OneSideScale::FaceCenter(Bounds, Face));
    bScreenSpaceDrag = !AxisParameter(PressPos.WorldRay, StartParameter);
    StartScreenPosition = PressPos.ScreenPosition;
    WorldUnitsPerPixel = HandleRadius(AxisOrigin) / GetDefault<UOneSideScaleSettings>()->HandleHalfSizePixels;
    if ((!bScreenSpaceDrag || PressPos.bHas2D) && Drag.Begin(Actor, Face))
    {
        DragActor = Actor;
        ActiveFace = Face;
    }
}

void UOneSideScaleGizmo::OnClickDrag(const FInputDeviceRay& DragPos)
{
    double Parameter;
    const bool bHasParameter = bScreenSpaceDrag ? DragPos.bHas2D : AxisParameter(DragPos.WorldRay, Parameter);
    if (IsDragging() && bHasParameter)
    {
        const double Grid = GetDefault<ULevelEditorViewportSettings>()->GridEnabled ? GEditor->GetGridSize() : 0.0;
        // End-on axes have no screen projection: upward motion consistently grows that face.
        const double Distance = bScreenSpaceDrag ? (StartScreenPosition.Y - DragPos.ScreenPosition.Y) * WorldUnitsPerPixel :
            Parameter - StartParameter;
        Drag.Update(Distance, Grid);
    }
}

void UOneSideScaleGizmo::OnClickRelease(const FInputDeviceRay& ReleasePos)
{
    if (IsDragging()) { OnClickDrag(ReleasePos); }
    Drag.End(false);
    ActiveFace = INDEX_NONE;
    DragActor.Reset();
}

void UOneSideScaleGizmo::OnTerminateDragSequence() { CancelDrag(); }
void UOneSideScaleGizmo::CancelDrag()
{
    Drag.End(true);
    ActiveFace = INDEX_NONE;
    DragActor.Reset();
}
