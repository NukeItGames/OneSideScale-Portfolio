# Controls and technical reference

## Features and controls

- Select one unlocked, unattached Static Mesh Actor or Dynamic Mesh Actor.
- Click **OneSideScale** beside native Scale, or press **T** in a focused Level
  Editor viewport in Selection mode. Drag a red, green, or blue face sphere
  along the actor's local X, Y, or Z axis.
- Release to commit one undo step; **Escape during a drag** restores the starting
  transform. Use Unreal's usual Undo/Redo after a committed drag.
- **W / E / R** select native Move / Rotate / Scale. With default bindings,
  **Space** cycles Move → Rotate → Scale → OneSideScale → Move. The cycle follows
  Unreal's configured Cycle Transform Gizmos binding and ignores key repeats.
- The viewport's **translation grid** toggle and increment control snapping along
  the selected local axis. Percentage Scale Snap is not used.
- Editor Preferences → Plugins → OneSideScale changes handle radius (default
  9 pixels). Hovered/active handles turn yellow. An exactly end-on axis uses
  upward dragging to grow and downward dragging to shrink.

T normally toggles Allow Translucent Selection. This tool takes priority only in
the focused Level Editor viewport in Selection mode outside camera navigation;
the native menu action remains available. Text fields, asset editors, other
modes, and PIE keep their own input handling. Right-mouse camera navigation
retains the active tool. Tool-switch keys are suppressed during a custom drag;
Escape can still cancel it. The author confirms personal use in Unreal Engine; formal test results are
recorded in [validation](VALIDATION.md).

## Architecture

| Source | Responsibility |
| --- | --- |
| `OneSideScaleModule.cpp` | Viewport keyboard routing and shutdown |
| `OneSideScaleToolbar.*` | Slate icon and transform toolbar integration |
| `OneSideScaleEdMode.*` | Activation, native-mode handoff, ITF gizmo lifetime |
| `OneSideScaleGizmo.*` | Rendering, hit testing, hover, face dragging |
| `OneSideScaleTarget.*` | Bounds adapters, anchored transforms, transactions |
| `OneSideScaleSettings.h` | Per-user editor preferences |
| `Tests/OneSideScaleTests.cpp` | Five Unreal tests and a dynamic-mesh fixture |

The calculation derives a new signed axis scale from world displacement, then
translates the actor so the opposite face maps to its original world position.
ITF owns the input behaviors; one `FScopedTransaction` owns each drag. Toolbar and
shortcuts share Unreal's mode state. All original C++/header/build-rule files and
automation tests are preserved byte-for-byte.

## Limits and data handling

One actor at a time. Bounds follow actor-local mesh axes regardless of the
viewport World/Local toggle. The implementation handles rotation, mirroring,
nonuniform scale, and off-center geometry; those cases still need runtime QA.
Width is clamped to 0.01 cm so a handle cannot cross its anchor plane.

Attached actors, locked actors/levels, empty or flat meshes, zero-scale
transforms, multi-selection, instances, and skeletal meshes are unsupported.
Unsupported selections hide native handles while this mode is active; choose a
native transform tool to restore them. The four-tool cycle skips optional
combined transform modes. Blueprint subclasses can execute existing construction
scripts during editor move notifications. Stored pivot offsets are preserved;
their world position moves with the actor.

This is an editor authoring tool with no packaged-game scaling feature, network
service, telemetry, credentials, or custom importer. Changes become ordinary
level transforms when saved. Preferences use Unreal's per-user editor config.
Save a backup before evaluating an important level. Native plugin code runs in
the editor process.
