#include "OneSideScaleEdMode.h"

#include "EditorModes.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor/UnrealEdEngine.h"
#include "InputRouter.h"
#include "InteractiveGizmoManager.h"
#include "OneSideScaleGizmo.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "UnrealEdGlobals.h"

const FEditorModeID UOneSideScaleEdMode::ModeId(TEXT("EM_OneSideScale"));

UOneSideScaleEdMode::UOneSideScaleEdMode()
{
    Info = FEditorModeInfo(ModeId, NSLOCTEXT("OneSideScale", "ModeName", "One Side Scale"), FSlateIcon(), false);
}

bool UOneSideScaleEdMode::IsToolActive()
{
    return GEditor && !GEditor->PlayWorld &&
        GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Default) &&
        GLevelEditorModeTools().IsModeActive(ModeId);
}

bool UOneSideScaleEdMode::CanChangeTool(const FEditorViewportClient& ViewportClient)
{
    if (!GEditor || GEditor->PlayWorld || ViewportClient.GetModeTools() != &GLevelEditorModeTools() ||
        !GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Default) ||
        GLevelEditorModeTools().IsTracking() || ViewportClient.IsTracking() ||
        ViewportClient.IsMovingCamera() || ViewportClient.IsFlightCameraActive() ||
        FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::RightMouseButton)) { return false; }
    const UOneSideScaleEdMode* Mode = Cast<UOneSideScaleEdMode>(
        GLevelEditorModeTools().GetActiveScriptableMode(ModeId));
    return !Mode || !Mode->IsDragging();
}

void UOneSideScaleEdMode::ActivateTool(FEditorViewportClient& ViewportClient)
{
    if (CanChangeTool(ViewportClient) && !IsToolActive())
    {
        GLevelEditorModeTools().ActivateMode(ModeId);
    }
}

void UOneSideScaleEdMode::CycleTool(FEditorViewportClient& ViewportClient)
{
    if (!CanChangeTool(ViewportClient)) { return; }
    if (IsToolActive()) { ViewportClient.SetWidgetMode(UE::Widget::WM_Translate); }
    else if (ViewportClient.GetWidgetMode() == UE::Widget::WM_Translate)
    {
        ViewportClient.SetWidgetMode(UE::Widget::WM_Rotate);
    }
    else if (ViewportClient.GetWidgetMode() == UE::Widget::WM_Rotate)
    {
        ViewportClient.SetWidgetMode(UE::Widget::WM_Scale);
    }
    else if (ViewportClient.GetWidgetMode() == UE::Widget::WM_Scale) { ActivateTool(ViewportClient); }
    else { ViewportClient.SetWidgetMode(UE::Widget::WM_Translate); }
}

void UOneSideScaleEdMode::OnNativeWidgetModeChanged(UE::Widget::EWidgetMode Mode)
{
    // Native viewport commands already suppress mode changes during camera navigation.
    GLevelEditorModeTools().DeactivateMode(ModeId);
}

EAxisList::Type UOneSideScaleEdMode::GetWidgetAxisToDraw(UE::Widget::EWidgetMode Mode) const
{
    // Both the legacy widget and UE 5.8's TRS gizmo consume this axis mask.
    return EAxisList::None;
}

void UOneSideScaleEdMode::Enter()
{
    Super::Enter();
    // Leave the native mode untouched so toolbar deactivation restores it without a second state copy.
    GLevelEditorModeTools().OnWidgetModeChanged().AddUObject(this, &UOneSideScaleEdMode::OnNativeWidgetModeChanged);
    UInteractiveGizmoManager* Manager = GetInteractiveToolsContext()->GizmoManager;
    Manager->RegisterGizmoType(TEXT("OneSideScale.FaceGizmo"), NewObject<UOneSideScaleGizmoBuilder>(this));
    Gizmo = Cast<UOneSideScaleGizmo>(Manager->CreateGizmo(TEXT("OneSideScale.FaceGizmo"), FString(), this));
    GEditor->RedrawLevelEditingViewports();
}

void UOneSideScaleEdMode::Exit()
{
    GLevelEditorModeTools().OnWidgetModeChanged().RemoveAll(this);
    CancelActiveDrag();
    if (Gizmo)
    {
        GetInteractiveToolsContext()->GizmoManager->DestroyGizmo(Gizmo);
        Gizmo = nullptr;
    }
    Super::Exit();
    if (GUnrealEd) { GUnrealEd->UpdatePivotLocationForSelection(); }
    if (GEditor) { GEditor->RedrawLevelEditingViewports(); }
}

bool UOneSideScaleEdMode::IsCompatibleWith(FEditorModeID OtherModeID) const
{
    return OtherModeID == FBuiltinEditorModes::EM_Default;
}

bool UOneSideScaleEdMode::CanAutoSave() const { return !Gizmo || !Gizmo->IsDragging(); }
bool UOneSideScaleEdMode::IsDragging() const { return Gizmo && Gizmo->IsDragging(); }

bool UOneSideScaleEdMode::CancelActiveDrag()
{
    if (!Gizmo || !Gizmo->IsDragging()) { return false; }
    GetInteractiveToolsContext()->InputRouter->ForceTerminateSource(Gizmo);
    Gizmo->CancelDrag();
    return true;
}
