# OneSideScale

Resize a mesh from one face while keeping the opposite face in place.

OneSideScale is an **editor-only C++ plugin for Unreal Engine**. It adds a fourth
transform tool to the Level Editor, with six face handles, translation-grid
snapping, and one undo transaction per drag. It changes actor location and scale;
mesh geometry and the stored pivot offset stay intact.

![Illustration of a fixed left face and an outward-moving right face](docs/one-sided-scaling.svg)

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
Escape can still cancel it. These describe the preserved implementation, not
completed runtime QA.

## Requirements and compatibility

The original source identifies **UE 5.8.2** as its development target. This is a
source-declared target, not verified compatibility. The code uses version-sensitive
Level Editor toolbar and legacy-widget/ITF APIs. Earlier UE5 versions, later
releases, macOS, and Linux have not been compiled or tested here. The descriptor
has no EngineVersion lock; that does not imply broad compatibility. This is a
source distribution with no prebuilt binary.

For Windows, install the intended Unreal Engine version through Epic and its
recommended Visual Studio C++ workload and Windows SDK. Obtain Unreal separately
under [Epic's terms](https://www.unrealengine.com/eula/unreal). Python 3.10+ is
needed only for repository checks, icon generation, and the optional host
generator. No Quantum Jumpers project, API key, backend, third-party plugin, or
private game dependency is required.

## Install in an existing project

Close Unreal Editor. From PowerShell, set your project location and clone into
its project-level Plugins directory:

```powershell
$ProjectRoot = Read-Host 'Absolute path to your Unreal project directory'
New-Item -ItemType Directory -Force (Join-Path $ProjectRoot 'Plugins') | Out-Null
git clone https://github.com/NukeItGames/OneSideScale-Portfolio.git (Join-Path $ProjectRoot 'Plugins/OneSideScale')
```

The descriptor must be at `YourProject/Plugins/OneSideScale/OneSideScale.uplugin`.
Regenerate project files and build your project's **Editor** target with Unreal
closed. The plugin enables itself by default; check Edit → Plugins → OneSideScale
if needed. For Blueprint-only projects, use the C++ host below or compile with
BuildPlugin before installation. A C++ toolchain is still required.

## Build without a private project

Clone outside an existing Unreal project and generate a small C++ host:

```powershell
git clone https://github.com/NukeItGames/OneSideScale-Portfolio.git
cd OneSideScale-Portfolio
python scripts/validate.py
python scripts/prepare_host.py ../OneSideScaleHost

$EngineRoot = Read-Host 'Absolute path to the Unreal Engine installation'
$HostProject = (Resolve-Path ../OneSideScaleHost/OneSideScaleHost.uproject).Path
& "$EngineRoot/Engine/Build/BatchFiles/Build.bat" OneSideScaleHostEditor Win64 Development "-Project=$HostProject" -WaitMutex -NoHotReloadFromIDE
if ($LASTEXITCODE -ne 0) { throw 'Editor build failed' }
& "$EngineRoot/Engine/Binaries/Win64/UnrealEditor.exe" $HostProject
```

The generator creates a new directory and copies only Source, Resources, the
descriptor, and license notices into it. It refuses to overwrite an existing
directory or put the host inside this checkout. No engine association or local
machine path is committed. Recreate the host at a new destination after changing
plugin source; it is a snapshot. The generated game module is empty; the scaling
code stays in the editor-only plugin module.

For a compiled plugin package, choose a new output directory outside the checkout
and use Epic's automation tool with Unreal closed:

```powershell
$PluginDescriptor = (Resolve-Path ./OneSideScale.uplugin).Path
$PackageOutput = Read-Host 'New absolute output directory outside this checkout'
& "$EngineRoot/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin "-Plugin=$PluginDescriptor" "-Package=$PackageOutput" -TargetPlatforms=Win64
if ($LASTEXITCODE -ne 0) { throw 'Plugin packaging failed' }
```

These Unreal commands have not been executed in this environment. Missing toolbar
symbols or override mismatches require checking the exact engine version and
toolchain. Passing static CI does not establish engine compatibility.

## Tests and demonstration

```sh
python scripts/validate.py
```

This engine-free check verifies publication structure, JSON, source boundaries,
local documentation links, icon reproducibility, and independent host generation.
GitHub Actions runs it on Linux and Windows and scans Git history with Gitleaks.
It does **not** compile Unreal or simulate editor input.

Five preserved Unreal tests cover anchored faces, grid snapping, Static Mesh
undo/redo, Dynamic Mesh/pivot preservation, and Escape cancellation through
ITF/Slate. Follow [the automation and manual instructions](docs/EDITOR_TESTING.md)
in the generated host. The interaction test requires a real Level Editor viewport
in Selection mode; commandlet execution does not cover that workflow.

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

## License

[MIT](LICENSE), copyright NukeItGames. Unreal Engine and its built-in assets keep
Epic's separate terms. See [third-party notices](THIRD_PARTY_NOTICES.md) and the
[publication audit](docs/AUDIT.md) for source and asset review.
