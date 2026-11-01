#!/usr/bin/env python3
"""Use normal-edition game buttons to export/restore its own save; retain file hashes."""
from pathlib import Path
import subprocess,os,struct,time,json,argparse,hashlib,xml.etree.ElementTree as ET,re
p=argparse.ArgumentParser();p.add_argument('--serial',required=True);p.add_argument('--out',type=Path,required=True);a=p.parse_args();out=a.out;out.mkdir(parents=True,exist_ok=True)
adb=str(Path(os.environ.get('ANDROID_SDK_ROOT',Path.home()/'Library/Android/sdk'))/'platform-tools/adb');base=[adb,'-s',a.serial];pkg='com.gelb2.heavymach.restored';rows=[]
def shell(*args):return subprocess.check_output(base+['shell',*map(str,args)],text=True)
def read(name):return subprocess.check_output(base+['exec-out','run-as',pkg,'cat','files/'+name])
def check(name,passed,**details):rows.append(dict(check=name,passed=bool(passed),**details));(out/'results.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows[-1]),flush=True)
def capture(name):
 raw=subprocess.check_output(base+['exec-out','screencap','-p']);(out/(name+'.png')).write_bytes(raw);return struct.unpack('>II',raw[16:24])
def node():
 shell('uiautomator','dump','/sdcard/heavymach-transfer-test.xml');text=shell('cat','/sdcard/heavymach-transfer-test.xml');return ET.fromstring(text).find(".//node[@class='android.widget.EditText']")
def button(id):
 # The code field is the only native rectangle. Its centre identifies the same
 # safe-area centre used by the game; controls below it use game logical units.
 x=centre[0]+(-232+12+(464-40)/6 if id==0 else -232+28+2*(464-40)/3+(464-40)/6 if id==2 else -232+12+(464-32)/4 if id==3 else 232-12-(464-32)/4)
 y=centre[1]+(40 if id in [0,2] else -124)
 shell('input','tap',int(x*scale),int((320-y)*scale));time.sleep(.4)
shell('am','force-stop',pkg)
original={n:read(n) for n in ['HvM.dat','HUDLayoutV1.txt','InputV1.txt']}
backup=subprocess.check_output(base+['exec-out','run-as',pkg,'tar','-cf','-','files']);(out/'before-files.tar').write_bytes(backup)
try:
 shell('am','start','-W','-n',pkg+'/com.gelb2.heavymach.GameActivity');time.sleep(8);screen=capture('title');shell('input','keyevent','66');time.sleep(3);shell('input','keyevent','133');time.sleep(.4)
 capture('after-f3');n=node();check('Normal F3 opens game transfer with text entry',n is not None)
 if n is None:raise RuntimeError('No recovery text field')
 x1,y1,x2,y2=map(int,re.findall(r'\d+',n.attrib['bounds']));scale=screen[1]/320;centre=((x1+x2)/2/scale,160)
 capture('normal-transfer');button(0);n=node();code=n.attrib.get('text','') if n is not None else '';check('Normal CREATE CODE produces HM1 code',code.startswith('HM1-'),bytes=len(code));
 if not code.startswith('HM1-'):raise RuntimeError('No exported recovery code')
 # The exact code stays in the app field; no clipboard injection or codec test shortcut.
 button(2);capture('normal-code-checked');button(3);capture('normal-restore-confirm')
 check('Confirmation hides native text entry',node() is None)
 # RESTORE on confirmation has y=panelBottom+18, normal panel height304.
 x=centre[0]-232+16+(464/2-24)/2;y=160-152+18+14
 shell('input','tap',int(x*scale),int((320-y)*scale));time.sleep(1.8);capture('normal-restored-slots')
 check('Actual normal restore keeps all saved values',{n:read(n) for n in original}==original)
 shell('input','keyevent','133');time.sleep(.4);check('Normal transfer reopens after restore',node() is not None);button(4);check('Normal CLOSE removes text field',node() is None)
finally:
 shell('am','force-stop',pkg)
 # Restore original files, including the pre-test backup history.
 shell('run-as',pkg,'rm','-rf','files')
 # shell -T waits for remote tar to exit; exec-in can return before extraction.
 r=subprocess.run(base+['shell','-T','run-as',pkg,'tar','-xf','-'],input=backup,capture_output=True)
 restored={n:read(n) for n in original}
 check('Original normal data preserved',r.returncode==0 and restored==original,restoreExit=r.returncode,restoreError=r.stderr.decode(errors='replace'),files={n:{'before':hashlib.sha256(v).hexdigest(),'after':hashlib.sha256(restored[n]).hexdigest()} for n,v in original.items()})
 shell('input','keyevent','3');shell('rm','-f','/sdcard/heavymach-transfer-test.xml')
print('EVIDENCE '+str(out),flush=True)
raise SystemExit(0 if all(x['passed'] for x in rows) else 1)
