#!/usr/bin/env python3
"""Remove abandoned test PWA launchers, never browser profiles or game saves.

Default: report only. --apply moves verified orphan launchers to the Trash.
The installed local watcher runs every ten minutes, recording main changes.
"""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import sys
import tempfile
import uuid
from urllib.parse import urlsplit

LABEL = 'com.gelb2.mach-browser-cleanup'
NAMES = {'Heavy Mach': 'HeavyMach', 'MachDefense': 'MachDefense'}


def inspect_app(app, apps, temp_root):
    """Return a deletion candidate only for a missing Puppeteer temp profile."""
    if app.parent != apps or apps.is_symlink() or app.is_symlink() or not app.is_dir():
        return None
    if not any(re.fullmatch(re.escape(name) + r'(?: \d+)?\.app', app.name) for name in NAMES):
        return None
    # Do not follow substituted bundle components or unknown bundled data.
    for item in app.rglob('*'):
        if item.is_symlink() or item.name in {'HvM.dat', 'GameSave.dat'} or item.suffix.lower() in {'.ipa', '.sqlite', '.ldb'}:
            return None
        if item.is_file() and item.relative_to(app).parts[:2] not in {
            ('Contents', 'MacOS'), ('Contents', 'Resources'), ('Contents', '_CodeSignature')
        } and item.relative_to(app).as_posix() not in {'Contents/Info.plist', 'Contents/PkgInfo'}:
            return None
    info = app / 'Contents/Info.plist'
    try:
        raw = info.read_bytes()
        data = plistlib.loads(raw)
    except (OSError, ValueError, plistlib.InvalidFileException):
        return None
    name = data.get('CrAppModeShortcutName')
    app_id = data.get('CrAppModeShortcutID', '')
    if name not in NAMES or not re.fullmatch('[a-p]{32}', app_id):
        return None
    if data.get('CFBundleIdentifier') != 'com.google.Chrome.app.' + app_id:
        return None
    if data.get('CFBundleExecutable') != 'app_mode_loader' or data.get('CrBundleIdentifier') != 'com.google.Chrome':
        return None
    url = urlsplit(data.get('CrAppModeShortcutURL', ''))
    if not (url.scheme == 'http' and url.hostname in {'localhost', '127.0.0.1', '[::1]', '::1'} or
            url.scheme == 'https' and url.hostname == 'gelb2.github.io' and
            url.path.startswith('/' + NAMES[name] + '/')):
        return None
    profile_string, sep, tail = data.get('CrAppModeUserDataDir', '').partition('/-/')
    if not sep or tail != 'Web Applications/_crx_' + app_id:
        return None
    profile = Path(profile_string)
    if not profile.is_absolute() or profile.parent.resolve() != temp_root.resolve():
        return None
    if not re.fullmatch(r'puppeteer_dev_chrome_profile-[A-Za-z0-9_-]+', profile.name):
        return None
    if os.path.lexists(profile):
        return None
    st = app.stat()
    return {'path': str(app), 'game': NAMES[name], 'profile': str(profile),
            'info_sha256': hashlib.sha256(raw).hexdigest(),
            'device': st.st_dev, 'inode': st.st_ino, 'mtime_ns': st.st_mtime_ns}


def cleanup(home, temp_root, apply=False):
    apps = home / 'Applications/Chrome Apps.localized'
    trash = home / '.Trash'
    report = {'time': datetime.datetime.now(datetime.timezone.utc).isoformat(),
              'apply': apply, 'candidates': [], 'moved': [], 'errors': []}
    if not apps.is_dir() or apps.is_symlink():
        return report
    try:
        processes = subprocess.run(['ps', '-axo', 'command='], capture_output=True, text=True,
                                   check=True, timeout=10).stdout
    except (OSError, subprocess.SubprocessError):
        report['errors'].append({'error': 'Cannot inspect running apps; nothing moved'})
        return report
    for app in sorted(apps.glob('*.app')):
        try:
            candidate = inspect_app(app, apps, temp_root)
            if candidate is None:
                continue
            if str(app) in processes or candidate['profile'] in processes:
                continue
            report['candidates'].append(candidate)
            if not apply:
                continue
            # Revalidate immediately before moving; a restored/live profile is retained.
            if inspect_app(app, apps, temp_root) != candidate:
                continue
            trash.mkdir(mode=0o700, exist_ok=True)
            if trash.is_symlink():
                raise RuntimeError('Trash is a symlink; refusing to move')
            dest = trash / (app.stem + '-test-' + uuid.uuid4().hex + '.app')
            app.rename(dest)
            report['moved'].append({'from': str(app), 'to': str(dest)})
        except (OSError, ValueError, RuntimeError) as exc:
            report['errors'].append({'path': str(app), 'error': str(exc)})
    return report


def main_refs(repos):
    refs = {}
    for repo in repos:
        try:
            result = subprocess.run(['git', '-C', repo, 'ls-remote', '--exit-code', 'origin', 'refs/heads/main'],
                                    capture_output=True, text=True, timeout=25, check=True,
                                    env={**os.environ, 'GIT_TERMINAL_PROMPT': '0'})
            sha = result.stdout.split()[0]
            if not re.fullmatch('[0-9a-f]{40,64}', sha):
                raise ValueError('Invalid main reference')
            refs[repo] = {'sha': sha}
        except (OSError, ValueError, IndexError, subprocess.SubprocessError):
            refs[repo] = {'error': 'main lookup failed; retry next interval'}
    return refs


def watch(config_path):
    config = json.loads(config_path.read_text())
    state_dir = config_path.parent
    with (state_dir / 'watch.lock').open('a') as lock:
        try:
            fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            return
        state_file = state_dir / 'last-run.json'
        try:
            previous = json.loads(state_file.read_text()) if state_file.exists() else {}
            if not isinstance(previous, dict):
                raise ValueError('Invalid state')
        except (ValueError, OSError):
            previous = {}
            if state_file.exists():
                state_file.rename(state_dir / ('last-run-corrupt-' + uuid.uuid4().hex + '.json'))
        refs = main_refs(config['repos'])
        report = cleanup(Path.home(), Path(tempfile.gettempdir()), apply=True)
        report['main'] = refs
        report['main_changed'] = [repo for repo, ref in refs.items() if 'sha' in ref and
                                  ref != previous.get('main', {}).get(repo)]
        with tempfile.NamedTemporaryFile(mode='w', dir=state_dir, delete=False) as pending:
            pending.write(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
            pending.flush()
            os.fsync(pending.fileno())
        os.replace(pending.name, state_file)
        if report['moved'] or report['errors'] or report['main_changed']:
            with (state_dir / 'history.jsonl').open('a') as log:
                log.write(json.dumps(report, ensure_ascii=False) + '\n')
        print(json.dumps({'moved': len(report['moved']), 'errors': len(report['errors']),
                          'main_changed': report['main_changed']}))


def install(repos):
    if sys.platform != 'darwin':
        raise SystemExit('LaunchAgent installation requires macOS')
    state_dir = Path.home() / 'Library/Application Support/MachBrowserCleanup'
    state_dir.mkdir(parents=True, mode=0o700, exist_ok=True)
    dest = state_dir / 'cleanup_browser_apps.py'
    if Path(__file__).resolve() != dest.resolve():
        shutil.copy2(Path(__file__).resolve(), dest)
    config_path = state_dir / 'config.json'
    config_path.write_text(json.dumps({'repos': [str(Path(r).resolve()) for r in repos]}, indent=2) + '\n')
    plist_path = Path.home() / 'Library/LaunchAgents' / (LABEL + '.plist')
    plist_path.parent.mkdir(parents=True, exist_ok=True)
    plist_path.write_bytes(plistlib.dumps({
        'Label': LABEL,
        'ProgramArguments': [sys.executable, str(dest), '--watch', str(config_path)],
        'RunAtLoad': True, 'StartInterval': 600,
        'EnvironmentVariables': {'PATH': '/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin'},
        'StandardOutPath': str(state_dir / 'stdout.log'),
        'StandardErrorPath': str(state_dir / 'stderr.log'),
    }))
    domain = 'gui/' + str(os.getuid())
    subprocess.run(['launchctl', 'bootout', domain + '/' + LABEL], capture_output=True)
    subprocess.run(['launchctl', 'bootstrap', domain, str(plist_path)], check=True)
    print(json.dumps({'installed': str(plist_path), 'config': str(config_path), 'interval_seconds': 600}))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--apply', action='store_true')
    group.add_argument('--watch', type=Path)
    group.add_argument('--install-agent', action='store_true')
    parser.add_argument('--repo', action='append', default=[])
    args = parser.parse_args()
    if args.install_agent:
        if not args.repo:
            parser.error('--install-agent requires --repo')
        install(args.repo)
    elif args.watch:
        watch(args.watch)
    else:
        report = cleanup(Path.home(), Path(tempfile.gettempdir()), args.apply)
        print(json.dumps(report, ensure_ascii=False, indent=2))
        return bool(report['errors'])
    return 0


if __name__ == '__main__':
    sys.exit(main())
