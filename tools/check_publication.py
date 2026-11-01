#!/usr/bin/env python3
"""Check reader-facing local links, the shared introduction, and source copies."""
import hashlib
import json
import re
from pathlib import Path
from check_translations import EXPECTED, strip_nav
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
FILES = ['docs/public/lux-lunae-intent-20261028.md', 'README.md', 'CONTRIBUTING.md', 'LICENSE-SCOPE.md', 'ASSET-LICENSES.md',
         'THIRD_PARTY_NOTICES.md', 'CREDITS.md', 'CHANGELOG.md', 'docs/RESTORATION.md',
         'docs/README.md', 'docs/developer-background.md', 'docs/public/publication-inventory.md',
         'docs/public/features.md', 'docs/public/gameplay-guide.md',
         'docs/public/build-and-install.md', 'docs/public/architecture.md',
         'docs/public/troubleshooting.md', 'docs/public/development-history.md',
         'docs/public/validation.md', 'docs/public/implementation-status.md',
         'docs/public/github-repository-plan-20261028.md', 'docs/public/github-followup-20261028.md']

FILES += [p for p in EXPECTED if p not in FILES] + list(EXPECTED.values())

# Known rejected copy, checked in every report manuscript as well as reader docs.
# This catches recurrence; contextual editing still determines what to rewrite.
REJECTED_COPY = [
    r'OS(?: 화면)? 전체(?:를 찍은)? 스크린샷은 아니다',
    r'실시간\s*60\s*fps를 측정한 값으로 사용하지 않는다',
    r'(?:실기기 성능|실제 iPad GPU 시간|실제 FPS)(?:으로|로) (?:사용|환산|해석)하지 (?:않는다|않았다)',
    r'(?:검증했다고 표시|전체 통과로 판정|모든 프레임이16\.67ms 이내라고 표현)하지 않는다',
    r'(?:이번에 줄 단위 의미 동등성을 재검사|모든 미션의 모든 지형에서 명중을 확인)했다는 뜻은 아니다',
]


EN_REJECTED_COPY = [r'(?i)it is (?:important|worth) (?:to note|noting) that',
                    r'(?i)(?:this does not constitute|should not be construed as|not a claim of)']


def editorial_files(root):
    extra = ['docs/documentation-audit-20261025.md', 'docs/related-titles-20261026.md',
             'docs/android-port-plan-20261026.md', 'docs/ios-completeness-audit-20261026.md',
             'docs/auto-aim-plan-20261026.md', 'docs/lux-lunae-plan-20261027.md',
             'docs/media-publication.md', 'docs/public/media/README.md']
    return sorted(set(FILES + extra + [str(p.relative_to(root))
                  for p in (root / 'docs/public').glob('*.md')] +
                  [str(p.relative_to(root)) for p in (root / 'docs').glob('*.md')
                   if p.name not in ('current-status.md', 'report-site.md')]))


def check_editorial(root):
    errors = []
    for name in editorial_files(root):
        for number, line in enumerate((root / name).read_text().splitlines(), 1):
            if line.lstrip().startswith(('>', '```')):
                continue
            patterns = REJECTED_COPY + (EN_REJECTED_COPY if name in EXPECTED.values() else [])
            if any(re.search(pattern, line) for pattern in patterns):
                errors.append(f'{name}:{number}: rejected editorial copy')
    for name in ('README.md', 'docs/public/build-and-install.md',
                 'docs/public/implementation-status.md'):
        text = (root / name).read_text()
        for stale in ('현재 저장소 접근에는 GitHub 권한이 필요하다',
                      '현재 저장소는 비공개', '저장소와 보고 사이트는 비공개',
                      '원작 에셋의 공개 이용허락은 아직 확인 중이다',
                      '코드와 에셋을 함께 공개해 누구나 빌드하고 플레이할 수 있도록 준비하고 있다'):
            if stale in text:
                errors.append(f'{name}: outdated publication status')
    return errors


def check(root):
    errors = check_editorial(root)
    links = 0
    for name in FILES:
        path = root / name
        if not path.is_file():
            errors.append(f'Missing document: {name}')
            continue
        # Fence examples can contain sample paths that are not document links.
        content = re.sub(r'^```.*?^```[^\n]*', '', path.read_text(), flags=re.M | re.S)
        for target in re.findall(r'\]\((<[^>]+>|[^\s)]+)(?:\s+"[^"]*")?\)', content):
            url = urlsplit(target.strip('<>'))
            if url.scheme or url.netloc or not url.path:
                continue
            links += 1
            dest = path.parent / unquote(url.path)
            if not dest.exists():
                errors.append(f'{name}: missing local link {target}')
    readme_text = strip_nav((root / 'README.md').read_text())
    for name, motive in [('README.md', '## HeavyMach를 다시 만든 이유'),
                         ('README.en.md', '## Why I rebuilt HeavyMach')]:
        text = strip_nav((root / name).read_text())
        quick = text.find('## Quick Play — PWA · Mac')
        badges = text.find('[![')
        heading = text.find(motive)
        if not (0 < quick < badges < heading):
            errors.append(f'{name}: expected Quick Play, badges, then the motive')
        if text[:quick].strip() != '# HeavyMach':
            errors.append(f'{name}: Quick Play must follow the title and language links')
        intro = text[quick:badges]
        for target in ('https://gelb2.github.io/HeavyMach/', '#tauri-mac',
                       'https://github.com/fastutlego', 'https://github.com/gelb2/HeavyMach/pull/24',
                       '#빌드하기'):
            if f']({target})' not in intro:
                errors.append(f'{name}: Quick Play missing {target}')
    approved = (root / 'docs/references/developer-background/approved-readme-opening-main-20261029.md').read_text().strip()
    expected = '#' + approved
    if expected not in readme_text:
        errors.append('README opening differs from the approved manuscript')
    restoration_paragraphs = strip_nav((root / 'docs/RESTORATION.md').read_text()).split('\n\n')
    shared = restoration_paragraphs[1:4]
    shared += [p for p in restoration_paragraphs if p.startswith('요즘 나는 내게 남은 시간이')]
    if any(p not in expected for p in shared):
        errors.append('README and restoration shared paragraphs differ')
    main_edits = json.loads((root / 'docs/references/developer-background/main-edits-20261029.json').read_text())
    for name, digest in main_edits['files'].items():
        if hashlib.sha256(strip_nav((root / name).read_text()).encode()).hexdigest() != digest:
            errors.append(f'Changed author manuscript from main: {name}')
    sources = root / 'docs/references/developer-background'
    manifest = json.loads((sources / 'source-manifest.json').read_text())
    entries = manifest['sources'] + [manifest['user_statement']] + manifest['user_followups']
    for item in entries:
        expected = item.get('copy_sha256', item.get('sha256'))
        if hashlib.sha256((sources / item['copy']).read_bytes()).hexdigest() != expected:
            errors.append(f'Changed preserved source: {item["copy"]}')
    return errors, links


if __name__ == '__main__':
    errors, count = check(ROOT)
    for error in errors:
        print('FAIL:', error)
    if not errors:
        print(f'PASS: {len(FILES)} linked documents, {len(editorial_files(ROOT))} editorial documents, '
              f'{count} local links, shared introduction and source hashes')
    raise SystemExit(bool(errors))
