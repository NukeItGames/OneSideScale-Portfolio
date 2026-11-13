#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "OneSideScaleTarget.h"
#include "Components/StaticMeshComponent.h"
#include "DynamicMeshActor.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Editor.h"
#include "Editor/TransBuffer.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "Engine/Selection.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Generators/MinimalBoxMeshGenerator.h"
#include "BaseGizmos/GizmoViewContext.h"
#include "ContextObjectStore.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "IAssetViewport.h"
#include "InputRouter.h"
#include "InteractiveGizmoManager.h"
#include "LevelEditor.h"
#include "OneSideScaleEdMode.h"
#include "OneSideScaleGizmo.h"
#include "Tools/EdModeInteractiveToolsContext.h"
#include "Widgets/SViewport.h"
#include "UObject/StrongObjectPtr.h"

namespace
{
    constexpr EAutomationTestFlags TestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

    /** Keeps both test objects and undo history separate from the user's editor session. */
    struct FTestScene
    {
        TStrongObjectPtr<UTransactor> OriginalTransactions{GEditor->Trans};
        TStrongObjectPtr<UTransBuffer> Transactions{NewObject<UTransBuffer>()};
        UWorld* World = nullptr;

        FTestScene()
        {
            Transactions->Initialize(8 * 1024 * 1024);
            GEditor->Trans = Transactions.Get();
            const UWorld::InitializationValues Values = UWorld::InitializationValues()
                .AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false)
                .CreateAISystem(false).ShouldSimulatePhysics(false);
            const FName WorldName = MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("OneSideScaleTestWorld"));
            World = UWorld::CreateWorld(EWorldType::EditorPreview, false, WorldName,
                GetTransientPackage(), true, ERHIFeatureLevel::Num, &Values);
        }
        ~FTestScene()
        {
            Transactions->Reset(NSLOCTEXT("OneSideScale", "ResetTests", "Finished OneSideScale tests"));
            GEditor->Trans = OriginalTransactions.Get();
            World->DestroyWorld(false);
        }
    };

    bool CheckAnchor(FAutomationTestBase& Test, const FTransform& Before, const FTransform& After,
        const FBox& Bounds, int32 Face)
    {
        bool bResult = true;
        const int32 Axis = Face / 2;
        // Check all four corners, not just a center that could hide rotation or shear errors.
        for (int32 Corner = 0; Corner < 4; ++Corner)
        {
            FVector Point = OneSideScale::FaceCenter(Bounds, Face ^ 1);
            Point[(Axis + 1) % 3] = Corner & 1 ? Bounds.Max[(Axis + 1) % 3] : Bounds.Min[(Axis + 1) % 3];
            Point[(Axis + 2) % 3] = Corner & 2 ? Bounds.Max[(Axis + 2) % 3] : Bounds.Min[(Axis + 2) % 3];
            bResult &= Test.TestTrue(TEXT("Opposite face corner stays fixed within 1e-7 cm"),
                Before.TransformPosition(Point).Equals(After.TransformPosition(Point), 1.e-7));
        }
        bResult &= Test.TestTrue(TEXT("Rotation preserved"), Before.GetRotation().Equals(After.GetRotation(), 1.e-12));
        for (int32 Other = 0; Other < 3; ++Other)
        {
            if (Other != Axis)
            {
                bResult &= Test.TestEqual(TEXT("Other axis scale preserved"), After.GetScale3D()[Other], Before.GetScale3D()[Other]);
            }
        }
        return bResult;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOneSideScaleMathTest, "OneSideScale.Geometry.AnchoredFaces", TestFlags)
bool FOneSideScaleMathTest::RunTest(const FString& Parameters)
{
    const FBox Boxes[] = {FBox(FVector(-50), FVector(50)), FBox(FVector(13, -97, 20), FVector(113, 33, 220))};
    const FQuat Rotations[] = {FQuat::Identity, FRotator(31, 47, -19).Quaternion()};
    const FVector Scales[] = {FVector::OneVector, FVector(2, 0.3, 4), FVector(-2, 3, -0.7)};
    for (const FBox& Bounds : Boxes)
    for (const FQuat& Rotation : Rotations)
    for (const FVector& Scale : Scales)
    for (int32 Face = 0; Face < 6; ++Face)
    for (double Distance : {37.25, -10.0, -100000.0})
    {
        const FTransform Before(Rotation, FVector(12345, -723, 500), Scale);
        const FTransform After = OneSideScale::ScaleFromFace(Before, Bounds, Face, Distance, 0);
        CheckAnchor(*this, Before, After, Bounds, Face);
        const double Width = Bounds.GetSize()[Face / 2] * FMath::Abs(After.GetScale3D()[Face / 2]);
        const double Expected = FMath::Max(0.01, Bounds.GetSize()[Face / 2] * FMath::Abs(Scale[Face / 2]) + Distance);
        TestTrue(TEXT("Dragged face changes dimension by requested world distance"), FMath::IsNearlyEqual(Width, Expected, 1.e-7));
        TestTrue(TEXT("Mirroring sign preserved"), FMath::Sign(Scale[Face / 2]) == FMath::Sign(After.GetScale3D()[Face / 2]));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOneSideScaleSnapTest, "OneSideScale.Geometry.GridSnap", TestFlags)
bool FOneSideScaleSnapTest::RunTest(const FString& Parameters)
{
    const FBox Bounds(FVector(13, -97, 20), FVector(113, 33, 220));
    const FTransform Before(FRotator(31, 47, -19), FVector(123, 42, -12), FVector(2, 3, 4));
    for (int32 Face = 0; Face < 6; ++Face)
    {
        const FTransform After = OneSideScale::ScaleFromFace(Before, Bounds, Face, 26, 10);
        CheckAnchor(*this, Before, After, Bounds, Face);
        const FVector Point = OneSideScale::FaceCenter(Bounds, Face);
        TestTrue(TEXT("26 cm drag snaps to 30 cm along the rotated axis"),
            FMath::IsNearlyEqual(FVector::Distance(Before.TransformPosition(Point), After.TransformPosition(Point)), 30.0, 1.e-7));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOneSideScaleStaticActorTest, "OneSideScale.Actors.StaticMeshUndoRedo", TestFlags)
bool FOneSideScaleStaticActorTest::RunTest(const FString& Parameters)
{
    FTestScene Scene;
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!TestNotNull(TEXT("Engine cube available"), Mesh)) { return false; }
    const FBox OriginalMeshBounds = Mesh->GetBoundingBox();
    AStaticMeshActor* Actor = Scene.World->SpawnActor<AStaticMeshActor>();
    Actor->SetFlags(RF_Transactional);
    Actor->GetStaticMeshComponent()->SetFlags(RF_Transactional);
    Actor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    Actor->SetPivotOffset(FVector(82, -19, 7));
    const FVector Pivot = Actor->GetPivotOffset();
    for (const FRotator Rotation : {FRotator::ZeroRotator, FRotator(31, 47, -19)})
    for (int32 Face = 0; Face < 6; ++Face)
    {
        const FTransform Before(Rotation, FVector(300, -50, 17), FVector(2, 0.5, 1.5));
        Actor->SetActorTransform(Before);
        FBox Bounds;
        TestTrue(TEXT("Static actor supported"), OneSideScale::GetLocalBounds(Actor, Bounds));
        const int32 QueueBefore = Scene.Transactions->GetQueueLength();
        FOneSideScaleDrag Drag;
        TestTrue(TEXT("Begin drag"), Drag.Begin(Actor, Face));
        Drag.Update(10, 0);
        Drag.Update(26, 10);
        Drag.End(false);
        const FTransform After = Actor->GetActorTransform();
        CheckAnchor(*this, Before, After, Bounds, Face);
        TestEqual(TEXT("One transaction per drag"), Scene.Transactions->GetQueueLength(), QueueBefore + 1);
        TestTrue(TEXT("Undo succeeds"), GEditor->UndoTransaction());
        TestTrue(TEXT("Undo restores full actor transform"), Actor->GetActorTransform().Equals(Before, 1.e-7));
        TestTrue(TEXT("Redo succeeds"), GEditor->RedoTransaction());
        TestTrue(TEXT("Redo restores full actor transform"), Actor->GetActorTransform().Equals(After, 1.e-7));
        const int32 BeforeCancel = Scene.Transactions->GetQueueLength();
        TestTrue(TEXT("Begin cancellation case"), Drag.Begin(Actor, Face));
        Drag.Update(-22, 0);
        Drag.End(true);
        TestTrue(TEXT("Cancellation restores transform"), Actor->GetActorTransform().Equals(After, 1.e-7));
        TestEqual(TEXT("Cancellation adds no undo entry"), Scene.Transactions->GetQueueLength(), BeforeCancel);
        TestTrue(TEXT("Pivot offset unchanged"), Actor->GetPivotOffset().Equals(Pivot, 0));
        TestTrue(TEXT("Mesh reference unchanged"), Actor->GetStaticMeshComponent()->GetStaticMesh() == Mesh);
        TestTrue(TEXT("Mesh local geometry bounds unchanged"), Mesh->GetBoundingBox().Equals(OriginalMeshBounds, 0));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOneSideScaleDynamicActorTest, "OneSideScale.Actors.DynamicMeshOffsetPivot", TestFlags)
bool FOneSideScaleDynamicActorTest::RunTest(const FString& Parameters)
{
    FTestScene Scene;
    ADynamicMeshActor* Actor = Scene.World->SpawnActor<ADynamicMeshActor>();
    Actor->SetFlags(RF_Transactional);
    Actor->GetDynamicMeshComponent()->SetFlags(RF_Transactional);
    Actor->GetDynamicMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    UE::Geometry::FMinimalBoxMeshGenerator Generator;
    Generator.Box = UE::Geometry::FOrientedBox3d(FVector3d(63, -32, 120), FVector3d(50, 65, 100));
    Generator.Generate();
    UE::Geometry::FDynamicMesh3 Mesh(&Generator);
    const UE::Geometry::FDynamicMesh3 OriginalMesh(Mesh);
    Actor->GetDynamicMeshComponent()->SetMesh(MoveTemp(Mesh));
    Actor->SetPivotOffset(FVector(-10, 8, 17));
    const FVector Pivot = Actor->GetPivotOffset();
    for (const FRotator Rotation : {FRotator::ZeroRotator, FRotator(37, 51, -23)})
    for (int32 Face = 0; Face < 6; ++Face)
    {
        const FTransform Before(Rotation, FVector(135, -471, 62), FVector(-2, 1.5, 3));
        Actor->SetActorTransform(Before);
        FBox Bounds;
        TestTrue(TEXT("Dynamic mesh supported"), OneSideScale::GetLocalBounds(Actor, Bounds));
        TestTrue(TEXT("Offset geometry bounds retained"), Bounds.GetCenter().Equals(FVector(63, -32, 120), 1.e-7));
        FOneSideScaleDrag Drag;
        TestTrue(TEXT("Begin dynamic mesh drag"), Drag.Begin(Actor, Face));
        Drag.Update(37, 0);
        Drag.End(false);
        const FTransform After = Actor->GetActorTransform();
        CheckAnchor(*this, Before, After, Bounds, Face);
        TestTrue(TEXT("Dynamic undo succeeds"), GEditor->UndoTransaction());
        TestTrue(TEXT("Dynamic undo restores transform"), Actor->GetActorTransform().Equals(Before, 1.e-7));
        TestTrue(TEXT("Dynamic redo succeeds"), GEditor->RedoTransaction());
        TestTrue(TEXT("Dynamic redo restores transform"), Actor->GetActorTransform().Equals(After, 1.e-7));
        TestTrue(TEXT("Dynamic pivot offset retained"), Actor->GetPivotOffset().Equals(Pivot, 0));
    }
    Actor->GetDynamicMeshComponent()->ProcessMesh([this, &OriginalMesh](const UE::Geometry::FDynamicMesh3& Result)
    {
        TestEqual(TEXT("Vertex count unchanged"), Result.VertexCount(), OriginalMesh.VertexCount());
        TestEqual(TEXT("Triangle count unchanged"), Result.TriangleCount(), OriginalMesh.TriangleCount());
        for (int32 Id : OriginalMesh.VertexIndicesItr())
        {
            TestTrue(TEXT("Original mesh vertex unchanged"), Result.GetVertex(Id) == OriginalMesh.GetVertex(Id));
        }
        for (int32 Id : OriginalMesh.TriangleIndicesItr())
        {
            TestTrue(TEXT("Topology unchanged"), Result.GetTriangle(Id) == OriginalMesh.GetTriangle(Id));
        }
    });
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FOneSideScaleInteractionTest, "OneSideScale.Interaction.EscapeCancelsCapture", TestFlags)
bool FOneSideScaleInteractionTest::RunTest(const FString& Parameters)
{
    FEditorModeTools& Modes = GLevelEditorModeTools();
    if (!Modes.IsOnlyActiveMode(FBuiltinEditorModes::EM_Default))
    {
        AddError(TEXT("Run interaction test from Selection Mode."));
        return false;
    }
    FTestScene Scene;
    TArray<TWeakObjectPtr<AActor>> PreviousSelection;
    for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
    {
        PreviousSelection.Add(Cast<AActor>(*It));
    }
    struct FRestoreEditor
    {
        TArray<TWeakObjectPtr<AActor>>& Selection;
        ~FRestoreEditor()
        {
            GLevelEditorModeTools().DeactivateMode(UOneSideScaleEdMode::ModeId);
            GEditor->SelectNone(false, true);
            for (const auto& Actor : Selection)
            {
                if (Actor.IsValid()) { GEditor->SelectActor(Actor.Get(), true, false); }
            }
            GEditor->NoteSelectionChange();
        }
    } Restore{PreviousSelection};
    AStaticMeshActor* Actor = Scene.World->SpawnActor<AStaticMeshActor>();
    Actor->SetFlags(RF_Transactional);
    Actor->GetStaticMeshComponent()->SetFlags(RF_Transactional);
    Actor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Actor->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    GEditor->SelectNone(false, true);
    GEditor->SelectActor(Actor, true, true);
    Modes.ActivateMode(UOneSideScaleEdMode::ModeId);
    UOneSideScaleEdMode* Mode = Cast<UOneSideScaleEdMode>(Modes.GetActiveScriptableMode(UOneSideScaleEdMode::ModeId));
    if (!TestNotNull(TEXT("Editor mode registered and entered"), Mode)) { return false; }
    UEditorInteractiveToolsContext* Context = Mode->GetInteractiveToolsContext();
    const TArray<UInteractiveGizmo*> Gizmos = Context->GizmoManager->FindAllGizmosOfType(TEXT("OneSideScale.FaceGizmo"));
    if (!TestEqual(TEXT("Exactly one managed gizmo"), Gizmos.Num(), 1)) { return false; }
    UOneSideScaleGizmo* Gizmo = Cast<UOneSideScaleGizmo>(Gizmos[0]);

    // A predictable view avoids depending on whichever viewport the user last hovered.
    UGizmoViewContext* View = Context->GizmoManager->GetContextObjectStore()->FindContext<UGizmoViewContext>();
    if (!TestNotNull(TEXT("Editor gizmo view context available"), View)) { return false; }
    FSceneViewInitOptions ViewOptions;
    ViewOptions.SetViewRectangle(FIntRect(0, 0, 1000, 1000));
    ViewOptions.ViewOrigin = FVector(0, 0, 500);
    ViewOptions.ViewRotationMatrix = FMatrix::Identity;
    ViewOptions.ProjectionMatrix = FReversedZOrthoMatrix(500, 500, 1.0 / 1000.0, 0);
    View->Reset(FIntRect(0, 0, 1000, 1000), FViewMatrices(ViewOptions), false, ViewOptions.ViewOrigin);

    FInputDeviceState Input;
    Input.InputDevice = EInputDevices::Mouse;
    Input.Mouse.Left.bPressed = true;
    Input.Mouse.Left.bDown = true;
    Input.Mouse.WorldRay = FRay(FVector(50, 0, 500), FVector(0, 0, -1));
    Input.Mouse.Position2D = FVector2D(550, 500);
    Context->InputRouter->PostInputEvent(Input);
    TestTrue(TEXT("ITF captures a face press"), Gizmo->IsDragging());
    Input.Mouse.Left.bPressed = false;
    Input.Mouse.WorldRay.Origin.X = 90;
    Input.Mouse.Position2D.X = 590;
    Context->InputRouter->PostInputEvent(Input);
    TestTrue(TEXT("ITF drag changes the actor"), !Actor->GetActorTransform().Equals(FTransform::Identity));

    const TSharedPtr<IAssetViewport> Viewport = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor").GetFirstActiveViewport();
    const TSharedPtr<SViewport> Widget = Viewport ? Viewport->GetViewportWidget().Pin() : nullptr;
    if (!TestTrue(TEXT("Level viewport exists"), Widget.IsValid())) { return false; }
    FSlateApplication::Get().SetKeyboardFocus(Widget, EFocusCause::SetDirectly);
    FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(EKeys::Escape, FModifierKeysState(), 0, false, 0, 0));
    TestFalse(TEXT("Escape releases ITF drag"), Gizmo->IsDragging());
    TestTrue(TEXT("Escape restores actor transform"), Actor->GetActorTransform().Equals(FTransform::Identity, 1.e-7));
    TestEqual(TEXT("Cancelled capture leaves no undo entry"), Scene.Transactions->GetQueueLength(), 0);
    TestTrue(TEXT("Escape during drag keeps mode active"), Modes.IsModeActive(UOneSideScaleEdMode::ModeId));
    Input.Mouse.Left.bDown = false;
    Input.Mouse.Left.bReleased = true;
    Context->InputRouter->PostInputEvent(Input);
    TestTrue(TEXT("Mouse release after Escape cannot recommit"), Actor->GetActorTransform().Equals(FTransform::Identity, 1.e-7));
    return true;
}

// Explicit, development-only fixture command for manual testing in an unsaved empty level.
static FAutoConsoleCommandWithWorld OneSideScaleDynamicFixture(
    TEXT("OneSideScale.Test.CreateDynamicBlock"),
    TEXT("Create a temporary Dynamic Mesh block. Requires an unsaved /Temp/ level."),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
    {
        if (!World || World->WorldType != EWorldType::Editor || !World->GetOutermost()->GetName().StartsWith(TEXT("/Temp/")))
        {
            UE_LOG(LogTemp, Warning, TEXT("OneSideScale test fixture requires an unsaved empty level."));
            return;
        }
        ADynamicMeshActor* Actor = World->SpawnActor<ADynamicMeshActor>();
        Actor->SetActorLabel(TEXT("OneSideScale_DynamicTestBlock"));
        Actor->GetDynamicMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        UE::Geometry::FMinimalBoxMeshGenerator Generator;
        Generator.Box = UE::Geometry::FOrientedBox3d(FVector3d(25, -10, 15), FVector3d(50, 50, 50));
        Generator.Generate();
        Actor->GetDynamicMeshComponent()->SetMesh(UE::Geometry::FDynamicMesh3(&Generator));
        GEditor->SelectNone(false, true);
        GEditor->SelectActor(Actor, true, true);
        GEditor->RedrawLevelEditingViewports();
    }));

#endif
