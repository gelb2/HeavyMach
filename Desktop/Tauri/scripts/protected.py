#!/usr/bin/env python3
"""Freeze every existing product file; also reject untracked additions in its roots."""
import argparse, hashlib, json, subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
BASELINE='c39448af28a727fdb531b2d3fa57358563596bb8'
ROOTS=('Sources','Web','Android','iOS','macOS','Modules','HeavyMach.xcworkspace','Resources','Art','vendor','tools','Tests','Compatibility')
MANIFEST=ROOT/'Desktop/Tauri/protected-files.json'
def git(*args):return subprocess.check_output(['git',*args],cwd=ROOT)
def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def paths():
 all_files=git('ls-files','-z').decode().split('\0')
 return sorted(p for p in all_files if p and (p.split('/')[0] in ROOTS or '/' not in p and p not in ('AGENTS.md','.gitignore') and not p.endswith('.md')))
def capture():
 # Initial capture is only allowed with unchanged product paths against develop.
 changed=git('diff','--name-only',BASELINE,'--',*ROOTS,'CMakeLists.txt').decode()
 if changed:raise RuntimeError('Product changes before baseline capture: '+changed)
 if MANIFEST.exists():raise RuntimeError('Baseline already exists; do not replace it')
 obj={'baseline_commit':BASELINE,'files':[{'path':p,'sha256':digest(ROOT/p)} for p in paths()]}
 MANIFEST.write_text(json.dumps(obj,indent=2)+'\n');print('Captured',len(obj['files']),'protected files')
def check():
 obj=json.loads(MANIFEST.read_text());bad=[]
 files={x['path']:x['sha256'] for x in obj['files']}
 current=set(paths());expected=set(files)
 bad.extend('added/deleted: '+p for p in current^expected)
 for p,h in files.items():
  if not (ROOT/p).is_file() or digest(ROOT/p)!=h:bad.append('changed: '+p)
 extra=git('ls-files','--others','--exclude-standard','-z','--',*ROOTS).decode().split('\0')
 bad.extend('untracked: '+p for p in extra if p)
 # Existing workflows stay unchanged; new platform-only workflows are allowed.
 changes=git('diff','--name-status',obj['baseline_commit'],'--','.github/workflows').decode().splitlines()
 bad.extend('workflow: '+p for p in changes if not p.startswith('A\t') or p.split('\t')[-1] not in ('.github/workflows/desktop-tauri.yml','.github/workflows/web-pages.yml'))
 if bad:raise RuntimeError('\n'.join(bad))
 print('PASS:',len(expected),'protected files and existing workflows unchanged')
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('action',choices=['capture','check']);args=a.parse_args();globals()[args.action]()
