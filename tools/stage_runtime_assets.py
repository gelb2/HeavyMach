#!/usr/bin/env python3
"""Stage verified original resources and authored runtime resources for an app."""
import argparse, hashlib, json, shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def stage(profile, destination):
    destination=destination.resolve()
    if destination==ROOT or any(destination.is_relative_to(ROOT/p) for p in ('Resources','Art','Sources')):
        raise ValueError('Output must be a generated build or app bundle directory')
    known={x['path']:x for x in json.loads((ROOT/'docs/build-resources.json').read_text())['files']}
    runtime=json.loads((ROOT/'Resources/RuntimeData/manifest.json').read_text())['runtime_sha256']
    runtime_files={p.name for p in (ROOT/'Resources/RuntimeData').iterdir() if p.suffix in ('.mch','.map','.hmx')}
    if runtime_files!=set(runtime):raise ValueError('Runtime data package has missing or unexpected files')
    images=json.loads((ROOT/'Resources/RuntimeImages/manifest.json').read_text())['profiles'][profile]
    image_hashes={x['file']:x['sha256'] for x in images}
    if len(image_hashes)!=len(images):raise ValueError('Runtime image manifest has duplicate files')
    image_root=ROOT/'Resources/RuntimeImages'/profile
    image_files={p.relative_to(image_root).as_posix() for p in image_root.rglob('*') if p.is_file() and not p.name.startswith('.')}
    if image_files!=set(image_hashes):raise ValueError('Runtime image package has missing or unexpected files')
    roots=[('Resources/OriginalImages','OriginalImages'),('Resources/Audio','Audio'),
           ('Resources/RuntimeData','RuntimeData'),('Art/Extensions','Extensions'),
           ('Resources/RuntimeImages/'+profile,'OriginalImages')]
    for source in ('Resources/OriginalImages','Resources/Audio','Art/Extensions'):
        missing=[name for name in known if name.startswith(source+'/') and not (ROOT/name).is_file()]
        if missing:raise ValueError('Missing original/approved resources: '+', '.join(missing))
    rows=[];seen=set()
    for source,prefix in roots:
        base=ROOT/source
        if not base.is_dir(): raise ValueError('Missing runtime resource directory: '+source)
        for p in sorted(base.rglob('*')):
            if not p.is_file() or p.name.startswith('.'):continue
            relative=p.relative_to(base).as_posix()
            # Authoring PNGs remain in Git; only baked unit images ship.
            if prefix=='Extensions' and (p.stem.startswith(('VTOL_','Lux_','GAA_'))):continue
            if prefix=='RuntimeData' and p.suffix not in ('.mch','.map','.hmx'):continue
            key=prefix+'/'+relative
            if key in seen:raise ValueError('Duplicate runtime image/resource: '+key)
            seen.add(key);h=digest(p);data=p.read_bytes()
            if data.startswith(b'version https://git-lfs.github.com/spec/v1'):raise ValueError('Missing LFS resource: '+str(p))
            entry=known.get(p.relative_to(ROOT).as_posix())
            if entry and (entry['sha256']!=h or entry['bytes']!=len(data)):raise ValueError('Original/approved resource changed: '+str(p))
            if prefix=='RuntimeData' and runtime.get(relative)!=h:raise ValueError('Runtime data manifest mismatch: '+relative)
            if source.startswith('Resources/RuntimeImages/') and image_hashes.get(relative)!=h:raise ValueError('Runtime image manifest mismatch: '+relative)
            target=destination/key;target.parent.mkdir(parents=True,exist_ok=True)
            if not target.exists() or digest(target)!=h:shutil.copy2(p,target)
            rows.append({'asset':key,'source':p.relative_to(ROOT).as_posix(),'bytes':len(data),'sha256':h})
    for prefix in ('OriginalImages','Audio','RuntimeData','Extensions','RawData'):
        for p in (destination/prefix).rglob('*'):
            if p.is_file() and p.relative_to(destination).as_posix() not in seen:p.unlink()
    (destination/'assets-manifest.json').write_text(json.dumps({'profile':profile,'files':rows},indent=2)+'\n')
    print(f'Staged {len(rows)} verified {profile} resources')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--profile',choices=('apple','android','web'),required=True);p.add_argument('--destination',type=Path,required=True);a=p.parse_args();stage(a.profile,a.destination)
