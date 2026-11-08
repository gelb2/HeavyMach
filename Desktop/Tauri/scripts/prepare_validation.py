#!/usr/bin/env python3
"""Package the explicit Web validation build; never copy the production launcher."""
import argparse, hashlib, json, os, shutil, subprocess, sys
from pathlib import Path
HERE=Path(__file__).resolve().parents[1];ROOT=HERE.parents[1]
sys.path.insert(0,str(HERE/'scripts'));import protected
from validate_web_artifact import validate_web_artifact
p=argparse.ArgumentParser();p.add_argument('--emsdk',default=os.environ.get('EMSDK'));p.add_argument('--skip-build',action='store_true');a=p.parse_args()
protected.check()
web=ROOT/'build/web-unit-validation';out=HERE/'build/validation-frontend'
if not a.skip_build:
 emcmake=Path(a.emsdk)/'upstream/emscripten/emcmake' if a.emsdk else Path(shutil.which('emcmake') or '')
 if not emcmake.is_file():raise SystemExit('Set EMSDK or --emsdk')
 subprocess.run([str(emcmake),'cmake','-S',str(ROOT/'Web'),'-B',str(web),'-DCMAKE_BUILD_TYPE=Release','-DHM_WEB_VALIDATION=ON'],check=True)
 subprocess.run(['cmake','--build',str(web),'--parallel','4'],check=True)
cache=(web/'CMakeCache.txt').read_text()
if 'HM_WEB_VALIDATION:BOOL=ON' not in cache:raise SystemExit('Refuse normal Web build in validation host')
if not (web/'dist/index.wasm').is_file():raise SystemExit('Missing Web validation build')
if '_hm_validation_start' not in (web/'dist/index.js').read_text() or '/validation-persist' not in (web/'dist/index.html').read_text():raise SystemExit('Validation cache points to a stale or normal artifact')
validate_web_artifact(web/'dist', validation=True)
if out.exists():shutil.rmtree(out)
shutil.copytree(web/'dist',out)
hashes={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted((web/'dist').iterdir()) if f.is_file()}
for name,h in hashes.items():assert hashlib.sha256((out/name).read_bytes()).hexdigest()==h
(HERE/'build/validation-game-files.json').write_text(json.dumps(hashes,indent=2)+'\n')
icons=HERE/'build/icons';icons.mkdir(exist_ok=True)
subprocess.run([str(HERE/'node_modules/.bin/tauri'),'icon',str(ROOT/'Resources/Assets.xcassets/AppIcon.appiconset/AppIcon1024.png'),'--output',str(icons)],check=True)
protected.check();print('Prepared',len(hashes),'unchanged validation game files; no production launcher/storage scripts')
