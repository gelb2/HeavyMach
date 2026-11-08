#!/usr/bin/env python3
"""Observe eight unit sprites and HP loss from ordinary auto-aim bullets on Android.

Uses the shell-only cheat receiver; no direct damage or bullet injection. Every
fixture starts the real campaign map, then adds one enemy through GameSession.
"""
from pathlib import Path
import argparse, hashlib, json, os, subprocess, time

p = argparse.ArgumentParser()
p.add_argument('--serial', required=True)
a = p.parse_args()
root = Path(__file__).resolve().parents[2]
adb = Path(os.environ.get('ANDROID_SDK_ROOT', Path.home() / 'Library/Android/sdk')) / 'platform-tools/adb'
base = [str(adb), '-s', a.serial]
pkg = 'com.gelb2.heavymach.validation'
out = root / 'Android/build/evidence' / time.strftime('unit-contract-%Y%m%d-%H%M%S')
out.mkdir(parents=True)
rows = []
units = [('VTOL_Yak141', 'VTOL_Yak141.png'), ('VTOL_Harrier', 'VTOL_Harrier.png'),
         ('VTOL_F35B', 'VTOL_F35B.png'), ('Lux_F22', 'Lux_F22.png'),
         ('Lux_Su57', 'Lux_Su57.png'), ('Lux_MiG31', 'Lux_MiG31.png'),
         ('GAA_Shilka_M1', 'GAA_Shilka_Body'), ('GAA_Tunguska_M1', 'GAA_Tunguska_Body')]

def shell(*args):
    return subprocess.check_output(base + ['shell', *map(str, args)], text=True)

def cmd(name, *values):
    args = ['am', 'broadcast', '-n', pkg + '/com.gelb2.heavymach.TestReceiver',
            '-a', 'com.gelb2.heavymach.TEST', '--es', 'command', name]
    for key, value in zip('abcd', values):
        args += ['--ei', key, str(value)]
    result = shell(*args)
    if 'Exception' in result:
        raise RuntimeError(result)

def dump():
    shell('run-as', pkg, 'rm', '-f', 'files/diagnostics.json')
    cmd('dump')
    result = json.loads(subprocess.check_output(base + ['exec-out', 'run-as', pkg, 'cat', 'files/diagnostics.json']))
    if result.get('error'):
        raise RuntimeError(result['error'])
    return result

apk = root / 'Android/app/build/outputs/apk/cheat/debug/app-cheat-debug.apk'
(out / 'environment.json').write_text(json.dumps({
    'serial': a.serial, 'model': shell('getprop', 'ro.product.model').strip(),
    'os': shell('getprop', 'ro.build.version.release').strip(),
    'apk_sha256': hashlib.sha256(apk.read_bytes()).hexdigest(),
    'cheats': 'player invulnerability / infinite ammo; enemy HP and bullet damage unchanged',
    'fixture': 'campaign 1-1, level 0, default gun; one added enemy 200 units ahead',
}, indent=2))
shell('am', 'start', '-W', '-n', pkg + '/com.gelb2.heavymach.GameActivity')
time.sleep(.5)
try:
    cmd('cancel'); cmd('fixtureSettings', 0, 0, 0, 0); cmd('cheats', 1)
    for index, (name, tile) in enumerate(units):
        cmd('stage', 0, 0, 0, 0); cmd('aim', 0); time.sleep(.5)
        cmd('auditUnit', index); time.sleep(.2)
        before = dump()
        target = next(e for e in before['enemyStates'] if e['name'] == name)
        observed = [before]
        # GAA's common renderer selects baked Body/Body2 animation tiles.
        drawn = any((d['tile'] == tile if index < 6 else d['tile'].startswith(tile))
                    and d['alpha'] > 0 for d in before['unitDraws'])
        (out / (name + '.png')).write_bytes(subprocess.check_output(base + ['exec-out', 'screencap', '-p']))
        cmd('aim', 1); cmd('fire', 1)
        damaged = False
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            time.sleep(.1)
            state = dump(); observed.append(state)
            enemy = next((e for e in state['enemyStates'] if e['id'] == target['id']), None)
            # Disappearance alone is not proof of bullet damage.
            if enemy and enemy['hp'] < target['hp'] and state['shots'] > before['shots']:
                damaged = True
                break
        cmd('fire', 0)
        row = {'unit': name, 'passed': drawn and damaged, 'drawn': drawn,
               'actual_hp_damage': damaged, 'initial_hp': target['hp'],
               'observations': len(observed)}
        rows.append(row)
        (out / (name + '.json')).write_text(json.dumps(observed, indent=2))
        (out / 'results.json').write_text(json.dumps(rows, indent=2))
        print(json.dumps(row), flush=True)
finally:
    shell('am', 'force-stop', pkg)
    shell('input', 'keyevent', 'KEYCODE_HOME')
print('EVIDENCE ' + str(out))
raise SystemExit(0 if len(rows) == 8 and all(r['passed'] for r in rows) else 1)
