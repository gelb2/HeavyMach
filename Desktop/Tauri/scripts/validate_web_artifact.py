"""Check emitted game files, independently of the mutable CMake cache."""
import re
from pathlib import Path
EXPORTS = ('catalog', 'state', 'start', 'spawn', 'touch', 'cheats')

def validate_web_artifact(dist: Path, *, validation: bool) -> None:
    html = (dist / 'index.html').read_text()
    js = (dist / 'index.js').read_text()
    wasm = (dist / 'index.wasm').read_bytes()
    if not wasm.startswith(b'\0asm\x01\0\0\0'):
        raise ValueError('Missing valid WebAssembly artifact')
    flag = r'(?:true|!0)' if validation else r'(?:false|!1)'
    if not re.search(r'\bhmValidation\s*:\s*' + flag + r'\s*[,}]', html):
        raise ValueError('Emitted HTML has the wrong validation flag')
    prefix = 'HeavyMach.Validation.' if validation else 'HeavyMach.'
    if not re.search(r'\bhmPreferencePrefix\s*:\s*[\"\']' + re.escape(prefix) + r'[\"\']', html):
        raise ValueError('Emitted HTML has the wrong preference namespace')
    mount = '/validation-persist' if validation else '/persist'
    if not re.search(r'FS\.mkdir\(\s*[\"\']' + re.escape(mount) + r'[\"\']\s*\)', html):
        raise ValueError('Emitted HTML has the wrong persistent root')
    if not re.search(r'FS\.mount\(\s*Module\.IDBFS\s*,\s*\{\s*\}\s*,\s*[\"\']' + re.escape(mount) + r'[\"\']\s*\)', html):
        raise ValueError('Emitted HTML has the wrong IDBFS mount')
    if mount.encode() not in wasm or (not validation and b'/validation-persist' in wasm):
        raise ValueError('WebAssembly save root does not match its host mode')
    if validation:
        if any('_hm_validation_' + name not in js for name in EXPORTS):
            raise ValueError('Validation artifact is missing a required game export')
    elif '_hm_validation_' in js or '/validation-persist' in html or 'HeavyMach.Validation.' in html:
        raise ValueError('Refuse validation artifact in the production host')
