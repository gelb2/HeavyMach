#!/usr/bin/env python3
"""Exercise authored-image rejection in isolated copies, preserving product assets."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    evidence=ROOT/'build/kamov-evidence/image-negative'
    evidence.mkdir(parents=True,exist_ok=True)
    before={str(p.relative_to(ROOT)):sha(p) for p in (ROOT/'Resources/RuntimeImages').rglob('*') if p.is_file()}
    cases=[]
    with tempfile.TemporaryDirectory(dir=evidence) as temp:
        temp=Path(temp)
        output=temp/'RuntimeImages';shutil.copytree(ROOT/'Resources/RuntimeImages',output)
        original=json.loads((output/'manifest.json').read_text())
        args=['python3',str(ROOT/'tools/prepare_unit_images.py'),'--authoring',str(ROOT/'Art/KamovEurofighter/20261109/bake.json'),'--output',str(output),'--evidence',str(temp/'run'),'--verify']
        for kind in ['missing-png','wrong-png','wrong-manifest-hash','wrong-manifest-crop']:
            target=output/'apple/Ka52_Body.png';saved=target.read_bytes()
            manifest=json.loads(json.dumps(original))
            row=next(r for r in manifest['profiles']['apple'] if r['file']=='Ka52_Body.png')
            if kind=='missing-png': target.unlink()
            elif kind=='wrong-png': target.write_bytes(saved+b'bad')
            elif kind=='wrong-manifest-hash': row['sha256']='0'*64
            else: row['source_crop']=[0,0,0,0]
            (output/'manifest.json').write_text(json.dumps(manifest))
            result=subprocess.run(args,text=True,capture_output=True)
            assert result.returncode!=0,(kind,result.stdout)
            assert 'Authored image' in result.stderr,(kind,result.stderr)
            cases.append({'case':kind,'rejected':True,'message':result.stderr.strip()})
            target.write_bytes(saved)
        (output/'manifest.json').write_text(json.dumps(original))
        result=subprocess.run(args,text=True,capture_output=True)
        assert result.returncode==0,result.stderr
        cases.append({'case':'unchanged-package','accepted':True})
    after={str(p.relative_to(ROOT)):sha(p) for p in (ROOT/'Resources/RuntimeImages').rglob('*') if p.is_file()}
    assert before==after
    result={'cases':cases,'product_images_preserved':True}
    (evidence/'result.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result))
if __name__=='__main__':main()
