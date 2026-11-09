import importlib.util
import json
from pathlib import Path
import plistlib
import tempfile
import unittest
from unittest.mock import patch, Mock

SPEC = importlib.util.spec_from_file_location('cleanup', Path(__file__).parents[1] / 'cleanup_browser_apps.py')
cleanup = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(cleanup)


class CleanupTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.home = Path(self.tmp.name) / 'home'
        self.temp = Path(self.tmp.name) / 'temp'
        self.temp.mkdir()
        self.apps = self.home / 'Applications/Chrome Apps.localized'
        self.apps.mkdir(parents=True)
        self.profile = self.temp / 'puppeteer_dev_chrome_profile-test'
        self.app = self.apps / 'Heavy Mach 9.app'
        self.info = self.app / 'Contents/Info.plist'
        self.info.parent.mkdir(parents=True)
        self.data = {
            'CFBundleIdentifier': 'com.google.Chrome.app.' + 'a'*32,
            'CFBundleExecutable': 'app_mode_loader',
            'CrBundleIdentifier': 'com.google.Chrome',
            'CrAppModeShortcutID': 'a'*32,
            'CrAppModeShortcutName': 'Heavy Mach',
            'CrAppModeShortcutURL': 'https://gelb2.github.io/HeavyMach/',
            'CrAppModeUserDataDir': str(self.profile) + '/-/Web Applications/_crx_' + 'a'*32,
        }
        self.save()

    def save(self):
        self.info.write_bytes(plistlib.dumps(self.data))

    def run_cleanup(self, apply=True):
        return cleanup.cleanup(self.home, self.temp, apply)

    def test_orphan_moves_to_trash_without_touching_other_apps(self):
        other = self.apps / 'Unrelated.app'
        other.mkdir()
        report = self.run_cleanup()
        self.assertEqual(len(report['moved']), 1)
        self.assertFalse(self.app.exists())
        self.assertTrue(Path(report['moved'][0]['to']).joinpath('Contents/Info.plist').exists())
        self.assertTrue(other.exists())
        self.assertEqual(self.run_cleanup()['moved'], [])

    def test_dry_run_does_not_move(self):
        self.assertEqual(len(self.run_cleanup(False)['candidates']), 1)
        self.assertTrue(self.app.exists())

    def test_live_temp_profile_and_save_are_preserved(self):
        self.profile.mkdir()
        save = self.profile / 'HvM.dat'
        save.write_bytes(b'actual progress')
        self.assertEqual(self.run_cleanup()['moved'], [])
        self.assertEqual(save.read_bytes(), b'actual progress')

    def test_missing_persistent_profile_is_not_removed(self):
        self.data['CrAppModeUserDataDir'] = str(self.home/'Chrome') + '/-/Web Applications/_crx_' + 'a'*32
        self.save()
        self.assertEqual(self.run_cleanup()['moved'], [])

    def test_same_name_different_origin_is_preserved(self):
        self.data['CrAppModeShortcutURL'] = 'https://example.org/HeavyMach/'
        self.save()
        self.assertEqual(self.run_cleanup()['moved'], [])

    def test_unknown_bundle_content_or_save_is_preserved(self):
        save = self.app / 'Contents/Resources/HvM.dat'
        save.parent.mkdir()
        save.write_bytes(b'progress')
        self.assertEqual(self.run_cleanup()['moved'], [])
        save.unlink()
        (self.app/'notes.txt').write_text('user note')
        self.assertEqual(self.run_cleanup()['moved'], [])

    def test_symlink_bundle_component_is_preserved(self):
        self.info.unlink()
        target = self.home/'external.plist'
        target.write_bytes(plistlib.dumps(self.data))
        self.info.symlink_to(target)
        self.assertEqual(self.run_cleanup()['moved'], [])
        self.assertTrue(target.exists())

    def test_profile_returning_between_checks_is_preserved(self):
        inspect = cleanup.inspect_app
        count = 0
        def changed(*args):
            nonlocal count
            count += 1
            if count == 2:
                self.profile.mkdir()
            return inspect(*args)
        with patch.object(cleanup, 'inspect_app', changed):
            self.assertEqual(self.run_cleanup()['moved'], [])
        self.assertTrue(self.app.exists())

    def test_machdefense_local_test_app(self):
        self.data['CrAppModeShortcutName'] = 'MachDefense'
        self.data['CrAppModeShortcutURL'] = 'http://127.0.0.1:8946/'
        self.save()
        new = self.apps/'MachDefense 7.app'
        self.app.rename(new)
        self.assertEqual(len(self.run_cleanup()['moved']), 1)

    def test_running_launcher_is_preserved(self):
        process = Mock(stdout=str(self.app/'Contents/MacOS/app_mode_loader'))
        with patch.object(cleanup.subprocess, 'run', return_value=process):
            self.assertEqual(self.run_cleanup()['moved'], [])
        self.assertTrue(self.app.exists())

    def test_trash_symlink_is_rejected(self):
        elsewhere = self.home/'elsewhere'
        elsewhere.mkdir()
        (self.home/'.Trash').symlink_to(elsewhere)
        report = self.run_cleanup()
        self.assertEqual(len(report['errors']), 1)
        self.assertTrue(self.app.exists())
        self.assertEqual(list(elsewhere.iterdir()), [])

    def test_corrupt_watch_state_recovers_and_cleans(self):
        config = self.home/'config.json'
        config.write_text(json.dumps({'repos': []}))
        (self.home/'last-run.json').write_text('{broken')
        with patch.object(cleanup.Path, 'home', return_value=self.home), \
             patch.object(cleanup.tempfile, 'gettempdir', return_value=str(self.temp)):
            cleanup.watch(config)
        report = json.loads((self.home/'last-run.json').read_text())
        self.assertEqual(len(report['moved']), 1)
        self.assertEqual(len(list(self.home.glob('last-run-corrupt-*.json'))), 1)

    def test_process_inspection_failure_never_moves(self):
        with patch.object(cleanup.subprocess, 'run', side_effect=OSError('ps failed')):
            report = self.run_cleanup()
        self.assertEqual(report['moved'], [])
        self.assertEqual(len(report['errors']), 1)
        self.assertTrue(self.app.exists())

    def test_watch_detects_main_changes_and_retries_periodically(self):
        config = self.home/'config.json'
        config.write_text(json.dumps({'repos': ['/repo']}))
        first = {'/repo': {'sha': 'a'*40}}
        second = {'/repo': {'sha': 'b'*40}}
        with patch.object(cleanup, 'main_refs', side_effect=[first, first, second]), \
             patch.object(cleanup.Path, 'home', return_value=self.home), \
             patch.object(cleanup.tempfile, 'gettempdir', return_value=str(self.temp)):
            cleanup.watch(config)
            self.assertEqual(json.loads((self.home/'last-run.json').read_text())['main_changed'], ['/repo'])
            cleanup.watch(config)
            self.assertEqual(json.loads((self.home/'last-run.json').read_text())['main_changed'], [])
            cleanup.watch(config)
            self.assertEqual(json.loads((self.home/'last-run.json').read_text())['main_changed'], ['/repo'])
        self.assertFalse(self.app.exists())


if __name__ == '__main__':
    unittest.main()
