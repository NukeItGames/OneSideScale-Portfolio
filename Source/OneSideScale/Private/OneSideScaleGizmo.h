#pragma once

#include "InteractiveGizmo.h"
#include "InteractiveGizmoBuilder.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"
#include "OneSideScaleTarget.h"
#include "OneSideScaleGizmo.generated.h"

class UMaterialInstanceDynamic;

UCLASS(Transient)
class UOneSideScaleGizmo : public UInteractiveGizmo, public IClickDragBehaviorTarget
{
    GENERATED_BODY()
public:
    virtual void Setup() override;
    virtual void Shutdown() override;
    virtual void Render(IToolsContextRenderAPI* RenderAPI) override;
    virtual void Tick(float DeltaTime) override;
    virtual FInputRayHit CanBeginClickDragSequence(const FInputDeviceRay& PressPos) override;
    virtual void OnClickPress(const FInputDeviceRay& PressPos) override;
    virtual void OnClickDrag(const FInputDeviceRay& DragPos) override;
    virtual void OnClickRelease(const FInputDeviceRay& ReleasePos) override;
    virtual void OnTerminateDragSequence() override;
    bool IsDragging() const { return Drag.IsActive(); }
    void CancelDrag();

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> HandleMaterials;

    AActor* GetTarget() const;
    int32 HitFace(const FInputDeviceRay& Ray, double& OutDepth) const;
    bool AxisParameter(const FRay& Ray, double& OutParameter) const;
    double HandleRadius(const FVector& Position) const;
    void UpdateHover(const FInputDeviceRay& Ray);
    FOneSideScaleDrag Drag;
    TWeakObjectPtr<AActor> DragActor;
    FVector AxisOrigin = FVector::ZeroVector;
    FVector AxisDirection = FVector::XAxisVector;
    double StartParameter = 0;
    int32 ActiveFace = INDEX_NONE;
    int32 HoverFace = INDEX_NONE;
    bool bScreenSpaceDrag = false;
    FVector2D StartScreenPosition = FVector2D::ZeroVector;
    double WorldUnitsPerPixel = 1.0;
};

UCLASS(Transient)
class UOneSideScaleGizmoBuilder : public UInteractiveGizmoBuilder
{
    GENERATED_BODY()
public:
    virtual UInteractiveGizmo* BuildGizmo(const FToolBuilderState& SceneState) const override;
};
