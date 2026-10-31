#!/usr/bin/env python3
"""Publish generated static files to the dedicated gh-pages branch."""
import json,shutil,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parents[1];ROOT=HERE.parents[1]
run=lambda *a,**kw:subprocess.run(a,check=True,**kw)
run('python3',str(ROOT/'Desktop/Tauri/scripts/protected.py'),'check')
site=HERE/'build/site';stage=HERE/'build/publish'
if not (site/'sw.js').is_file():raise SystemExit('Run package.py first')
remote=subprocess.check_output(['git','remote','get-url','origin'],cwd=ROOT,text=True).strip()
if not stage.exists():
 stage.mkdir();run('git','init','--initial-branch=gh-pages',str(stage));run('git','remote','add','origin',remote,cwd=stage)
exists=subprocess.check_output(['git','ls-remote','--heads','origin','gh-pages'],cwd=stage,text=True).strip()
if exists:
 run('git','fetch','origin','gh-pages',cwd=stage);run('git','checkout','-B','gh-pages','FETCH_HEAD',cwd=stage)
for p in stage.iterdir():
 if p.name=='.git':continue
 if p.is_dir():shutil.rmtree(p)
 else:p.unlink()
shutil.copytree(site,stage,dirs_exist_ok=True)
run('git','add','--all',cwd=stage)
if subprocess.run(['git','diff','--cached','--quiet'],cwd=stage).returncode==0:print('Already published');raise SystemExit()
version=json.loads((HERE/'build/manifest.json').read_text())['version']
run('git','commit','-m','Deploy HeavyMach web '+version,cwd=stage)
run('git','push','origin','HEAD:gh-pages',cwd=stage)
