#include "OneSideScaleToolbar.h"

#include "Brushes/SlateImageBrush.h"
#include "EditorViewportCommands.h"
#include "OneSideScaleEdMode.h"
#include "Interfaces/IPluginManager.h"
#include "SEditorViewport.h"
#include "Styling/AppStyle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/ToolBarStyle.h"
#include "ToolMenus.h"
#include "ViewportToolbar/UnrealEdViewportToolbarContext.h"

#define LOCTEXT_NAMESPACE "OneSideScaleToolbar"

void FOneSideScaleToolbar::Register()
{
    Style = MakeShared<FSlateStyleSet>("OneSideScaleStyle");
    Style->SetContentRoot(IPluginManager::Get().FindPlugin(TEXT("OneSideScale"))->GetBaseDir() / TEXT("Resources"));

    const FString IconPath = Style->RootToContentDir(TEXT("icon"), TEXT(".png"));
    // The square PNG uses the same 16x16 Slate layout as native Move and Rotate.
    const FVector2D IconSize(FAppStyle::Get().GetWidgetStyle<FToolBarStyle>("ViewportToolbar.TransformTools").IconSize);
    Style->Set("OneSideScale.Scale", new FSlateImageBrush(IconPath, IconSize));
    Style->Set("OneSideScale.Scale.Small", new FSlateImageBrush(IconPath, IconSize));
    FSlateStyleRegistry::RegisterSlateStyle(*Style);

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FOneSideScaleToolbar::RegisterMenus));
}

void FOneSideScaleToolbar::RegisterMenus()
{
    FToolMenuOwnerScoped Owner(this);
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.ViewportToolbar.Transform");
    Menu->AddDynamicSection("OneSideScale", FNewToolMenuDelegate::CreateRaw(this, &FOneSideScaleToolbar::BuildTransformMenu));
}

void FOneSideScaleToolbar::BuildTransformMenu(UToolMenu* Menu)
{
    const TSharedPtr<SEditorViewport> Viewport = UUnrealEdViewportToolbarContext::GetEditorViewport(Menu->Context);
    if (!Viewport) { return; }
    const TWeakPtr<FEditorViewportClient> WeakClient = Viewport->GetViewportClient();
    FToolMenuSection& Section = Menu->FindOrAddSection("TransformTools");

    // Keep native commands, icons, and execution; only suppress their checked state during the fourth tool.
    const FEditorViewportCommands& Commands = FEditorViewportCommands::Get();
    const TSharedRef<FUICommandList> DisplayCommands = MakeShared<FUICommandList>();
    for (const TSharedPtr<FUICommandInfo>& Command : { Commands.SelectMode, Commands.TranslateMode,
        Commands.RotateMode, Commands.ScaleMode, Commands.TranslateRotateMode, Commands.TranslateRotate2DMode })
    {
        if (const FUIAction* NativeAction = Viewport->GetCommandList()->GetActionForCommand(Command))
        {
            FUIAction Action = *NativeAction;
            const FUIAction Original = *NativeAction;
            Action.GetActionCheckState = FGetActionCheckState::CreateLambda([Original]()
            {
                return UOneSideScaleEdMode::IsToolActive() ? ECheckBoxState::Unchecked : Original.GetCheckState();
            });
            DisplayCommands->MapAction(Command, Action);
            FToolMenuEntry NativeEntry = FToolMenuEntry::InitMenuEntryWithCommandList(Command, DisplayCommands);
            NativeEntry.SetShowInToolbarTopLevel(true);
            NativeEntry.ToolBarData.StyleNameOverride = "ViewportToolbar.TransformTools";
            Section.AddEntry(NativeEntry);
        }
    }

    FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
        "OneSideScale",
        LOCTEXT("Label", "OneSideScale"),
        LOCTEXT("Tooltip", "Scale one side of the selected actor's bounds (T)"),
        FSlateIcon(Style->GetStyleSetName(), "OneSideScale.Scale", "OneSideScale.Scale.Small"),
        FUIAction(
            FExecuteAction::CreateLambda([WeakClient]()
            {
                if (const TSharedPtr<FEditorViewportClient> Client = WeakClient.Pin())
                {
                    UOneSideScaleEdMode::ActivateTool(*Client);
                }
            }),
            FCanExecuteAction::CreateLambda([WeakClient]()
            {
                const TSharedPtr<FEditorViewportClient> Client = WeakClient.Pin();
                return Client && UOneSideScaleEdMode::CanChangeTool(*Client);
            }),
            FIsActionChecked::CreateStatic(&UOneSideScaleEdMode::IsToolActive)),
        EUserInterfaceActionType::RadioButton);
    Entry.InsertPosition = FToolMenuInsert("ScaleMode", EToolMenuInsertType::After);
    Entry.SetShowInToolbarTopLevel(true);
    Entry.ToolBarData.StyleNameOverride = "ViewportToolbar.TransformTools";
    Section.AddEntry(Entry);
}

void FOneSideScaleToolbar::Unregister()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    if (Style)
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*Style);
        Style.Reset();
    }
}

#undef LOCTEXT_NAMESPACE
