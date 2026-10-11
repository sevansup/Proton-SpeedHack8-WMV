#!/usr/bin/env python3
"""Add GE's matching codec libraries without replacing any Wine binaries."""
import hashlib, json, os, shutil, stat, subprocess
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / 'work'
OUT = ROOT / 'output'
NAME = 'Proton-SpeedHack8-WMV'
BASE = WORK / 'base/proton-speedhack-8.0-5'
GE = WORK / 'ge/GE-Proton8-16/files'
DEST = OUT / NAME

def inventory(folder):
    result = {}
    for p in sorted(folder.rglob('*')):
        name = str(p.relative_to(folder))
        if p.is_symlink():
            result[name] = {'link': os.readlink(p)}
        elif p.is_file():
            digest = hashlib.sha256()
            with p.open('rb') as handle:
                for chunk in iter(lambda: handle.read(1024 * 1024), b''):
                    digest.update(chunk)
            result[name] = {'sha256': digest.hexdigest(),
                            'mode': stat.S_IMODE(p.stat().st_mode)}
    return result

def main():
    OUT.mkdir(exist_ok=True)
    shutil.copytree(BASE, DEST, symlinks=True, ignore=shutil.ignore_patterns('dist'))
    dist = DEST / 'dist'
    dist.mkdir()
    subprocess.run(['tar', '--no-same-owner', '-xzf', str(BASE / 'proton_dist.tar.gz'), '-C', str(dist)], check=True)
    before = inventory(dist)
    added = []
    for lib in ('lib', 'lib64'):
        for stem, major in [('libavcodec',58), ('libavformat',58), ('libavfilter',7), ('libavutil',56)]:
            # Keep only the runtime soname link and its actual shared library.
            soname = GE / lib / f'{stem}.so.{major}'
            for source in (soname.resolve(), soname):
                target = dist / lib / source.name
                if target.exists() or target.is_symlink():
                    raise RuntimeError(f'Unexpected existing codec: {target}')
                shutil.copy2(source, target, follow_symlinks=False)
                added.append(str(target.relative_to(dist)))
        source = GE / lib / 'gstreamer-1.0/libgstasf.so'
        target = dist / lib / 'gstreamer-1.0/libgstasf.so'
        if target.exists():
            raise RuntimeError('Baseline already contains ASF plugin')
        shutil.copy2(source, target)
        added.append(str(target.relative_to(dist)))
    after = inventory(dist)
    changed = [p for p in before if before[p] != after.get(p)]
    assert not changed, changed
    assert set(after) - set(before) == set(added)
    launcher = DEST / 'proton'
    old = 'prepend_to_env_str(g_session.env, ld_path_var, steamdir + "/ubuntu12_64/video/:" + steamdir + "/ubuntu12_32/video/", ":")'
    new = old.replace('prepend_to_env_str', 'append_to_env_str', 1)
    text = launcher.read_text()
    assert text.count(old) == 1
    launcher.write_text(text.replace(old, new))
    compile(launcher.read_text(), str(launcher), 'exec')
    vdf = DEST / 'compatibilitytool.vdf'
    vdf.write_text(vdf.read_text().replace('proton-speedhack-8.0-5', NAME))
    shutil.copytree(ROOT / 'licenses', DEST / 'WMV-codec-licenses')
    shutil.copy2(ROOT / 'CODEC-SOURCES.md', DEST / 'WMV-codec-licenses/SOURCES.md')
    report = {'base': 'proton-speedhack-8.0-5', 'codec_donor': 'GE-Proton8-16',
              'unchanged_runtime_files': len(before), 'replaced_runtime_files': changed,
              'added_runtime_files': {p: after[p] for p in sorted(added)},
              'launcher_change': 'Append Steam video libraries, giving bundled libraries priority',
              'other_changes': ['compatibilitytool.vdf name', 'codec license/source notices']}
    (OUT / 'changed-files.json').write_text(json.dumps(report, indent=2)+'\n')
    print(f'Audited {len(before)} runtime files: none replaced; {len(added)} codec files/links added.')

if __name__ == '__main__':
    main()
