"""Create an independent C++ Unreal test host; never overwrite an existing path."""
import argparse
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def prepare(destination):
    destination = destination.resolve()
    if destination == ROOT or ROOT in destination.parents:
        raise ValueError('Choose a destination outside the plugin repository.')
    destination.mkdir(parents=True, exist_ok=False)
    plugin = destination / 'Plugins/OneSideScale'
    plugin.mkdir(parents=True)
    for name in ('Source', 'Resources'):
        shutil.copytree(ROOT / name, plugin / name)
    for name in ('OneSideScale.uplugin', 'LICENSE', 'THIRD_PARTY_NOTICES.md'):
        shutil.copy2(ROOT / name, plugin / name)
    project = {'FileVersion': 3, 'Category': 'Samples',
               'Description': 'Independent OneSideScale validation host',
               'Modules': [{'Name': 'OneSideScaleHost', 'Type': 'Runtime', 'LoadingPhase': 'Default'}],
               'Plugins': [{'Name': 'OneSideScale', 'Enabled': True}]}
    (destination / 'OneSideScaleHost.uproject').write_text(json.dumps(project, indent=2) + '\n', encoding='utf-8')
    source = destination / 'Source'
    module = source / 'OneSideScaleHost'
    module.mkdir(parents=True)
    (module / 'OneSideScaleHost.Build.cs').write_text('''using UnrealBuildTool;
public class OneSideScaleHost : ModuleRules
{
    public OneSideScaleHost(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
    }
}
''', encoding='utf-8')
    (module / 'OneSideScaleHost.cpp').write_text('''#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, OneSideScaleHost, "OneSideScaleHost");
''', encoding='utf-8')
    for suffix, target_type in (('', 'Game'), ('Editor', 'Editor')):
        name = 'OneSideScaleHost' + suffix
        (source / (name + '.Target.cs')).write_text(f'''using UnrealBuildTool;
public class {name}Target : TargetRules
{{
    public {name}Target(TargetInfo Target) : base(Target)
    {{
        Type = TargetType.{target_type};
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("OneSideScaleHost");
    }}
}}
''', encoding='utf-8')
    return destination / 'OneSideScaleHost.uproject'


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('destination', type=Path, help='New directory outside this repository')
    args = parser.parse_args()
    print(prepare(args.destination))
