#!/usr/bin/env python3
"""Render the Kamov armament report into the existing report Site; never publish."""
import argparse, shutil
from pathlib import Path
from bs4 import BeautifulSoup
from build_report_site import article
ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--checkout', type=Path, required=True)
parser.add_argument('--asset-commit', help='Immutable public GitHub commit for media and evidence')
args = parser.parse_args()
dist = args.checkout / 'dist'
if args.asset_commit:
    import re, subprocess
    if not re.fullmatch(r'[0-9a-f]{40}', args.asset_commit):
        parser.error('--asset-commit must be a full commit SHA')
    subprocess.run(['git', 'cat-file', '-e', args.asset_commit + '^{commit}'], cwd=ROOT, check=True)
    remote = 'https://raw.githubusercontent.com/gelb2/HeavyMach/' + args.asset_commit + '/docs/'
name = 'kamov-armament-hotfix-20261109'
source = ROOT / 'docs/reviews' / (name + '.md')
template = (dist / 'kamov-lux-stage-placement-20261109.html').read_text()
article(source, dist / (name + '.html'), name + '.md', template)
page = BeautifulSoup((dist / (name + '.html')).read_text(), 'html.parser')
for node in page.select('main a[href],main img[src]'):
    key = 'src' if node.name == 'img' else 'href'
    raw = node[key]
    if raw.startswith('../public/media/' + name + '/'):
        node[key] = (remote.replace('raw.githubusercontent.com/', 'media.githubusercontent.com/media/') + 'public/media/' if args.asset_commit else 'media/') + name + '/' + raw.rsplit('/', 1)[1]
    elif raw.startswith('../evidence/' + name + '/'):
        node[key] = (remote + 'evidence/' if args.asset_commit else 'evidence/') + name + '/' + raw.rsplit('/', 1)[1]
style = page.new_tag('style')
style.string = 'main img{display:block;max-width:100%;height:auto;margin:24px auto}'
page.head.append(style)
(dist / (name + '.html')).write_text(str(page))
if not args.asset_commit:
    media = dist / 'media' / name
    media.mkdir(parents=True, exist_ok=True)
    for node in page.select('main img[src]'):
        if node['src'].startswith('media/' + name + '/'):
            filename = node['src'].rsplit('/', 1)[1]
            shutil.copy2(ROOT / 'docs/public/media' / name / filename, media / filename)
    shutil.copytree(ROOT / 'docs/evidence' / name, dist / 'evidence' / name, dirs_exist_ok=True)
else:
    download = dist / (name + '.md')
    body = download.read_text()
    body = body.replace('../public/media/' + name + '/', remote.replace('raw.githubusercontent.com/', 'media.githubusercontent.com/media/') + 'public/media/' + name + '/')
    body = body.replace('../evidence/' + name + '/', remote + 'evidence/' + name + '/')
    download.write_text(body)
index = BeautifulSoup((dist / 'index.html').read_text(), 'html.parser')
if not index.select_one('a[href="' + name + '.html"]'):
    section = index.select_one('#reports') or index.main
    paragraph = index.new_tag('p')
    link = index.new_tag('a', href=name + '.html')
    link.string = '카모프 기관포 제거와 미사일 2연사 — 변경·웹 검사'
    paragraph.append(link)
    heading = section.find('h2')
    if heading: heading.insert_after(paragraph)
    else: section.insert(0, paragraph)
    (dist / 'index.html').write_text(str(index))
print(dist / (name + '.html'))
