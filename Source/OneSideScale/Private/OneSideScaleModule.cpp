#include "Modules/ModuleManager.h"

#include "Editor.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "EditorViewportClient.h"
#include "EditorViewportCommands.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "IAssetViewport.h"
#include "LevelEditor.h"
#include "Misc/CoreDelegates.h"
#include "OneSideScaleEdMode.h"
#include "OneSideScaleToolbar.h"
#include "Widgets/SViewport.h"

// Only a focused Level Editor viewport participates; text fields and asset editors retain their bindings.
class FOneSideScaleViewportInputProcessor : public IInputProcessor
{
public:
    virtual void Tick(float, FSlateApplication&, TSharedRef<ICursor>) override {}
    virtual bool HandleKeyDownEvent(FSlateApplication& Slate, const FKeyEvent& Event) override
    {
        const FKey Key = Event.GetKey();
        const bool bUndoOrRedo = (Event.IsControlDown() || Event.IsCommandDown()) &&
            (Key == EKeys::Z || Key == EKeys::Y);
        FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
        const TSharedPtr<IAssetViewport> Viewport = LevelEditor ? LevelEditor->GetFirstActiveViewport() : nullptr;
        const TSharedPtr<SViewport> Widget = Viewport ? Viewport->GetViewportWidget().Pin() : nullptr;
        if (!GEditor || GEditor->PlayWorld || !Widget || !Widget->HasKeyboardFocus()) { return false; }

        UOneSideScaleEdMode* Mode = Cast<UOneSideScaleEdMode>(
            GLevelEditorModeTools().GetActiveScriptableMode(UOneSideScaleEdMode::ModeId));
        const FInputChord Chord(Key, Event.IsShiftDown(), Event.IsControlDown(), Event.IsAltDown(), Event.IsCommandDown());
        const FEditorViewportCommands& Commands = FEditorViewportCommands::Get();
        const bool bActivate = Chord == FInputChord(EKeys::T);
        const bool bCycle = Commands.CycleTransformGizmos->HasActiveChord(Chord);
        FEditorViewportClient& Client = Viewport->GetAssetViewportClient();
        if (GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Default) &&
            Client.GetModeTools() == &GLevelEditorModeTools())
        {
            // Let Unreal route every navigation key, including Space and vertical-flight E, normally.
            const bool bNavigating = Slate.GetPressedMouseButtons().Contains(EKeys::RightMouseButton) ||
                Client.IsFlightCameraActive() || Client.IsMovingCamera();
            if (!bNavigating)
            {
                const bool bNativeTransform = Commands.TranslateMode->HasActiveChord(Chord) ||
                    Commands.RotateMode->HasActiveChord(Chord) || Commands.ScaleMode->HasActiveChord(Chord);
                if (Mode && Mode->IsDragging() && (bActivate || bCycle || bNativeTransform)) { return true; }
                if (bActivate || bCycle)
                {
                    if (!Event.IsRepeat() && UOneSideScaleEdMode::CanChangeTool(Client))
                    {
                        if (bActivate) { UOneSideScaleEdMode::ActivateTool(Client); }
                        else { UOneSideScaleEdMode::CycleTool(Client); }
                    }
                    // T takes priority over translucent selection here; suppress repeats and native Space cycling.
                    return true;
                }
            }
        }
        if (Key != EKeys::Escape && Key != EKeys::Delete && Key != EKeys::BackSpace && !bUndoOrRedo)
        {
            return false;
        }
        const bool bCancelled = Mode && Mode->CancelActiveDrag();
        // Never undo another edit underneath an open drag transaction. Idle Escape remains native.
        return bCancelled && (Key == EKeys::Escape || bUndoOrRedo);
    }
};

class FOneSideScaleModule : public IModuleInterface
{
public:
    virtual bool SupportsDynamicReloading() override { return false; }
    virtual void StartupModule() override
    {
        if (!IsRunningCommandlet())
        {
            Toolbar.Register();
            FEditorDelegates::OnEditorInitialized.AddRaw(this, &FOneSideScaleModule::InitializeEditorIntegration);
            FCoreDelegates::OnEnginePreExit.AddRaw(this, &FOneSideScaleModule::RemoveEditorIntegration);
        }
    }

    virtual void ShutdownModule() override
    {
        FEditorDelegates::OnEditorInitialized.RemoveAll(this);
        FCoreDelegates::OnEnginePreExit.RemoveAll(this);
        RemoveEditorIntegration();
    }

private:
    void InitializeEditorIntegration(double)
    {
        ModeTools = &GLevelEditorModeTools();
        if (FSlateApplication::IsInitialized())
        {
            InputProcessor = MakeShared<FOneSideScaleViewportInputProcessor>();
            FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
        }
    }

    void RemoveEditorIntegration()
    {
        Toolbar.Unregister();
        if (FSlateApplication::IsInitialized() && InputProcessor)
        {
            FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
        }
        InputProcessor.Reset();
        if (ModeTools)
        {
            ModeTools->DestroyMode(UOneSideScaleEdMode::ModeId);
            ModeTools = nullptr;
        }
    }

    FEditorModeTools* ModeTools = nullptr;
    FOneSideScaleToolbar Toolbar;
    TSharedPtr<FOneSideScaleViewportInputProcessor> InputProcessor;
};

IMPLEMENT_MODULE(FOneSideScaleModule, OneSideScale)
