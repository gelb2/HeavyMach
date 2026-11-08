#!/usr/bin/env python3
"""Render the Kamov armament report into the existing report Site; never publish."""
import argparse, shutil
from pathlib import Path
from bs4 import BeautifulSoup
from build_report_site import article
ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--checkout', type=Path, required=True)
args = parser.parse_args()
dist = args.checkout / 'dist'
name = 'kamov-armament-hotfix-20261109'
source = ROOT / 'docs/reviews' / (name + '.md')
template = (dist / 'kamov-lux-stage-placement-20261109.html').read_text()
article(source, dist / (name + '.html'), name + '.md', template)
page = BeautifulSoup((dist / (name + '.html')).read_text(), 'html.parser')
for node in page.select('main a[href],main img[src]'):
    key = 'src' if node.name == 'img' else 'href'
    raw = node[key]
    if raw.startswith('../public/media/' + name + '/'):
        node[key] = 'media/' + name + '/' + raw.rsplit('/', 1)[1]
    elif raw.startswith('../evidence/' + name + '/'):
        node[key] = 'evidence/' + name + '/' + raw.rsplit('/', 1)[1]
style = page.new_tag('style')
style.string = 'main img{display:block;max-width:100%;height:auto;margin:24px auto}'
page.head.append(style)
(dist / (name + '.html')).write_text(str(page))
for source_folder, dest_folder in [('docs/public/media', 'media'), ('docs/evidence', 'evidence')]:
    shutil.copytree(ROOT / source_folder / name, dist / dest_folder / name, dirs_exist_ok=True)
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
