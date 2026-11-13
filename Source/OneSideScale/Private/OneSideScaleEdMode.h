#pragma once

#include "Tools/LegacyEdModeWidgetHelpers.h"
#include "OneSideScaleEdMode.generated.h"

class UOneSideScaleGizmo;

UCLASS(Transient)
class UOneSideScaleEdMode : public UBaseLegacyWidgetEdMode
{
    GENERATED_BODY()
public:
    static const FEditorModeID ModeId;
    static bool IsToolActive();
    static bool CanChangeTool(const FEditorViewportClient& ViewportClient);
    static void ActivateTool(FEditorViewportClient& ViewportClient);
    static void CycleTool(FEditorViewportClient& ViewportClient);
    UOneSideScaleEdMode();
    virtual void Enter() override;
    virtual void Exit() override;
    virtual bool UsesToolkits() const override { return false; }
    virtual bool ShouldDrawWidget() const override { return true; }
    virtual bool UsesTransformWidget() const override { return false; }
    virtual bool UsesTransformWidget(UE::Widget::EWidgetMode Mode) const override { return false; }
    virtual EAxisList::Type GetWidgetAxisToDraw(UE::Widget::EWidgetMode Mode) const override;
    virtual bool RequiresLegacyViewportInteractions() const override { return false; }
    virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;
    virtual bool CanAutoSave() const override;
    bool CancelActiveDrag();
    bool IsDragging() const;

private:
    void OnNativeWidgetModeChanged(UE::Widget::EWidgetMode Mode);

    UPROPERTY(Transient)
    TObjectPtr<UOneSideScaleGizmo> Gizmo;
};
