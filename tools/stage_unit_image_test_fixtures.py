#!/usr/bin/env python3
"""Stage only the eleven adopted sources in a TEST bundle, never game assets."""
import argparse, hashlib, json, pathlib, shutil
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(path): return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--output',type=pathlib.Path,required=True);args=ap.parse_args()
    out=args.output.resolve()
    if out.name!='UnitImagePreservationFixtures':raise SystemExit('Output must be a dedicated UnitImagePreservationFixtures directory')
    # Explicit test-directory contract prevents accidental use as production art.
    if out==ROOT or out.is_relative_to(ROOT/'Resources') or out.is_relative_to(ROOT/'Art'):
        raise SystemExit('Test fixtures cannot be staged in production Resources or Art')
    manifest_path=ROOT/'Resources/RuntimeImages/manifest.json'
    manifest=json.loads(manifest_path.read_text());rows=manifest['profiles']['apple']
    if len(rows)!=63:raise SystemExit('Expected exactly 63 Apple images')
    selected={};basenames={}
    for row in rows:
        source=(ROOT/row['source']).resolve()
        if not source.is_relative_to(ROOT/'Art/Extensions'):raise SystemExit('Source outside adopted Extensions art')
        if sha(source)!=row['source_sha256']:raise SystemExit('Source hash mismatch: '+row['source'])
        output=ROOT/'Resources/RuntimeImages/apple'/row['file']
        if output.name!=row['file'] or sha(output)!=row['sha256']:raise SystemExit('Baked file/hash mismatch: '+row['file'])
        if source.name in basenames and basenames[source.name]!=source:raise SystemExit('Duplicate source basename')
        selected[row['source']]=source;basenames[source.name]=source
    if len(selected)!=11:raise SystemExit('Expected exactly 11 adopted source PNGs')
    out.mkdir(parents=True,exist_ok=True);sources=out/'sources';sources.mkdir(exist_ok=True)
    allowed={p.name for p in selected.values()}
    for old in sources.iterdir():
        if old.is_file() and old.name not in allowed:old.unlink()
    for source in selected.values():shutil.copyfile(source,sources/source.name)
    shutil.copyfile(manifest_path,out/'manifest.json')
    evidence={'schema':1,'test_only':True,'manifest_sha256':sha(manifest_path),'source_count':11,'logical_image_count':63,'sources':{s:sha(p) for s,p in sorted(selected.items())}}
    (out/'fixture-manifest.json').write_text(json.dumps(evidence,indent=2)+'\n')
    print(json.dumps({'test_fixture':str(out),'source_count':11,'logical_images':63}))
if __name__=='__main__':main()
