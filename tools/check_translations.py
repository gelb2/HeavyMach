#!/usr/bin/env python3
"""Validate the explicit visitor-document translations, without loading game policies."""
import hashlib
import json
import re
import sys
from collections import Counter
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
ROOT_DOCS = ('README', 'CHANGELOG', 'CREDITS', 'CONTRIBUTING', 'LICENSE-SCOPE',
             'ASSET-LICENSES', 'THIRD_PARTY_NOTICES')
PUBLIC_DOCS = ('features', 'implementation-status', 'build-and-install', 'gameplay-guide',
               'architecture', 'development-history', 'troubleshooting', 'validation',
               'lux-lunae-intent-20261028', 'index', 'design-intent', 'roadmap',
               'android-ai-experience-20261030', 'save-recovery-20261102')
EXPECTED = {**{f'{n}.md': f'{n}.en.md' for n in ROOT_DOCS},
            'docs/RESTORATION.md': 'docs/en/RESTORATION.md',
            **{f'docs/public/{n}.md': f'docs/en/public/{n}.md' for n in PUBLIC_DOCS}}
NAV = re.compile(r'<!-- language-nav:start -->\n.*?\n<!-- language-nav:end -->\n\n', re.S)
LINK = re.compile(r'\]\((<[^>]+>|[^\s)]+)(?:\s+"[^"]*")?\)')
FENCE = re.compile(r'^```[^\n]*\n.*?^```[^\n]*', re.M | re.S)
ANCHOR = re.compile(r'<a id="([^"]+)"></a>\n\n')


def strip_nav(text):
    return NAV.sub('', text)


def digest(text):
    return hashlib.sha256(strip_nav(text).encode()).hexdigest()


def slug(text):
    text = re.sub(r'!?\[([^\]]+)\]\([^)]*\)', r'\1', text)
    text = re.sub(r'<[^>]+>', '', text).strip().lower()
    text = re.sub(r'[^\w\-\s]', '', text, flags=re.UNICODE)
    return re.sub(r'\s', '-', text)


def headings(text):
    return re.findall(r'^(#{1,6}) (.+)$', FENCE.sub('', text), re.M)


def anchors(text):
    ids = set(re.findall(r'\bid=["\']([^"\']+)["\']', text))
    counts = Counter()
    for _, title in headings(text):
        base = slug(title)
        ids.add(base if not counts[base] else f'{base}-{counts[base]}')
        counts[base] += 1
    return ids


def check_scope(scope):
    pairs = [(p['korean'], p['english']) for p in scope['documents']]
    return [] if len(pairs) == len(EXPECTED) and dict(pairs) == EXPECTED else [
        'Translation scope differs from the approved visitor documents']


def resolved_links(root, name, text):
    found, errors = [], []
    path = root / name
    for target in LINK.findall(FENCE.sub('', strip_nav(text))):
        url = urlsplit(target.strip('<>'))
        if url.scheme or url.netloc:
            found.append(('external', target))
            continue
        dest = (path.parent / unquote(url.path)).resolve() if url.path else path.resolve()
        try:
            rel = dest.relative_to(root.resolve()).as_posix()
        except ValueError:
            errors.append(f'{name}: link escapes repository: {target}')
            continue
        if not dest.exists():
            errors.append(f'{name}: missing link: {target}')
        elif url.fragment and dest.suffix == '.md':
            if unquote(url.fragment) not in anchors(dest.read_text()):
                errors.append(f'{name}: missing anchor: {target}')
        found.append(('local', rel, unquote(url.fragment)))
    return found, errors


def check_pair(root, ko, en, record):
    errors = []
    if not (root / ko).is_file() or not (root / en).is_file():
        return [f'Missing translation pair: {ko} / {en}']
    source, translation = (root / ko).read_text(), (root / en).read_text()
    if digest(source) != record.get('source_sha256'):
        errors.append(f'{ko}: source changed; translation review required')
    if digest(translation) != record.get('english_sha256'):
        errors.append(f'{en}: translation changed; review required')
    for name, text, other in ((ko, source, en), (en, translation, ko)):
        nav = NAV.findall(text)
        if len(nav) != 1:
            errors.append(f'{name}: expected one language navigation block')
        else:
            links = LINK.findall(nav[0])
            destinations = {(root / name).parent.joinpath(x).resolve() for x in links}
            if destinations != {(root / ko).resolve(), (root / en).resolve()}:
                errors.append(f'{name}: wrong language destinations')
    def identifiers(text):
        return Counter(re.findall(r'(?<!`)`([^`\n]+)`(?!`)', FENCE.sub('', strip_nav(text))))
    if identifiers(source) != identifiers(translation):
        errors.append(f'{en}: inline code identifiers differ')
    if FENCE.findall(source) != FENCE.findall(translation):
        errors.append(f'{en}: code block differs from Korean source')
    if [x[0] for x in headings(source)] != [x[0] for x in headings(translation)]:
        errors.append(f'{en}: heading structure differs')
    source_rows = [line for line in source.splitlines() if line.startswith('|')]
    english_rows = [line for line in translation.splitlines() if line.startswith('|')]
    if [x.count('|') for x in source_rows] != [x.count('|') for x in english_rows]:
        errors.append(f'{en}: table structure differs')
    src_links, problems = resolved_links(root, ko, source)
    errors.extend(problems)
    dst_links, problems = resolved_links(root, en, translation)
    errors.extend(problems)
    expected = Counter((kind, EXPECTED.get(path, path), fragment) if kind == 'local' else item
                       for item in src_links for kind, path, *tail in [item]
                       for fragment in [tail[0] if tail else ''])
    if expected != Counter(dst_links):
        errors.append(f'{en}: link targets differ from source/translation mapping')
    # Numeric differences are reviewed explicitly (e.g. Korean number words and timestamps).
    def numbers(text):
        text = LINK.sub('](link)', FENCE.sub('', strip_nav(text)))
        text = re.sub(r'<[^>]+>', '', text)
        return Counter(re.findall(r'(?<![\w])\d+(?:[.,]\d+)*(?:%|ms|rad)?', text))
    delta = {'source_only': dict(numbers(source) - numbers(translation)),
             'english_only': dict(numbers(translation) - numbers(source))}
    if delta != record.get('reviewed_numeric_difference'):
        errors.append(f'{en}: numeric differences need review: {delta}')
    body = ANCHOR.sub('', FENCE.sub('', strip_nav(translation)))
    if len(re.findall(r'[A-Za-z]{2,}', body)) < 30:
        errors.append(f'{en}: English prose missing')
    return errors


def numeric_difference(source, translation):
    def nums(text):
        text = LINK.sub('](link)', FENCE.sub('', strip_nav(text)))
        text = re.sub(r'<[^>]+>', '', text)
        return Counter(re.findall(r'(?<![\w])\d+(?:[.,]\d+)*(?:%|ms|rad)?', text))
    return {'source_only': dict(nums(source) - nums(translation)),
            'english_only': dict(nums(translation) - nums(source))}


def check(root=ROOT):
    scope = json.loads((root / 'docs/public/document-translation-scope.json').read_text())
    errors = check_scope(scope)
    manifest = json.loads((root / 'docs/translations/manifest.json').read_text())
    records = manifest['documents']
    if set(records) != set(EXPECTED):
        errors.append('Manifest must contain exactly the approved source paths')
    if errors:
        return errors
    for ko, en in EXPECTED.items():
        errors += check_pair(root, ko, en, records[ko])
    return errors


if __name__ == '__main__':
    errors = check()
    for error in errors:
        print('FAIL:', error)
    if not errors:
        print(f'PASS: {len(EXPECTED)} translation pairs, hashes, language navigation, headings, tables, code, links and reviewed numeric differences')
    sys.exit(bool(errors))
