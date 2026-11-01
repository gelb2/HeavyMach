#!/usr/bin/env python3
"""Check held keys and keyboard settings across game-tool/door transitions."""
from pathlib import Path
source=Path(__file__).with_name('validate_game_buttons.py').read_text().split("try:\n shell('am','start'")[0]
exec(source)
try:
 shell('am','start','-W','-n',pkg+'/com.gelb2.heavymach.GameActivity');time.sleep(1);screen=capture('start')
 cmd('fixtureSettings',0,0,0,0);cmd('cheats',1);cmd('stage',0,0,0,0);cmd('aim',1);time.sleep(.4)
 shell('input','keycombination','-t','400','34','136');time.sleep(.2);a=state();time.sleep(1.2);b=state()
 check('F6 stops held FIRE before consuming its key-up',b['shots']==a['shots'],shotsBefore=a['shots'],shotsAfter=b['shots']);capture('fire-f6')
 cmd('interrupt');shell('input','keyevent','111');cmd('resume');shell('input','keycombination','-t','400','32','136');time.sleep(.5);a=state();time.sleep(.8);b=state()
 check('F6 stops held MOVE before consuming its key-up',abs(b['x']-a['x'])<2,xBefore=a['x'],xAfter=b['x']);capture('move-f6')
 cmd('interrupt');shell('input','keyevent','111');s=state();v=next(b for b in s['originalButtons'] if b['id']==102);x,y,w,h=v['rect'];cw,ch=s['canvas'];shell('input','tap',int((x+w/2)*screen[0]/cw),int((ch-y-h/2)*screen[1]/ch));shell('input','keyevent','132');time.sleep(1.5)
 s=state();check('F2 during door transition cannot freeze hidden key settings',any(b['id']==22000 for b in s['buttons']),screen=s['screen'],buttons=[b['id'] for b in s['buttons']]);capture('door-f2')
finally:
 shell('am','force-stop',pkg);shell('run-as',pkg,'rm','-rf','files');r=subprocess.run(base+['shell','-T','run-as',pkg,'tar','-xf','-'],input=backup.stdout,capture_output=True)
 check('Original validation files restored',r.returncode==0 and all(file(n)==v for n,v in original_files.items()));shell('input','keyevent','3')
raise SystemExit(0 if all(row['passed'] for row in rows) else 1)
