# Installation and builds

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
generator. No API key, backend, third-party plugin, or private game dependency is required.

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
