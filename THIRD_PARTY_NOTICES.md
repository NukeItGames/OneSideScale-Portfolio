# License and dependency boundaries

OneSideScale's original plugin source, tests, documentation, host generator, and
procedural icon are released under the repository's MIT license by NukeItGames.
The owner explicitly selected MIT for this portfolio publication. The source
history attributes the plugin to NukeItGames; no separate third-party source
library or licensed content pack was identified in the repository.

Unreal Engine is a separate prerequisite from Epic Games. It is not bundled or
relicensed by this repository. Obtain it under Epic's applicable terms:
https://www.unrealengine.com/eula/unreal

The plugin links these Unreal modules: Core, CoreUObject, Engine, InputCore,
Slate, SlateCore, UnrealEd, EditorFramework, LevelEditor,
InteractiveToolsFramework, RenderCore, GeometryFramework, GeometryCore,
DeveloperSettings, Projects, and ToolMenus. No private game module, downloadable
plugin dependency, package-manager dependency, or vendored engine implementation
is included. Security and compatibility of these modules depend on the installed
engine release; no engine dependency advisory scan was possible here.

Automation loads Epic's `/Engine/BasicShapes/Cube.Cube` and creates dynamic mesh
geometry in memory. Gizmo materials use the installed editor's ArrowMaterial.
Neither engine asset is redistributed. The former toolbar image had no recorded
provenance or redistribution terms and is excluded from the public copy. Its
replacement is generated from original geometric drawing instructions in
`scripts/generate_icon.py` and contains no metadata or external artwork.

GitHub Actions uses GitHub's checkout and setup-python actions under their own
upstream terms. Secret scanning downloads Gitleaks 8.30.1 (MIT) from its official
release and verifies the release archive's published SHA-256 checksum. These
tools are CI prerequisites, not bundled runtime dependencies.
