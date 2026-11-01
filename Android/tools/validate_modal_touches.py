#!/usr/bin/env python3
"""Retain touch ownership when a second pointer closes a game panel."""
from pathlib import Path
source=Path(__file__).with_name('validate_game_buttons.py').read_text().split("try:\n shell('am','start'")[0]
exec(source)
def pointer(token,phase,x,y):
 s=state();cw,ch=s['canvas'];cmd('touch',token,phase,round(x/cw*1000000),round((ch-y)/ch*1000000))
def centre(id):
 r=next(b['rect'] for b in state()['originalButtons'] if b['id']==id);return r[0]+r[2]/2,r[1]+r[3]/2
try:
 shell('am','start','-W','-n',pkg+'/com.gelb2.heavymach.GameActivity');time.sleep(1);screen=capture('start');cmd('fixtureSettings',0,0,0,0);cmd('cheats',1);cmd('stage',0,0,0,0);cmd('pause');tap(102,True);time.sleep(1)
 tap(22000);s=state();cw,ch=s['canvas'];pointer(70,0,cw/2-228,ch/2);tap(22104);x,y=centre(1000);pointer(70,1,x,y);pointer(70,2,x,y);time.sleep(.8);s=state();check('Modal background release after CLOSE cannot activate original slot OK',s['screen']==3 and s['showMenu'] and s['panel']==0);capture('modal-background-release')
 cmd('stage',0,0,0,0);cmd('pause');tap(104,True);x,y=centre(1000);pointer(71,0,x,y);check('HUD drag hides tools while pointer remains held',not any(b['id']==21001 for b in state()['buttons']));tap(21005);tap(21001);x,y=centre(101);pointer(71,1,x,y);pointer(71,2,x,y);s=state();check('HUD drag release after CANCEL cannot activate original RESUME',s['paused'] and not s['editor']);capture('hud-drag-release')
 tap(104,True);cw,ch=state()['canvas'];pointer(72,0,cw/2,ch/2);tap(21001);x,y=centre(101);pointer(72,1,x,y);pointer(72,2,x,y);s=state();check('HUD blank-area release after CANCEL cannot activate original RESUME',s['paused'] and not s['editor']);capture('hud-blank-release')
finally:
 shell('am','force-stop',pkg);shell('run-as',pkg,'rm','-rf','files');r=subprocess.run(base+['shell','-T','run-as',pkg,'tar','-xf','-'],input=backup.stdout,capture_output=True);check('Original validation files restored',r.returncode==0 and all(file(n)==v for n,v in original_files.items()));shell('input','keyevent','3')
raise SystemExit(0 if all(row['passed'] for row in rows) else 1)
