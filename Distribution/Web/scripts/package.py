#!/usr/bin/env python3
"""Package unchanged Web output with a versioned PWA host."""
import hashlib,json,shutil,sys,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parents[1];ROOT=HERE.parents[1]
sys.path.insert(0,str(ROOT/'Desktop/Tauri/scripts'));import protected
protected.check()
source=ROOT/'build/web/dist'
if not (source/'index.wasm').is_file():raise SystemExit('Build Web first with Desktop/Tauri/scripts/prepare.py')
hashfile=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
game={p.name:hashfile(p) for p in sorted(source.iterdir()) if p.is_file()}
game_version=hashlib.sha256(json.dumps(game,sort_keys=True).encode()).hexdigest()[:16]
out=HERE/'build/site';out.mkdir(parents=True,exist_ok=True)
# Only this generated output is replaced.
shutil.rmtree(out);shutil.copytree(HERE/'pwa',out)
subprocess.run([sys.executable,str(HERE/'launcher/prepare.py'),str(out/'skin')],check=True)
shutil.copytree(source,out/'game'/game_version)
for name,digest in game.items():assert hashfile(out/'game'/game_version/name)==digest
icon=ROOT/'Resources/Assets.xcassets/AppIcon.appiconset/AppIcon1024.png'
# Format/size conversion of the shipped app icon, without changing its artwork.
for size in [192,512]:
 subprocess.run(['sips','-z',str(size),str(size),str(icon),'--out',str(out/f'icon-{size}.png')],check=True,stdout=subprocess.DEVNULL)
for name in ['LICENSE','CREDITS.en.md','ASSET-LICENSES.en.md','THIRD_PARTY_NOTICES.en.md','LICENSE-SCOPE.en.md']:
 shutil.copy2(ROOT/name,out/name)
shutil.copytree(ROOT/'LICENSES',out/'LICENSES')
(out/'config.js').write_text('export const GAME_URL='+json.dumps(f'game/{game_version}/index.html')+';\n')
files=[{'path':str(p.relative_to(out)),'bytes':p.stat().st_size,'sha256':hashfile(p)} for p in sorted(out.rglob('*')) if p.is_file() and p.name!='sw.js']
version=hashlib.sha256(json.dumps(files,sort_keys=True).encode()).hexdigest()[:16]
sw=(HERE/'pwa/sw.js').read_text()
(out/'sw.js').write_text('const VERSION='+json.dumps(version)+';\nconst FILES='+json.dumps(files)+';\n'+sw)
(out/'.nojekyll').touch()
(HERE/'build/manifest.json').write_text(json.dumps({'version':version,'game_version':game_version,'game':game,'files':files},indent=2)+'\n')
protected.check();print(json.dumps({'version':version,'game_version':game_version,'bytes':sum(f['bytes'] for f in files)}))
