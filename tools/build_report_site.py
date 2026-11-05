#!/usr/bin/env python3
"""Render approved HeavyMach report manuscripts into the existing Sites checkout.
Requires Python packages Markdown and beautifulsoup4. Does not deploy.
"""
import argparse,json,re,shutil,hashlib
from pathlib import Path
from bs4 import BeautifulSoup
import markdown
ROOT=Path(__file__).resolve().parents[1]

def render(text):
    return BeautifulSoup(markdown.markdown(text,extensions=['tables','fenced_code','attr_list']), 'html.parser')

def check_runtime_art_references():
    source=ROOT/'docs/public/ground-aa-art-20261028.md'
    for img in render(source.read_text()).select('img[data-runtime-asset]'):
        shown=(source.parent/img['src']).resolve()
        runtime=ROOT/img['data-runtime-asset']
        if hashlib.sha256(shown.read_bytes()).digest()!=hashlib.sha256(runtime.read_bytes()).digest():
            raise ValueError(f'Current art reference differs from runtime: {shown} != {runtime}')

def deduplicate_styles(soup):
    # Keep the last occurrence so CSS precedence stays unchanged on regeneration.
    seen=set()
    for style in reversed(soup.select('head style')):
        css=style.get_text()
        if css in seen: style.decompose()
        else: seen.add(css)

def tables(soup):
    for table in soup.select('table'):
        headers=[h.get_text(' ',strip=True) for h in table.select('th')]
        for tr in table.select('tr'):
            for i,td in enumerate(tr.select('td')):
                if i<len(headers): td['data-label']=headers[i]
        wrapper=soup.new_tag('div',attrs={'class':'tablewrap table-scroll'});table.wrap(wrapper)

def article(source,dest,download,template):
    raw=source.read_text(); content=render(raw); title=content.h1.get_text() if content.h1 else source.stem
    old=BeautifulSoup(template,'html.parser')
    if old.h1 and old.h1.get('id') and content.h1: content.h1['id']=old.h1['id']
    ids=[x.get('id') for x in old.select('h2')]
    # README sections keep their existing URLs when gameplay sections move.
    readme_ids = {
        'HeavyMach를 다시 만든 이유': 'restoration-motive',
        '간단한 설명과 설치': 'introduction-and-installation',
        '원작의 전투와 조작': 'section-0', '원작의 무기와 성장': 'section-1',
        '내가 추가한 항공기': 'section-2', '내가 추가한 자동조준': 'auto-aim',
        '내가 추가한 지상 유닛': 'ground-units', 'HUD 위치 편집': 'hud-layout',
        '플랫폼과 개발 상태': 'section-3', '빌드하기': 'section-4',
        '개발 중 고친 문제': 'section-5', '코드 구조와 버전': 'section-6',
        '라이선스와 자원': 'section-7',
    } if source == ROOT/'README.md' else {}
    if source == ROOT/'docs/public/ground-aa-art-20261028.md':
        readme_ids.update({name:f'section-{i}' for i,name in enumerate(['쉴카','퉁구스카','확인한 원작 조립 구조','덜컹거림과 파츠 조립','제작 자료','초기 게임 화면과 무장','미션별 수량','구현 검사 결과','초기 실기기 검사'])})
        readme_ids['크기·연결부·레이더·포신 수정']='refinement'
        readme_ids['포신 단축·접합부 재수정과 외형 개선 계획']='barrels-and-art-plan'
        readme_ids['색·질감 수정 시안']='texture-proposal'
        readme_ids['공중 유닛 화풍을 참조한 수정 시안']='aircraft-style'
        readme_ids['현재 에셋으로 다시 그린 두 시안']='runtime-style'
        readme_ids['새 파츠와 레이더 적용']='integrated'
        readme_ids['쉴카 포탑 높이 수정']='shilka-turret'
        readme_ids['두 포탑 높이 맞춤']='matched-height'
        readme_ids['GitHub main 독립 빌드 재검사']='independent-build'
        readme_ids['해리어 참조 오류 정정']='reference-audit'
    if source == ROOT/'docs/ground-aa-expansion.md':
        readme_ids.update({name:f'section-{i}' for i,name in enumerate(['요구와 무장','파츠와 움직임','체력과 배치','검사'])})
        readme_ids['크기와 연결부 수정 · 2026-10-28']='refinement'
    if source == ROOT/'docs/public/hud-improvement-plan-20261028.md':
        readme_ids.update({name:f'section-{i}' for i,name in enumerate(['현재 화면과 코드','이동할 묶음과 그림 분할','배치·그림·터치를 연결하는 방법','게임 시작 전 편집 화면','자동조준 상태를 알리는 방법','구현과 검증 순서','투두에서 함께 확인한 남은 일','코드와 정책'])})
        readme_ids['미션 선택 버튼 제거 · 2026-10-29']='mission-entry-removal'
        readme_ids['HUD 부품과 분할 검사']='asset-audit'
        readme_ids['편집 구현과 시뮬레이터 검사']='editor-validation'
        readme_ids['실제 iPad 설치와 조작 검사']='physical-ipad'
        readme_ids['편집 도구 디자인 변경 계획']='editor-design'
        readme_ids['HUD 편집 도구·상단 장식 수정']='editor-release'
        readme_ids['실제 iPhone 설치와 조작 검사']='physical-iphone'
        readme_ids['원작 버튼에 맞춘 멈춤 메뉴']='pause-menu'
    if source == ROOT/'docs/public/lux-lunae-intent-20261028.md':
        readme_ids.update({name:f'section-{i}' for i,name in enumerate(['달빛과 두 게임','검은색과 금색','보스전의 난이도와 긴장감','텍스트 지시로 어디까지 만들 수 있는가','승인한 도안 3종','실제 인게임 모습','게임에 적용한 조건','대화 원문'])})
        readme_ids.update({'게임을 하고 달라진 일':'games-and-life','움직임과 무기, 빠른 진입':'movement-and-entry','그림 작업에서 본 한계':'art-direction','지금도 고치고 있는 것':'ongoing-work'})
    if source == ROOT/'docs/public/lux-formation-plan-20261029.md':
        readme_ids.update({'X·Y 배치 적용과 기기 검사': 'implementation', '원작 공중 적의 생성 위치': 'original-spawns', '생성 높이만 바꾼 추가 비교': 'vertical-spawns', '현재 코드에서 확인한 원인': 'section-0', '게임 코어 비교': 'section-1', '수정할 배치 규칙': 'section-2', '구현과 확인 순서': 'section-3', '조사 자료': 'section-4'})
    nav=BeautifulSoup('<nav aria-label="본문 목차"><ol></ol></nav>','html.parser')
    for i,h in enumerate(content.select('h2')):
        h['id']=readme_ids.get(h.get_text(), ids[i] if i<len(ids) and ids[i] else f'section-{i}')
        li=nav.new_tag('li');a=nav.new_tag('a',href='#'+h['id']);a.string=h.get_text();li.append(a);nav.ol.append(li)
    tables(content)
    page=BeautifulSoup(template,'html.parser');main=page.main;main.clear()
    links=BeautifulSoup('<div class="top"><a href="index.html#reports">← 전체 보고서</a> · <a href="'+download+'">Markdown 원고</a></div>','html.parser')
    main.append(links)
    for child in list(content.contents):
        if getattr(child,'name',None)=='h2' and nav.parent is None: main.append(nav)
        main.append(child)
    footer=page.new_tag('footer');footer.string='HeavyMach';main.append(footer)
    page.title.string=title+' | HeavyMach'
    # Keep long hashes, code, and tables within narrow mobile screens.
    style=page.new_tag('style');style.string='main{min-width:0}code,a{overflow-wrap:anywhere}pre{white-space:pre-wrap}nav{display:block}nav ol{padding-left:0;list-style:none;columns:2}nav li{break-inside:avoid}@media(max-width:700px){table thead{display:none}table tbody tr:first-child{display:block}nav ol{columns:1}}';page.head.append(style)
    deduplicate_styles(page)
    dest.write_text(str(page));shutil.copyfile(source,dest.parent/download)

def main():
    check_runtime_art_references()
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--checkout',type=Path,required=True);args=ap.parse_args();dist=args.checkout/'dist'
    original=(dist/'index.html').read_text();old=BeautifulSoup(original,'html.parser');images={}
    for sid in ('menu-joins','menu-result'):
        gallery=BeautifulSoup('<div class="grid"></div>','html.parser')
        for img in old.select(f'#{sid} img'):
            fig=gallery.new_tag('figure');a=gallery.new_tag('a',href=img['src']);a.append(img.__copy__());fig.append(a)
            cap=gallery.new_tag('figcaption');cap.string=img.get('alt','');fig.append(cap);gallery.div.append(fig)
        images[sid]=str(gallery)
    raw=(ROOT/'docs/public/report-overview-20261025.md').read_text()
    for sid,gallery in images.items():raw=raw.replace('{{'+sid+'-gallery}}',gallery)
    content=render(raw);tables(content)
    # Wrap each h2 and its following content into stable existing anchor sections.
    main=old.main;main.clear();current=main
    for child in list(content.contents):
        if getattr(child,'name',None)=='h2':
            sid=child.get('id');child.attrs.pop('id',None);current=old.new_tag('section',id=sid);main.append(current)
        current.append(child)
    nav=old.select_one('aside nav');nav.clear()
    for section in main.select('section'):
        a=old.new_tag('a',href='#'+section['id']);a.string={'reports':'전체 보고서', 'apple-platforms':'Apple·키보드·투두', 'clone-build':'새 clone 빌드', 'independence':'IPA 없는 빌드 검증', 'publication':'집필 자료·설치 안내', 'developer-story':'개발자의 변', 'ios-audit':'iOS 코드·문서 검사', 'android-plan':'타이틀·Android', 'migration-audit':'전수검사·REA', 'repositories':'저장소·버전','port-and-native':'포팅·복원 과정','retrospective':'기술 회고','rea':'REA 활용','documentation':'문서 체계','menu-joins':'배경 연결 수정','menu-result':'메뉴 확장','phone':'아이폰 검사','ai':'적 이동','rotation':'가로 회전','screens':'남은 화면','evidence':'남은 작업'}[section['id']];nav.append(a)
    old.select_one('.sidefoot').string='2026.10.27 · 전체 보고서'
    old.title.string='HeavyMach 개발 보고'
    old.select_one('meta[name="description"]')['content']='HeavyMach 개발 과정, 기능·버그 수정, 아이폰 검증과 남은 작업'
    style=old.new_tag('style');style.string='figure{margin:0;min-width:0}figure img{width:100%;height:auto;display:block}figcaption{font-size:12px;color:var(--muted);margin-top:6px}aside{overflow:auto}nav{margin:24px 0}nav a{line-height:1.4}code,a{overflow-wrap:anywhere}@media(max-width:540px){table thead{display:none}table tr{display:block;border-top:1px solid var(--line);padding:10px 0}table td{display:block;border:0;padding:5px 0}table td:before{content:attr(data-label);display:block;font-size:11px;color:var(--muted)}}';old.head.append(style)
    deduplicate_styles(old)
    (dist/'index.html').write_text(str(old))
    retro=(dist/'retrospective.html').read_text();doc=(dist/'documentation.html').read_text()
    article(ROOT/'docs/public/engineering-retrospective-20261025.md',dist/'retrospective.html','engineering-retrospective.md',retro)
    article(ROOT/'docs/documentation-audit-20261025.md',dist/'documentation.html','documentation-audit-20261025.md',doc)
    article(ROOT/'docs/public/rea-restoration-method-20261025.md',dist/'rea.html','rea-restoration-method-20261025.md',retro)
    article(ROOT/'docs/public/ipa-port-and-native-app-20261026.md',dist/'port-and-native.html','ipa-port-and-native-app-20261026.md',retro)
    article(ROOT/'docs/related-titles-20261026.md',dist/'related-titles.html','related-titles-20261026.md',retro)
    article(ROOT/'docs/android-port-plan-20261026.md',dist/'android-plan.html','android-port-plan-20261026.md',retro)
    article(ROOT/'docs/ios-completeness-audit-20261026.md',dist/'ios-audit.html','ios-completeness-audit-20261026.md',retro)
    article(ROOT/'docs/auto-aim-plan-20261026.md',dist/'auto-aim.html','auto-aim-plan-20261026.md',retro)
    aim_page=dist/'auto-aim.html'
    aim_page.write_text(aim_page.read_text().replace('href="public/auto-aim-ipad-20261027.md"', 'href="auto-aim-progress.html"'))
    article(ROOT/'docs/RESTORATION.md',dist/'restoration.html','RESTORATION.md',retro)
    public_articles = [
        ('lux-formation-plan-20261029.md', 'lux-formation.html'),
        ('app-icon-and-media-20261029.md', 'icon-and-media.html'),
        ('hud-improvement-plan-20261028.md', 'hud-improvement.html'),
        ('auto-aim-visibility-plan-20261028.md', 'auto-aim-visibility.html'),
        ('apple-platform-progress-20261026.md', 'apple-platforms.html'),
        ('fullscreen-transitions-20261027.md', 'fullscreen.html'),
        ('auto-aim-ipad-20261027.md', 'auto-aim-progress.html'),
        ('vtol-art-and-loadout-20261027.md', 'vtol-art.html'),
        ('lux-lunae-design-20261027.md', 'lux-lunae.html'),
        ('lux-lunae-intent-20261028.md', 'lux-lunae-intent.html'),
        ('clone-build-20261026.md', 'clone-build.html'),
        ('ipa-independent-build-20261026.md', 'independence.html'),
        ('rea-implementation-audit-20261026.md', 'rea-implementation.html'),
        ('build-and-install.md', 'installation.html'),
        ('implementation-status.md', 'implementation.html'),
        ('ground-units-research-20261028.md', 'ground-units.html'),
        ('ground-aa-art-20261028.md', 'ground-aa-art.html'),
        ('publication-inventory.md', 'publication.html'),
        ('after-heavymach1-20261105.md', 'after-heavymach1.html'),
        ('gameplay-media-20261027.md', 'gameplay-media.html'),
        ('github-documentation-plan-20261027.md', 'github-documentation.html'),
        ('github-licenses-and-badges-20261028.md', 'github-licenses-and-badges.html'),
        ('github-repository-plan-20261028.md', 'github-repository-plan.html'),
        ('github-followup-20261028.md', 'github-followup.html'),
    ]
    for source_name, page_name in public_articles:
        article(ROOT/'docs/public'/source_name, dist/page_name, source_name, retro)
        page = BeautifulSoup((dist/page_name).read_text(), 'html.parser')
        if page.footer: page.footer.decompose()
        if page_name == 'after-heavymach1.html':
            style=page.new_tag('style');style.string='main img{display:block;width:min(100%,420px);height:auto;margin:24px auto}';page.head.append(style)
        if page_name in ('lux-formation.html','icon-and-media.html','hud-improvement.html','fullscreen.html','auto-aim-progress.html','lux-lunae.html','lux-lunae-intent.html','gameplay-media.html','ground-aa-art.html'):
            style=page.new_tag('style');style.string='main img,main video{display:block;max-width:100%;height:auto;margin:16px 0}main video{width:100%}';page.head.append(style)
            for img in page.select('main img'):
                link=page.new_tag('a',href=img['src']);img.wrap(link)
        if page_name == 'gameplay-media.html':
            for img in page.select('main img'):
                img['width']='2736';img['height']='1260'
            for video in page.select('video'):
                video['width']='2736';video['height']='1260'
        if page_name == 'lux-lunae-intent.html':
            style=page.new_tag('style');style.string='main img[src*="-gold.png"]{background:#525a58;border-radius:8px;padding:12px;box-sizing:border-box;width:100%}';page.head.append(style)
            link=page.select_one('a[href="media/gameplay-20261027/07-fighters.mp4"]')
            if link:
                video=page.new_tag('video',controls='',playsinline='',preload='metadata',poster='media/gameplay-20261027/07-fighters.png')
                video['aria-label']='추가 전투기들과 싸우는 13초 플레이 영상'
                video.append(page.new_tag('source',src=link['href'],type='video/mp4'))
                link.parent.insert_before(video)
        if page_name == 'lux-lunae.html':
            style=page.new_tag('style');style.string='.lux-small{display:flex;gap:24px;flex-wrap:wrap;align-items:center;padding:20px;background:#4c524f}.lux-small figure{margin:0}.lux-small img{max-width:none}.lux-small figcaption{font-size:12px;color:#fff}.lux-small a{display:block}'
            page.head.append(style)
        if page_name == 'ground-aa-art.html':
            for anchor in page.select('a#integrated'): anchor.decompose()
            for link in list(page.select('a[href^="media/ground-aa/"][href$=".mp4"]')):
                video=page.new_tag('video',controls='',playsinline='',preload='metadata',loop='')
                video['aria-label']=link.get_text()
                video.append(page.new_tag('source',src=link['href'],type='video/mp4'))
                link.parent.insert_before(video)
            style=page.new_tag('style');style.string='.gaa-art-pair{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:20px;background:#626e70;padding:20px}.gaa-art-pair figure{margin:0;min-width:0}.gaa-art-pair img{width:100%;height:auto}.gaa-art-pair figcaption{font-size:14px;color:#fff}.gaa-reference-strip{display:flex;flex-wrap:wrap;align-items:center;gap:28px;padding:24px;background:#626e70}.gaa-reference-strip figure{margin:0}.gaa-reference-strip img{max-width:none;width:auto;margin:0}.gaa-reference-strip img[width="200"]{width:200px}.gaa-reference-strip img[width="99"]{width:99px}.gaa-reference-strip img[width="126"]{width:126px}.gaa-reference-strip img[width="92"]{width:92px}.gaa-reference-strip img[width="90"]{width:90px}.gaa-reference-strip img[width="86"]{width:86px}.gaa-reference-strip img[width="114"]{width:114px}.gaa-reference-strip figcaption{font-size:13px;color:#fff;margin-top:8px}'
            page.head.append(style)
        for link in page.select('a[href]'):
            if link['href'].startswith('../references/'):
                link['href']='https://github.com/gelb2/HeavyMach/blob/main/docs/'+link['href'][3:]
            if link['href'].startswith('../evidence/') and page_name not in ('hud-improvement.html', 'installation.html', 'implementation.html', 'ground-units.html', 'ground-aa-art.html', 'publication.html', 'gameplay-media.html', 'github-documentation.html', 'github-licenses-and-badges.html', 'github-followup.html'):
                link['href']=link['href'][3:]
            if link['href'] in ('../apple-platforms.md','../keyboard-input-policy.md','../current-status.md','../auto-aim-plan-20261026.md','../wireless-device-workflow.md'):
                link['href']=link['href'].removeprefix('../')
        (dist/page_name).write_text(str(page))
    reader_sources = {
        'README.md': 'github.html', 'docs/README.md': 'docs-index.html',
        'CHANGELOG.md': 'changelog.html', 'CREDITS.md': 'credits.html',
        'LICENSE-SCOPE.md': 'license-scope.html',
        'ASSET-LICENSES.md': 'asset-licenses.html',
        'THIRD_PARTY_NOTICES.md': 'third-party-notices.html',
        'docs/public/features.md': 'features.html',
        'docs/public/gameplay-guide.md': 'gameplay-guide.html',
        'docs/public/architecture.md': 'architecture.html',
        'docs/public/troubleshooting.md': 'troubleshooting.html',
        'docs/public/development-history.md': 'development-history.html',
        'docs/public/validation.md': 'validation.html',
        'docs/ground-aa-expansion.md': 'ground-aa-implementation.html',
    }
    routes = {**reader_sources, **{'docs/public/'+src: page for src, page in public_articles},
              'docs/RESTORATION.md': 'restoration.html', 'docs/auto-aim-plan-20261026.md': 'auto-aim.html',
              'docs/android-port-plan-20261026.md': 'android-plan.html'}
    for source_name, page_name in reader_sources.items():
        article(ROOT/source_name, dist/page_name, page_name.replace('.html', '.md'), retro)
    # Resolve relative Markdown paths from each manuscript's original directory.
    from urllib.parse import urlsplit, unquote, quote
    for source_name, page_name in routes.items():
        if page_name not in list(reader_sources.values()) + ['icon-and-media.html', 'hud-improvement.html', 'auto-aim-visibility.html', 'restoration.html', 'installation.html', 'implementation.html', 'ground-units.html', 'ground-aa-art.html', 'publication.html', 'gameplay-media.html', 'github-documentation.html', 'github-licenses-and-badges.html', 'github-repository-plan.html', 'github-followup.html']:
            continue
        page = BeautifulSoup((dist/page_name).read_text(), 'html.parser')
        if page.footer: page.footer.decompose()
        for node in page.select('main a[href], main img[src]'):
            attr = 'src' if node.name == 'img' else 'href'
            raw = node[attr]
            if node.find_parent(class_='top') or node.find_parent('nav'): continue
            url = urlsplit(raw)
            if url.scheme or url.netloc or not url.path: continue
            target = (ROOT/source_name).parent / unquote(url.path)
            try: rel = target.resolve().relative_to(ROOT).as_posix()
            except ValueError: continue
            if rel in routes: node[attr] = routes[rel]
            elif rel.startswith('Art/AppIcon/20261029/'):
                node[attr] = 'app-icon-source/'+rel.removeprefix('Art/AppIcon/20261029/')
            elif rel.startswith('Art/GroundAA/20261028-refinement/') and rel.endswith('.json'):
                node[attr] = 'evidence/ground-aa-refinement-'+Path(rel).name
            elif rel.startswith('Art/GroundAA/20261028-texture-proposal/') and rel.endswith('.json'):
                node[attr] = 'evidence/ground-aa-texture-'+Path(rel).name
            elif rel.startswith('Art/GroundAA/20261028-aircraft-style/') and rel.endswith('.json'):
                node[attr] = 'evidence/ground-aa-aircraft-style-'+Path(rel).name
            elif rel.startswith('Art/GroundAA/20261028-runtime-style/') and rel.endswith('.json'):
                node[attr] = 'evidence/ground-aa-runtime-style-'+Path(rel).name
            elif rel.startswith(('docs/evidence/lux-formation-20261029.', 'docs/evidence/lux-xy-20261029.', 'docs/evidence/lux-vertical-20261029.', 'docs/evidence/original-air-spawns-20261029.')):
                node[attr] = 'evidence/'+Path(rel).name
            elif rel == 'docs/evidence/hud-editor-release-20261029.json':
                node[attr] = 'evidence/hud-editor-release-20261029.json'
            elif rel == 'docs/evidence/hud-editor-20261029.json':
                node[attr] = 'evidence/hud-editor-20261029.json'
            elif rel == 'docs/evidence/independence-main-20261028.json':
                node[attr] = 'evidence/independence-main-20261028.json'
            elif rel.startswith('Art/GroundAA/20261028-shilka-turret/') and rel.endswith('.json'):
                node[attr] = 'evidence/shilka-turret-'+Path(rel).name
            elif rel == 'docs/evidence/shilka-height-20261028.json':
                node[attr] = 'evidence/shilka-height-20261028.json'
            elif rel == 'docs/evidence/shilka-turret-20261028.json':
                node[attr] = 'evidence/shilka-turret-20261028.json'
            elif rel.startswith('Art/GroundAA/20261028-integrated/') and rel.endswith('.json'):
                node[attr] = 'evidence/ground-aa-integrated-'+Path(rel).name
            elif rel == 'docs/evidence/ground-aa-integrated-20261028.json':
                node[attr] = 'evidence/ground-aa-integrated-20261028.json'
            elif rel == 'docs/evidence/ground-aa-20261028.json':
                node[attr] = 'evidence/ground-aa-20261028.json'
            elif rel.startswith('docs/public/media/'):
                node[attr] = rel.removeprefix('docs/public/')
            else:
                node[attr] = 'https://github.com/gelb2/HeavyMach/blob/main/'+quote(rel, safe='/')
            if url.fragment:
                fragment = 'section-2' if rel == 'docs/public/app-icon-and-media-20261029.md' and unquote(url.fragment) == 'hud-위치-편집' else url.fragment
                node[attr] += '#'+fragment
        style = page.new_tag('style')
        style.string = 'main img,main video{display:block;max-width:100%;height:auto;margin:18px 0}main h1{line-height:1.25}main img[src^="media/badges/"]{display:inline-block;width:auto;height:20px;margin:0 4px 6px 0;vertical-align:middle}'
        page.head.append(style)
        if page_name == 'icon-and-media.html':
            icon_style = page.new_tag('style')
            icon_style.string = 'main table{table-layout:fixed;width:100%}main th,main td{width:50%;vertical-align:top}main td img{width:100%;max-width:360px;margin:12px auto}'
            page.head.append(icon_style)
        deduplicate_styles(page)
        (dist/page_name).write_text(str(page))
    article(ROOT/'docs/media-publication.md', dist/'media-preparation.html', 'media-publication.md', retro)
    preparation=dist/'media-preparation.html'
    preparation.write_text(preparation.read_text().replace('href="public/gameplay-media-20261027.md"', 'href="gameplay-media.html"'))
    shutil.copyfile(ROOT/'docs/media-scenes.example.json', dist/'media-scenes.example.json')
    story=BeautifulSoup((dist/'restoration.html').read_text(),'html.parser')
    if story.footer: story.footer.decompose()
    (dist/'restoration.html').write_text(str(story))
    shutil.copytree(ROOT/'docs/public/rea',dist/'rea',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/independence',dist/'independence',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/clone',dist/'clone',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/hud-audit',dist/'media/hud-audit',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/hud-editor',dist/'media/hud-editor',dirs_exist_ok=True)
    shutil.copytree(ROOT/'Art/AppIcon/20261029',dist/'app-icon-source',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/app-icon-20261029',dist/'media/app-icon-20261029',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/updates-20261029',dist/'media/updates-20261029',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/ground-aa',dist/'media/ground-aa',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/vtol',dist/'media/vtol',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/fullscreen',dist/'media/fullscreen',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/auto-aim',dist/'media/auto-aim',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/lux-lunae',dist/'media/lux-lunae',dirs_exist_ok=True)
    shutil.copyfile(ROOT/'docs/public/media/README.md', dist/'media/README.md')
    shutil.copytree(ROOT/'docs/public/media/gameplay-20261027',dist/'media/gameplay-20261027',dirs_exist_ok=True)
    shutil.copytree(ROOT/'docs/public/media/badges',dist/'media/badges',dirs_exist_ok=True)
    for name in ('github-social-preview.svg', 'github-social-preview.png', 'github-social-preview-gameplay.svg', 'github-social-preview-gameplay.png'):
        shutil.copyfile(ROOT/'docs/public/media'/name,dist/'media'/name)
    shutil.copyfile(ROOT/'docs/lux-lunae-plan-20261027.md',dist/'lux-lunae-plan-20261027.md')
    (dist/'evidence').mkdir(exist_ok=True)
    for pattern in ('lux-xy-20261029.*', 'lux-vertical-20261029.*', 'original-air-spawns-20261029.*'):
        for file in (ROOT/'docs/evidence').glob(pattern):
            shutil.copyfile(file,dist/'evidence'/file.name)
    for suffix in ('cpp','csv','json'):
        name='lux-formation-20261029.'+suffix
        shutil.copyfile(ROOT/'docs/evidence'/name,dist/'evidence'/name)
    shutil.copyfile(ROOT/'docs/evidence/hud-editor-20261029.json',dist/'evidence/hud-editor-20261029.json')
    shutil.copyfile(ROOT/'docs/evidence/hud-editor-release-20261029.json',dist/'evidence/hud-editor-release-20261029.json')
    for file in (ROOT/'Art/GroundAA/20261028-refinement').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('ground-aa-refinement-'+file.name))
    for file in (ROOT/'Art/GroundAA/20261028-texture-proposal').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('ground-aa-texture-'+file.name))
    for file in (ROOT/'Art/GroundAA/20261028-aircraft-style').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('ground-aa-aircraft-style-'+file.name))
    for file in (ROOT/'Art/GroundAA/20261028-runtime-style').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('ground-aa-runtime-style-'+file.name))
    shutil.copyfile(ROOT/'docs/evidence/independence-main-20261028.json',dist/'evidence/independence-main-20261028.json')
    for file in (ROOT/'Art/GroundAA/20261028-shilka-turret').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('shilka-turret-'+file.name))
    shutil.copyfile(ROOT/'docs/evidence/shilka-height-20261028.json',dist/'evidence/shilka-height-20261028.json')
    shutil.copyfile(ROOT/'docs/evidence/shilka-turret-20261028.json',dist/'evidence/shilka-turret-20261028.json')
    for file in (ROOT/'Art/GroundAA/20261028-integrated').glob('*.json'):
        shutil.copyfile(file,dist/'evidence'/('ground-aa-integrated-'+file.name))
    shutil.copyfile(ROOT/'docs/evidence/ground-aa-integrated-20261028.json',dist/'evidence/ground-aa-integrated-20261028.json')
    shutil.copyfile(ROOT/'docs/evidence/ground-aa-20261028.json',dist/'evidence/ground-aa-20261028.json')
    shutil.copyfile(ROOT/'docs/evidence/lux-lunae-implementation-20261027.json',dist/'evidence/lux-lunae-implementation-20261027.json')
    for suffix in ['txt','cpp']:
        name='lux-lunae-inventory-20261027.'+suffix
        shutil.copyfile(ROOT/'docs/evidence'/name,dist/'evidence'/name)
    for name in ['apple-platform-performance-20261026.json', 'mac-fullscreen-capture-20261027.json', 'physical-apple-20261027.json', 'wireless-apple-20261027.json', 'vtol-feasibility-20261027.json', 'vtol-art-plan-20261027.json', 'vtol-helicopter-density-20261027.json', 'vtol-implementation-20261027.json', 'vtol-refinement-20261027.json', 'fullscreen-transitions-20261027.json', 'auto-aim-ipad-20261027.json']:
        shutil.copyfile(ROOT/'docs/evidence'/name,dist/'evidence'/name)
    for name in ['game-design.md','document-governance.md','document-map.json','apple-platforms.md','keyboard-input-policy.md','current-status.md','wireless-device-workflow.md','usability-todo.md']:
        shutil.copyfile(ROOT/'docs'/name,dist/name)
    report={'pages':['index.html','retrospective.html','documentation.html','rea.html','port-and-native.html','related-titles.html','android-plan.html','ios-audit.html','restoration.html'],'manuscripts':['report-overview-20261025.md','engineering-retrospective-20261025.md','rea-restoration-method-20261025.md','ipa-port-and-native-app-20261026.md','related-titles-20261026.md','android-port-plan-20261026.md','ios-completeness-audit-20261026.md','RESTORATION.md'],'image_count_before':len(BeautifulSoup(original,'html.parser').select('img')),'image_count_after':len(old.select('img'))}
    report['pages'] += [page for _, page in public_articles] + list(reader_sources.values()) + ['media-preparation.html']
    report['manuscripts'] += [source for source, _ in public_articles] + [page.replace('.html', '.md') for page in reader_sources.values()] + ['media-publication.md']
    assert report['image_count_before']==report['image_count_after']
    catalog=BeautifulSoup((dist/'index.html').read_text(),'html.parser')
    listed={a['href'].split('#')[0] for a in catalog.select('#reports a[href]')}
    report_pages={p.name for p in dist.glob('*.html')} - {'index.html'}
    assert report_pages <= listed, f'Reports missing from main catalog: {report_pages-listed}'
    for name in report_pages:
        page=BeautifulSoup((dist/name).read_text(),'html.parser')
        assert page.select_one('a[href="index.html#reports"]'), f'Missing catalog backlink: {name}'
        anchors = [node['id'] for node in page.select('[id]')]
        assert len(anchors) == len(set(anchors)), f'Duplicate anchors: {name}'
    report['catalog_report_count']=len(report_pages)
    print(json.dumps(report,ensure_ascii=False))
if __name__=='__main__':main()
