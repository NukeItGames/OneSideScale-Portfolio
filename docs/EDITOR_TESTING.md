# Reproducing Unreal validation

No Unreal compilation, automation, manual viewport test, screenshot, or recording
was performed for the initial portfolio publication. These are the checks still
to run on a licensed Unreal workstation. Record the engine Build.version,
operating system, toolchain, commit, and actual results.

## Build and automation

Generate and build the independent host from the README. Open it, create a new
unsaved empty level, and stay in **Selection** mode. Use a fresh session: the
interaction test temporarily changes selection and mode and requires a Level
Editor viewport. Open Tools → Test Automation (or Session Frontend → Automation,
depending on engine version), filter `OneSideScale`, and run all five tests:

| Test | Intended coverage |
| --- | --- |
| `OneSideScale.Geometry.AnchoredFaces` | Six faces, rotated/off-center bounds, mirrored/nonuniform scale, minimum width |
| `OneSideScale.Geometry.GridSnap` | Snapping along a rotated local axis |
| `OneSideScale.Actors.StaticMeshUndoRedo` | One transaction, cancel, undo/redo, unchanged mesh and pivot |
| `OneSideScale.Actors.DynamicMeshOffsetPivot` | Offset dynamic geometry, mirrored scale, undo/redo, unchanged topology |
| `OneSideScale.Interaction.EscapeCancelsCapture` | ITF face capture and Slate Escape cancellation |

Geometry tests define 216 anchored-face combinations and six snapping cases.
These are existing test definitions, **not passing results**. The interaction test
does not verify the complete toolbar/Space-cycle integration added later.

From an already-built host, an alternative Windows launch is:

```powershell
$EngineRoot = Read-Host 'Absolute path to the Unreal Engine installation'
$HostProject = (Resolve-Path ../OneSideScaleHost/OneSideScaleHost.uproject).Path
$ReportPath = Join-Path (Split-Path $HostProject) 'Saved/Automation/OneSideScale'
& "$EngineRoot/Engine/Binaries/Win64/UnrealEditor.exe" $HostProject -unattended -nop4 -nosplash '-ExecCmds=Automation RunTests OneSideScale' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$ReportPath" -log
```

Use the full editor with a display/GPU, not `UnrealEditor-Cmd`, `-NullRHI`, or a
commandlet for interaction coverage. This launch itself is unverified; if no
viewport exists or initialization races, run through the UI. Require **all five
named tests** to pass in the exported report and log; exit code alone is
insufficient. Skipped, undiscovered, or timed-out tests are not passing tests.

## Actual-tool walkthrough and capture

1. Add an Engine basic Cube to a new unsaved empty level. Select it; note its
   transform/dimensions. Activate OneSideScale via its toolbar button.
2. Drag all six face handles. Verify the opposite face stays fixed and other
   dimensions/rotation remain unchanged. Test growth and shrinkage.
3. Toggle translation grid snapping, change increments, rotate the cube, and
   repeat. Check snapping along its local axis, not world grid planes.
4. Commit a drag, undo, then redo. Confirm location and scale restore. Start
   another drag, press Escape, then release the mouse; verify no new undo entry.
5. Test mirrored/nonuniform scale and a custom pivot offset. Check unchanged
   geometry and stored pivot. Use the development-only console command
   `OneSideScale.Test.CreateDynamicBlock` in an unsaved `/Temp/` editor level for
   an off-center Dynamic Mesh fixture.
6. Verify T activation, repeated clicks, all four Space-cycle steps, W/E/R native
   handoff, right-mouse navigation, and no interception in text fields, asset
   editors, other modes, or PIE. Check native gizmo visibility.
7. Try no selection, multi-selection, attached/locked actors, and unsupported
   actors. Confirm no invalid edit or crash. Restore native Scale.
8. Record a face drag, fixed anchor, undo/redo, Escape cancellation, and native
   handoff. Capture a still of the actual tool. Use procedural geometry/engine
   primitives, hide local paths/account information, and review media for private
   data before publication.

Add genuine captures under `docs/screenshots/` and `docs/demo.webm` only after
performing the walkthrough. Label engine/platform/version and actual passed
steps. The README's conceptual SVG is not a substitute for these checks.
