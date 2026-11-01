#!/usr/bin/env python3
"""Pack the reference-matched static lettering into the game's texture atlas.

Requires Pillow only for asset preparation; app builds use the committed PNG.
Original menu images and original button implementation are never modified.
"""
from pathlib import Path
import hashlib,json
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1]
source=ROOT/'Art/UI/20261102-original-button-lettering/glyphs-generated.png'
im=Image.open(source).convert('RGBA')
rows=[(63,225),(266,432),(478,655),(688,851),(888,1049),(1078,1237)]
cols=[(64,222),(272,407),(462,598),(643,786),(827,1001),(1045,1188)]
chars='ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
cellw,cellh=32,24
atlas=Image.new('RGBA',(512,144));widths={};records={}
def place(c,glyph):
 box=glyph.getchannel('A').point(lambda a:255 if a>128 else 0).getbbox()
 if not box:raise ValueError('Empty glyph '+c)
 glyph=glyph.crop(box)
 # Keep proportional letter shapes. Q descender is retained inside the cell.
 h=18;w=max(2,min(28,round(glyph.width*h/glyph.height)))
 glyph=glyph.resize((w,h),Image.Resampling.LANCZOS)
 i=ord(c)-32;atlas.alpha_composite(glyph,(i%16*cellw+1,i//16*cellh+1));widths[c]=w+2
for i,c in enumerate(chars):
 x0,x1=cols[i%6];y0,y1=rows[i//6];g=im.crop((x0,y0,x1,y1));place(c,g);records[c]=[x0,y0,x1,y1]
# Plain UI punctuation uses straight sans-serif strokes at the same cap height.
# These marks accompany labels and numeric values; no system font is rasterized.
patterns={
 '?':['01110','10001','00001','00010','00100','00000','00100'],
 '!':['00100','00100','00100','00100','00100','00000','00100'],
 '|':['00100']*7,'-':['00000']*3+['11111']+['00000']*3,
 '+':['00100']*3+['11111']+['00100']*3,
 '.':['00000']*6+['00100'],',':['00000']*5+['00100','01000'],
 ':':['00000','00100','00000','00000','00100','00000','00000'],
 ';':['00000','00100','00000','00000','00100','01000','00000'],
 '/':['00001','00001','00010','00100','01000','10000','10000'],
 '\\':['10000','10000','01000','00100','00010','00001','00001'],
 '>':['10000','01000','00100','00010','00100','01000','10000'],
 '<':['00001','00010','00100','01000','00100','00010','00001'],
 '[':['01110','01000','01000','01000','01000','01000','01110'],
 ']':['01110','00010','00010','00010','00010','00010','01110'],
 '(' :['00010','00100','01000','01000','01000','00100','00010'],
 ')' :['01000','00100','00010','00010','00010','00100','01000'],
 '=':['00000','00000','11111','00000','11111','00000','00000'],
 '_':['00000']*6+['11111'],'#':['01010','11111','01010','01010','11111','01010','00000'],
 '%':['11001','11010','00010','00100','01000','01011','10011'],
 '"':['01010','01010']+['00000']*5,"'":['00100','00100']+['00000']*5,
 '&':['01100','10010','10100','01000','10101','10010','01101'],
 '@':['01110','10001','10111','10101','10111','10000','01110'],
 '*':['00000','10101','01110','11111','01110','10101','00000'],
 '^':['00100','01010','10001']+['00000']*4,
 '~':['00000','00000','01001','10110']+['00000']*3,
 '$':['00100','01111','10100','01110','00101','11110','00100'],
 '{':['00011','00100','00100','01000','00100','00100','00011'],
 '}':['11000','00100','00100','00010','00100','00100','11000'],
 '`':['01000','00100']+['00000']*5,
}
for c,pattern in patterns.items():
 # Keep punctuation on its proper baseline instead of stretching a dot to cap height.
 g=Image.new('RGBA',(24,24));d=ImageDraw.Draw(g)
 for y,row in enumerate(pattern):
  for x,on in enumerate(row):
   if on=='1':d.rectangle((2+x*3,1+y*3,4+x*3,3+y*3),fill=(234,188,65,255))
 i=ord(c)-32;atlas.alpha_composite(g,(i%16*cellw,i//16*cellh));widths[c]=18
out=ROOT/'Art/Extensions/GameMenuGlyphs.png';atlas.save(out)
metrics=[widths.get(chr(i+32),8) for i in range(96)]
(ROOT/'Sources/iOS/HMGameGlyphMetrics.hpp').write_text('#pragma once\n// Generated static sprite metrics; no runtime font rendering.\nnamespace heavymach::gameui {\ninline constexpr int glyphWidths[96]={'+','.join(map(str,metrics))+'};\n}\n')
record={'source':source.relative_to(ROOT).as_posix(),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'output':out.relative_to(ROOT).as_posix(),'sha256':hashlib.sha256(out.read_bytes()).hexdigest(),'size':[512,144],'glyph_crops':records,'method':'Reference-matched imagegen lettering, transparent crop and proportional downsampling; original buttons unchanged.'}
(ROOT/'docs/evidence/game-menu-glyphs-20261102.json').write_text(json.dumps(record,indent=2)+'\n')
p=ROOT/'docs/build-resources.json';data=json.loads(p.read_text())
for f in data['files']:
 if f['path']==out.relative_to(ROOT).as_posix():f.update(bytes=out.stat().st_size,sha256=record['sha256'])
p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n')
print('Packed reference-matched 36 letters/digits and',len(patterns),'punctuation sprites')
