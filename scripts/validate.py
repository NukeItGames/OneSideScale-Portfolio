"""Engine-free publication checks. This does not compile or run Unreal Engine."""
import json
from pathlib import Path
import re
import tempfile
import xml.etree.ElementTree as ET

from generate_icon import icon_bytes
from prepare_host import prepare

ROOT = Path(__file__).resolve().parents[1]


def main():
    descriptor = json.loads((ROOT / 'OneSideScale.uplugin').read_text(encoding='utf-8'))
    assert descriptor['Modules'] == [{'Name': 'OneSideScale', 'Type': 'Editor', 'LoadingPhase': 'Default'}]
    assert descriptor['CanContainContent'] is False
    assert not descriptor.get('Plugins'), 'Unexpected external plugin dependency'
    assert (ROOT / 'Resources/icon.png').read_bytes() == icon_bytes(), 'Icon is not reproducible'
    ET.parse(ROOT / 'docs/one-sided-scaling.svg')
    assert 'MIT License' in (ROOT / 'LICENSE').read_text(encoding='utf-8')
    files = []
    ignored = {'.git', '__pycache__'}
    forbidden = {'Binaries', 'Intermediate', 'Saved', 'DerivedDataCache', 'node_modules', '.vs', '.idea', '.vscode'}
    private_path = re.compile(r'(?:[A-Za-z]:[/\\]Users[/\\]|/' + r'Users/|/' + r'home/)[^\s]+', re.I)
    for path in ROOT.rglob('*'):
        relative = path.relative_to(ROOT)
        if any(part in ignored for part in relative.parts):
            continue
        assert not path.is_symlink(), f'Symlink: {relative}'
        assert not any(part in forbidden for part in relative.parts), f'Generated/local content: {relative}'
        assert not path.name.startswith('.env'), f'Environment file: {relative}'
        if not path.is_file():
            continue
        files.append(path)
        if path.suffix == '.png':
            assert relative.as_posix() == 'Resources/icon.png', f'Unreviewed image: {relative}'
            continue
        content = path.read_text(encoding='utf-8')
        assert not private_path.search(content), f'Machine-specific path: {relative}'
        if path.suffix == '.md':
            for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)', content):
                if '://' in target or target.startswith('#'):
                    continue
                target = target.split('#', 1)[0]
                resolved = (path.parent / target).resolve()
                assert resolved.is_relative_to(ROOT) and resolved.exists(), f'Broken local link: {relative}: {target}'
    tests = (ROOT / 'Source/OneSideScale/Private/Tests/OneSideScaleTests.cpp').read_text(encoding='utf-8')
    assert tests.count('IMPLEMENT_SIMPLE_AUTOMATION_TEST(') == 5
    with tempfile.TemporaryDirectory(prefix='onesidescale-validation-') as temp:
        destination = Path(temp) / 'host'
        project = prepare(destination)
        host = json.loads(project.read_text(encoding='utf-8'))
        assert host['Plugins'] == [{'Name': 'OneSideScale', 'Enabled': True}]
        copied = destination / 'Plugins/OneSideScale'
        for source in (ROOT / 'Source').rglob('*'):
            if source.is_file():
                assert source.read_bytes() == (copied / source.relative_to(ROOT)).read_bytes()
        assert (destination / 'Source/OneSideScaleHostEditor.Target.cs').exists()
        try:
            prepare(destination)
        except FileExistsError:
            pass
        else:
            raise AssertionError('Host generator overwrote an existing directory')
    for destination in (ROOT, ROOT / '_invalid_host'):
        try:
            prepare(destination)
        except ValueError:
            pass
        else:
            raise AssertionError('Host generator accepted an in-repository destination')
    print(f'PASS: {len(files)} publication files; descriptor, links, icon, and independent host checks')
    print('Unreal compilation, five automation tests, and viewport operation: NOT RUN')


if __name__ == '__main__':
    main()
