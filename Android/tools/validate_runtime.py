#!/usr/bin/env python3
"""Run behavioral checks against the installed cheat APK through real input/JNI paths."""
from pathlib import Path
import argparse,subprocess,json,time,struct,os
P=argparse.ArgumentParser();P.add_argument('--serial',required=True);P.add_argument('--weapons',action='store_true');a=P.parse_args()
ROOT=Path(__file__).resolve().parents[2];ADB=str(Path(os.environ.get('ANDROID_SDK_ROOT',Path.home()/'Library/Android/sdk'))/'platform-tools/adb');BASE=[ADB,'-s',a.serial]
PKG='com.gelb2.heavymach.validation';OUT=ROOT/'Android/build/evidence'/time.strftime('runtime-%Y%m%d-%H%M%S');OUT.mkdir(parents=True)
checks=[]
def shell(*args):return subprocess.check_output(BASE+['shell',*map(str,args)],text=True)
def cmd(name,*values):
 args=['am','broadcast','-n',PKG+'/com.gelb2.heavymach.TestReceiver','-a','com.gelb2.heavymach.TEST','--es','command',name]
 for key,value in zip('abcd',values):args+=['--ei',key,str(value)]
 result=shell(*args)
 if 'Exception' in result:raise RuntimeError(result)
def dump():
 shell('run-as',PKG,'rm','-f','files/diagnostics.json');cmd('dump');return json.loads(subprocess.check_output(BASE+['exec-out','run-as',PKG,'cat','files/diagnostics.json']))
def check(name,ok,**evidence):
 row={'check':name,'passed':bool(ok),**evidence};checks.append(row);print(json.dumps(row),flush=True);(OUT/'results.json').write_text(json.dumps(checks,indent=2))
def capture(name):
 (OUT/(name+'.png')).write_bytes(subprocess.check_output(BASE+['exec-out','screencap','-p']))
def swipe(nx,ny,ms=1200):
 raw=subprocess.check_output(BASE+['exec-out','screencap','-p']);w,h=struct.unpack('>II',raw[16:24]);shell('input','swipe',round(w*nx),round(h*ny),round(w*nx),round(h*ny),ms)
shell('am','start','-W','-n',PKG+'/com.gelb2.heavymach.GameActivity');time.sleep(1)
cmd('cancel');cmd('fixtureSettings',0,0,0,0);cmd('cheats',1);cmd('stage',0,0,0,0);cmd('aim',0);time.sleep(1)
b=dump();check('game started',b.get('mission')==0 and b.get('stage')==0 and b.get('active') and not b.get('paused') and not b.get('error'),state=b)
cmd('key',7,1);time.sleep(1.2);cmd('key',7,0);m=dump();check('keyboard forward',m.get('x',0)>b.get('x',0)+5,before=b,after=m)
cmd('key',4,1);time.sleep(1.2);cmd('key',4,0);l=dump();check('keyboard backward',l.get('x',0)<m.get('x',0)-5,before=m,after=l)
cmd('jump');time.sleep(.25);j=dump();check('jump rises',j.get('y',0)>l.get('y',0)+5,before=l,after=j)
time.sleep(1);before=dump();swipe(.85,.4);after=dump();check('touch fire at expanded right side',after.get('shots',0)>before.get('shots',0),before=before,after=after)
check('firing does not save',after.get('saveWrites')==before.get('saveWrites'),before=before.get('saveWrites'),after=after.get('saveWrites'))
capture('manual-combat')
# A lifecycle transition must freeze the world, preserve position, and require Resume.
before=dump();shell('input','keyevent','KEYCODE_HOME');time.sleep(.8)
shell('am','start','-W','-n',PKG+'/com.gelb2.heavymach.GameActivity');time.sleep(.8);paused=dump();time.sleep(.5);later=dump()
check('background/foreground pauses world',paused.get('paused') and later.get('worldTime')==paused.get('worldTime'),before=before,after=paused,later=later)
cmd('resume');time.sleep(.5);r=dump();check('explicit resume advances world',not r.get('paused') and r.get('worldTime',0)>paused.get('worldTime',0),state=r)
cmd('focus',0);time.sleep(.25);f=dump();cmd('focus',1);time.sleep(.25);g=dump();check('audio focus loss pauses and recovery retains pause',f.get('paused') and not f.get('audioActive') and g.get('paused') and g.get('audioActive'),loss=f,gain=g)
cmd('resume');before=dump();swipe(.85,.4);after=dump();check('audio requests resume after focus recovery',after.get('audioStarts',0)>before.get('audioStarts',0),before=before,after=after)
cmd('aim',1);time.sleep(.5);cmd('fire',1);time.sleep(1.5);cmd('fire',0);auto=dump();check('automatic aim mode remains enabled',auto.get('aim')==2,state=auto);capture('automatic-combat');cmd('aim',0)
if a.weapons:
 data=subprocess.check_output(BASE+['exec-out','run-as',PKG,'cat','files/HvM.dat']);count=struct.unpack_from('<i',data,0x30)[0]
 names=[data[0x34+i*0x1c:0x4c+i*0x1c].split(b'\0',1)[0].decode() for i in range(count)]
 for i,name in enumerate(names):
  cmd('weapon',i);time.sleep(.25);before=dump();swipe(.86,.35,2200);after=dump();check('weapon '+name,after.get('shots',0)>before.get('shots',0) and not after.get('error'),index=i,before=before,after=after)
  if i in (0,3,6,count-1):capture('weapon-'+str(i))
cmd('pause');capture('pause-menu');(OUT/'logcat.txt').write_text(shell('logcat','-d','-s','HeavyMach','AndroidRuntime'));print('EVIDENCE '+str(OUT),flush=True)
raise SystemExit(0 if all(x['passed'] for x in checks) else 1)
