#!/usr/bin/env python3
"""Build the frozen Web project, then copy its exact output into the new host."""
import argparse, hashlib, json, os, shutil, subprocess, sys
from pathlib import Path
HERE=Path(__file__).resolve().parents[1];ROOT=HERE.parents[1]
sys.path.insert(0,str(HERE/'scripts'));import protected
p=argparse.ArgumentParser();p.add_argument('--emsdk',default=os.environ.get('EMSDK'));p.add_argument('--skip-build',action='store_true');a=p.parse_args()
protected.check()
web=ROOT/'build/web';out=HERE/'build/frontend'
if not a.skip_build:
 emcmake=Path(a.emsdk)/'upstream/emscripten/emcmake' if a.emsdk else Path(shutil.which('emcmake') or '')
 if not emcmake.is_file():raise SystemExit('Set EMSDK or use --emsdk PATH to an installed Emscripten 6.0.13 SDK')
 subprocess.run([str(emcmake),'cmake','-S',str(ROOT/'Web'),'-B',str(web),'-DCMAKE_BUILD_TYPE=Release'],check=True)
 subprocess.run(['cmake','--build',str(web),'--parallel','4'],check=True)
files=sorted((web/'dist').glob('*'))
if not (web/'dist/index.wasm').is_file():raise SystemExit('Missing Web build output')
# Only our generated host directory is replaced, never source or user saves.
if out.exists():shutil.rmtree(out)
shutil.copytree(HERE/'frontend',out)
subprocess.run([sys.executable,str(ROOT/'Distribution/Web/launcher/prepare.py'),str(out/'skin')],check=True)
shutil.copytree(web/'dist',out/'game')
hashes={f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in files if f.is_file()}
for name,h in hashes.items():assert hashlib.sha256((out/'game'/name).read_bytes()).hexdigest()==h
(HERE/'build/game-files.json').write_text(json.dumps(hashes,indent=2)+'\n')
icons=HERE/'build/icons';icons.mkdir(exist_ok=True)
subprocess.run([str(HERE/'node_modules/.bin/tauri'),'icon',str(ROOT/'Resources/Assets.xcassets/AppIcon.appiconset/AppIcon1024.png'),'--output',str(icons)],check=True)
protected.check();print('Prepared',len(hashes),'unchanged game files')
