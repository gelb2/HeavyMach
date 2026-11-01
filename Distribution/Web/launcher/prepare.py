"""Build launcher-only sprite derivatives from unchanged runtime artwork."""
from pathlib import Path
import sys,json,hashlib,struct,zlib,shutil
ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'tools'))
from resource_codecs import pngdecode

def png(w,h,pixels):
 def chunk(t,b):return struct.pack('>I',len(b))+t+b+struct.pack('>I',zlib.crc32(t+b)&0xffffffff)
 raw=b''.join(b'\0'+pixels[y*w*4:(y+1)*w*4] for y in range(h))
 return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',w,h,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')
def prepare(out):
 out=Path(out);out.mkdir(parents=True,exist_ok=True);manifest=[]
 def source(name):
  p=ROOT/'Resources/Images'/name;m,b=pngdecode(p.read_bytes());b=bytearray(b)
  if m['cgbi']:
   for i in range(0,len(b),4):
    a=b[i+3]
    if a:
     for c in range(3):b[i+c]=min(255,(b[i+c]*255+a//2)//a)
  manifest.append({'source':str(p.relative_to(ROOT)),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
  return m['width'],m['height'],b
 def crop(img,x,y,w,h):
  sw,sh,b=img;assert 0<=x<x+w<=sw and 0<=y<y+h<=sh
  return b''.join(b[((y+r)*sw+x)*4:((y+r)*sw+x+w)*4] for r in range(h))
 for name,dest in [('Main_BG.png','background.png'),('Main_Title.png','title.png')]:
  w,h,b=source(name);(out/dest).write_bytes(png(w,h,b))
 # Existing metal frame, including the lower pressed state. Nine-slice keeps screws intact.
 img=source('Common_Btn_OK.png')
 for name,y in [('button.png',0),('button-down.png',39)]:
  b=bytearray(crop(img,0,y,62,39))
  # Reuse an empty strip inside the frame as a clean label field; no new painting.
  for row in range(5,34):
   for col in range(8,54):b[(row*62+col)*4:(row*62+col+1)*4]=crop(img,7,y+row,1,1)
  (out/name).write_bytes(png(62,39,b))
 (out/'source-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 shutil.copy2(Path(__file__).with_name('skin.css'),out/'skin.css')
if __name__=='__main__':prepare(sys.argv[1])
