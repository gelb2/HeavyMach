#!/usr/bin/env python3
"""Copy verified existing resources into the generated APK assets directory."""
from pathlib import Path
import hashlib,json,shutil,sys
ROOT=Path(__file__).resolve().parents[2]
DEST=Path(sys.argv[1]).resolve()
if ROOT/'Android/app/build' not in DEST.parents:
    raise SystemExit('Output must be under Android/app/build')
manifest=json.loads((ROOT/'docs/build-resources.json').read_text())
known={v['path']:v for v in manifest['files']}
rows=[]
for src,prefix in [('Resources/OriginalImages','OriginalImages'),('Resources/RawData','RawData'),('Resources/Audio','Audio'),('Art/Extensions','Extensions')]:
    for p in sorted((ROOT/src).rglob('*')):
        if not p.is_file() or p.name.startswith('.'):continue
        data=p.read_bytes();relative=p.relative_to(ROOT).as_posix();digest=hashlib.sha256(data).hexdigest()
        if data.startswith(b'version https://git-lfs.github.com/spec/v1'):
            raise SystemExit(f'Git LFS content missing: {relative}')
        entry=known.get(relative)
        if entry and (entry['sha256']!=digest or entry['bytes']!=len(data)):
            raise SystemExit(f'Resource changed: {relative}')
        target=DEST/prefix/p.relative_to(ROOT/src)
        target.parent.mkdir(parents=True,exist_ok=True)
        if not target.exists() or hashlib.sha256(target.read_bytes()).hexdigest()!=digest:shutil.copy2(p,target)
        rows.append({'asset':target.relative_to(DEST).as_posix(),'source':relative,'bytes':len(data),'sha256':digest})
expected={r['asset'] for r in rows}|{'assets-manifest.json'}
if DEST.exists():
    for p in DEST.rglob('*'):
        if p.is_file() and p.relative_to(DEST).as_posix() not in expected:p.unlink()
(DEST/'assets-manifest.json').write_text(json.dumps({'files':rows},indent=2)+'\n')
print(f'Prepared {len(rows)} assets ({sum(r["bytes"] for r in rows):,} bytes)')
