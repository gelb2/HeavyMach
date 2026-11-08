#!/usr/bin/env python3
"""Regression for cache/output disagreement using the actual emitted artifacts."""
import pathlib, shutil, sys, tempfile, unittest
HERE=pathlib.Path(__file__).resolve().parents[1];ROOT=HERE.parents[1]
sys.path.insert(0,str(HERE/'scripts'))
from validate_web_artifact import validate_web_artifact
class ArtifactTest(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=pathlib.Path(self.temp.name)
    def copy(self, validation):
        source=ROOT/('build/web-unit-validation' if validation else 'build/web-unit-normal')/'dist'
        out=self.root/'dist';out.mkdir(exist_ok=True)
        for name in ('index.html','index.js','index.wasm'):shutil.copyfile(source/name,out/name)
        return out
    def test_real_normal_passes(self):validate_web_artifact(self.copy(False),validation=False)
    def test_real_validation_passes(self):validate_web_artifact(self.copy(True),validation=True)
    def test_no_cache_cheat_dist_rejected(self):
        with self.assertRaises(ValueError):validate_web_artifact(self.copy(True),validation=False)
    def test_false_cache_cannot_hide_cheat_dist(self):
        (self.root/'CMakeCache.txt').write_text('HM_WEB_VALIDATION:BOOL=OFF\n')
        with self.assertRaises(ValueError):validate_web_artifact(self.copy(True),validation=False)
    def test_normal_html_with_cheat_js_rejected(self):
        out=self.copy(False);shutil.copyfile(ROOT/'build/web-unit-validation/dist/index.js',out/'index.js')
        with self.assertRaises(ValueError):validate_web_artifact(out,validation=False)
    def test_normal_js_with_cheat_html_rejected(self):
        out=self.copy(False);shutil.copyfile(ROOT/'build/web-unit-validation/dist/index.html',out/'index.html')
        with self.assertRaises(ValueError):validate_web_artifact(out,validation=False)
    def test_normal_shell_with_validation_wasm_rejected(self):
        out=self.copy(False);shutil.copyfile(ROOT/'build/web-unit-validation/dist/index.wasm',out/'index.wasm')
        with self.assertRaises(ValueError):validate_web_artifact(out,validation=False)
    def test_validation_shell_with_normal_wasm_rejected(self):
        out=self.copy(True);shutil.copyfile(ROOT/'build/web-unit-normal/dist/index.wasm',out/'index.wasm')
        with self.assertRaises(ValueError):validate_web_artifact(out,validation=True)
    def test_validation_host_rejects_normal_dist(self):
        with self.assertRaises(ValueError):validate_web_artifact(self.copy(False),validation=True)
if __name__=='__main__':unittest.main()
