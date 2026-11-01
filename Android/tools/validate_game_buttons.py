#!/usr/bin/env python3
"""Exercise rendered original-style game controls, retaining the user's cheat save."""
from pathlib import Path
import argparse, subprocess, time, json, os, struct, hashlib, xml.etree.ElementTree as ET, re
p=argparse.ArgumentParser();p.add_argument('--serial',required=True);p.add_argument('--out',type=Path);a=p.parse_args()
adb=Path(os.environ.get('ANDROID_SDK_ROOT',Path.home()/'Library/Android/sdk'))/'platform-tools/adb';base=[str(adb),'-s',a.serial];pkg='com.gelb2.heavymach.validation'
out=a.out or Path(__file__).resolve().parents[1]/'build/evidence'/time.strftime('game-buttons-%Y%m%d-%H%M%S');out.mkdir(parents=True,exist_ok=True);rows=[]
def shell(*args):return subprocess.check_output(base+['shell',*map(str,args)],text=True)
def cmd(name,*args):
 v=['am','broadcast','-n',pkg+'/com.gelb2.heavymach.TestReceiver','-a','com.gelb2.heavymach.TEST','--es','command',name]
 for k,n in zip('abcd',args):v+=['--ei',k,str(n)]
 shell(*v)
def state():
 time.sleep(.1);shell('run-as',pkg,'rm','-f','files/diagnostics.json');cmd('dump');return json.loads(shell('run-as',pkg,'cat','files/diagnostics.json'))
def check(name,ok,**details):
 rows.append(dict(check=name,passed=bool(ok),**details));print(json.dumps(rows[-1]),flush=True);(out/'results.json').write_text(json.dumps(rows,indent=2))
def capture(name):
 image=subprocess.check_output(base+['exec-out','screencap','-p']);(out/(name+'.png')).write_bytes(image);(out/(name+'.json')).write_text(json.dumps(state(),indent=2));return struct.unpack('>II',image[16:24])
def tap(id,original=False,hold=0):
 s=state();v=next((b for b in s['originalButtons' if original else 'buttons'] if b['id']==id),None)
 for _ in range(30):
  if v:break
  time.sleep(.15);s=state();v=next((b for b in s['originalButtons' if original else 'buttons'] if b['id']==id),None)
 if not v:raise RuntimeError(f'Button {id} absent: {s}')
 x,y,w,h=v['rect'];cw,ch=s['canvas'];px=int((x+w/2)*screen[0]/cw);py=int((ch-y-h/2)*screen[1]/ch)
 if hold:shell('input','swipe',px,py,px,py,hold)
 else:shell('input','tap',px,py)
 time.sleep(.18)
def file(name):
 r=subprocess.run(base+['exec-out','run-as',pkg,'cat','files/'+name],capture_output=True)
 if r.returncode==0:return r.stdout
 if b'No such file' in r.stderr+r.stdout:return b''
 raise RuntimeError('Cannot read '+name+': '+(r.stderr+r.stdout).decode(errors='replace'))
def savehash():return hashlib.sha256(file('HvM.dat')).hexdigest()
# Only the validation app is changed. Preserve its complete private files first.
shell('am','force-stop',pkg)
backup=subprocess.run(base+['exec-out','run-as',pkg,'tar','-cf','-','files'],capture_output=True)
if backup.returncode:raise SystemExit('Could not back up validation files')
(out/'before-files.tar').write_bytes(backup.stdout)
original_files={n:file(n) for n in ['HvM.dat','HUDLayoutV1.txt','InputV1.txt']}
(out/'original-file-hashes.json').write_text(json.dumps({n:hashlib.sha256(v).hexdigest() for n,v in original_files.items()},indent=2))
try:
 shell('am','start','-W','-n',pkg+'/com.gelb2.heavymach.GameActivity');time.sleep(.8);screen=capture('start')
 cmd('fixtureSettings',0,0,0,0);cmd('cheats',1);cmd('stage',0,0,0,0);cmd('pause');time.sleep(.3)
 tap(104,True);s=state();check('Original pause SET GAME UI opens editor',s['editor']);capture('hud-tools')
 before=s;old=file('HUDLayoutV1.txt');shell('input','keyevent','136');shell('input','keyevent','61');shell('input','keyevent','66');check('F6 Tab Enter operates HUD tools',not any(b['id']==21002 for b in state()['buttons']));shell('input','keyevent','111');tap(21005);tap(21003);s=state();check('Preview hides tool rows',s['editor'] and not any(b['id']==21002 for b in s['buttons']))
 tap(21005);tap(21004);s=state();check('Sample switches without advancing combat',s['editor'] and s['worldTime']==before['worldTime']);tap(21004)
 tap(21000);check('Reset draft does not write layout',file('HUDLayoutV1.txt')==old)
 tap(21001);check('Cancel closes editor without saving',not state()['editor'] and file('HUDLayoutV1.txt')==old)
 tap(104,True);tap(21000);tap(21002);check('Apply closes editor',not state()['editor'])
 tap(101,True);time.sleep(.25);s=state();x=s['x'];tap(25001,hold=650);check('Cheat RIGHT moves through game button',state()['x']>x+5)
 x=state()['x'];tap(25000,hold=650);check('Cheat LEFT moves through game button',state()['x']<x-5)
 s=state();tap(25002);check('Cheat JUMP operates',state()['y']>s['y']+1)
 s=state();tap(25003,hold=600);check('Cheat FIRE operates',state()['shots']>s['shots'])
 tap(24000);s=state();check('TEST opens game panel',s['panel']==3 and s['paused']);capture('cheat-panel')
 tap(24100);check('Stage fields render in game',state()['panel']==4);capture('stage-fields')
 s=state()
 for i in range(9):tap(24200+i*2+1);tap(24200+i*2)
 check('Nine stage increment/decrement controls retain values',state()['stageFields']==s['stageFields'])
 tap(24301);check('Stage cancel returns to TEST',state()['panel']==3)
 tap(24100);tap(24300);s=state();check('Stage START runs selected mission',s['panel']==0 and s['mission']==0 and s['stage']==0)
 for action,name in [(24102,'Cheats off'),(24101,'Cheats on'),(24106,'Save diagnostics'),(24107,'Resume saved game')]:
  tap(24000);tap(action);check(name+' game button closes panel',state()['panel']==0)
 tap(24000);before=state();tap(24103);check('Add experience button adds one',state()['experience']==before['experience']+1)
 tap(24000);tap(24104);check('Resume button restarts combat',not state()['paused'])
 tap(24000);tap(24108);check('TEST close keeps game paused',state()['panel']==0 and state()['paused'])
 tap(24000);tap(24105);s=state();check('Key settings game panel',len([b for b in s['buttons'] if 23000<=b['id']<23016])==16);capture('key-settings')
 for i in range(16):tap(23000+i);s=state();check('Key selection '+str(i),s['captureKey'] and s['keySelection']==i);shell('input','keyevent','111')
 tap(23006);shell('input','keyevent','54');check('Captured Z remaps fire',state()['bindings'][6]==29);tap(23100);s=state();check('Reset keys restores default move/fire',s['bindings'][0]==4 and s['bindings'][6]==9);tap(23101)
 cmd('pause');time.sleep(.25);tap(102,True);time.sleep(.8);s=state();check('Save and exit reaches slots',s['screen']==3 and s['showMenu'])
 before=savehash();tap(22000);s=state();check('Transfer game entry opens panel',s['panel']==1);capture('transfer')
 tap(22100);s=state();check('Create code produces data',s['recoveryCodeBytes']>4 and s['textRevision']>0)
 tap(22101);tap(22102);s=state();check('Copy and check retain save',savehash()==before and bool(s['uiStatus']))
 # Enter invalid text through the native text field, then exercise rendered CHECK/RESTORE.
 shell('uiautomator','dump','/sdcard/heavymach-buttons-test.xml')
 tree=ET.fromstring(shell('cat','/sdcard/heavymach-buttons-test.xml'));entry=tree.find(".//node[@class='android.widget.EditText']")
 if entry is None:raise RuntimeError('Recovery text entry absent')
 x1,y1,x2,y2=map(int,re.findall(r'\d+',entry.attrib['bounds']));shell('input','tap',(x1+x2)//2,(y1+y2)//2)
 shell('input','keycombination','113','29');shell('input','text','NOT-A-RECOVERY-CODE');shell('input','keyevent','4');time.sleep(.2)
 tap(22102);check('Invalid typed code reports error without changing save',savehash()==before and state()['panel']==1 and state()['uiStatus']=='Unsupported recovery code')
 tap(22103);check('Invalid code cannot open restore confirmation',state()['panel']==1 and savehash()==before)
 tap(22100);tap(22102)
 for _ in range(4):tap(22105);tap(22106)
 tap(22107);tap(22107);check('Slot/settings choices do not write save',savehash()==before)
 tap(22103);check('Restore opens game confirmation',state()['panel']==2);capture('transfer-confirm')
 tap(22109);check('Restore cancel preserves progress',state()['panel']==1 and savehash()==before)
 tap(22103);tap(22108);time.sleep(.8);check('Restore confirms and reloads slots',state()['screen']==3 and state()['panel']==0 and savehash()==before)
 tap(22000);tap(22104);check('Transfer close returns to slots',state()['panel']==0)
 shell('input','keyevent','133');check('F3 opens transfer',state()['panel']==1);shell('input','keyevent','61');shell('input','keyevent','66');check('Transfer Tab Enter creates code',state()['recoveryCodeBytes']>4);tap(22104)
 cmd('recreateContext');time.sleep(.4);tap(22000);check('Buttons survive graphics context rebuild',state()['panel']==1 and not state()['error']);tap(22104)
 shell('input','keyevent','3');time.sleep(.3);shell('am','start','-W','-n',pkg+'/com.gelb2.heavymach.GameActivity');time.sleep(.4);tap(22000);check('Buttons survive background/foreground',state()['panel']==1);tap(22104)
 capture('finished');(out/'logcat.txt').write_text(shell('logcat','-d','-s','HeavyMach','AndroidRuntime'))
finally:
 shell('am','force-stop',pkg)
 # Restore exactly the private directory backed up above, after test process exit.
 shell('run-as',pkg,'rm','-rf','files')
 # shell -T waits for remote tar to exit; exec-in can return before extraction.
 r=subprocess.run(base+['shell','-T','run-as',pkg,'tar','-xf','-'],input=backup.stdout,capture_output=True)
 restored_files={n:file(n) for n in original_files}
 check('Original validation files restored',r.returncode==0 and restored_files==original_files,restoreExit=r.returncode,restoreError=r.stderr.decode(errors='replace'),files={n:{'before':hashlib.sha256(v).hexdigest(),'after':hashlib.sha256(restored_files[n]).hexdigest()} for n,v in original_files.items()})
 shell('rm','-f','/sdcard/heavymach-buttons-test.xml')
 shell('input','keyevent','3')
print('EVIDENCE '+str(out),flush=True)
raise SystemExit(0 if all(row['passed'] for row in rows) else 1)
