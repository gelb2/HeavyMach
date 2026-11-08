#!/usr/bin/env python3
"""Build a derived 1.13-format runtime pack. Original payloads stay byte-identical.
Reviewed authoring data is serialized offline, never synthesized at app startup.
UnitExtensions.hmx is an explicit versioned extension, not an MCH record.
"""
import argparse, copy, hashlib, json, math, pathlib, struct
ROOT=pathlib.Path(__file__).resolve().parents[1]
def f32(x): return struct.unpack('<f',struct.pack('<f',x))[0]
def text(s,n):
    b=s.encode('ascii'); assert len(b)<n,(s,n)
    return b+b'\0'*(n-len(b))
def node(kind,payload=b'',children=(),size=None):
    body=payload+struct.pack('<I',len(children))+b''.join(children)
    return struct.pack('<III',size if size is not None else len(body)+12,kind,len(payload))+body
def read_node(b,at=0):
    size,kind,n=struct.unpack_from('<III',b,at); payload=b[at+12:at+12+n]; pos=at+12+n
    count,=struct.unpack_from('<I',b,pos);pos+=4; children=[]
    for _ in range(count):
        child,pos=read_node(b,pos);children.append(child)
    return {'size':size,'kind':kind,'payload':payload,'children':children,'raw':b[at:pos]},pos
def append_children(original,children):
    root,end=read_node(original); assert end==len(original)
    return node(root['kind'],root['payload'],[c['raw'] for c in root['children']]+children,root['size'])
def part_node(p): return node(4,text(p[0],24)+bytes(p[1:3]),[part_node(c) for c in p[3]])
def part_bytes(p): return node(1,text(p[0],24)+bytes([p[1],p[2],len(p[3])])+b''.join(struct.pack('<ff',*v) for v in p[3]))
def weapon_bytes(w):
    payload=text(w[0],24)+struct.pack('<hhBBffffffhfBB',*w[1:15])+text(w[15],24)+struct.pack('<h',w[16])+text(w[17],16)+text(w[18],24)+struct.pack('<hh',*w[19:21])+text(w[21],24)
    assert len(payload)==156
    return node(1,payload)
def string(s):
    b=s.encode('utf-8');assert len(b)<65536;return struct.pack('<H',len(b))+b
def floats(*xs): return struct.pack('<'+'f'*len(xs),*xs)
def verify_original_nodes(before,after,is_map):
    a,end=read_node(before);b,end2=read_node(after)
    assert end==len(before) and end2==len(after)
    assert (a['kind'],a['size'],a['payload'])==(b['kind'],b['size'],b['payload'])
    assert len(b['children'])>=len(a['children'])
    for old,new in zip(a['children'],b['children']):
        if is_map and old['kind']==10:
            assert (old['kind'],old['size'],old['payload'])==(new['kind'],new['size'],new['payload'])
            assert len(new['children'])>=len(old['children'])
            assert all(x['raw']==y['raw'] for x,y in zip(old['children'],new['children']))
        else: assert old['raw']==new['raw'], 'Original record bytes changed'

def build(root=ROOT,output=None):
    a=json.loads((root/'tools/prepare_unit_data_authoring.json').read_text()); output=output or root/'Resources/RuntimeData';output.mkdir(parents=True,exist_ok=True)
    original=root/'Resources/RawData'; parts=copy.deepcopy(a['parts']);machines=copy.deepcopy(a['machines']); presentations=[];behaviors=[]
    bypart={p[0]:p for p in parts};bymachine={m[0]:m for m in machines}
    # Aircraft body and exhaust are real nodes traversed by the common part renderer.
    for name,tile,width,aspect,bounds,muzzle,nozzle,aft in a['aircraft']:
        old=name+'_Anchor';body=name+'_Body';p=bypart[old];p[0]=body;p[2]=1
        rootpart=bymachine[name][4][0];rootpart[0]=body;rootpart[2]=1;rootpart[3][0][2]=2
        effect=name+'_Exhaust';parts.append([effect,1,1,[[0,0]]]);rootpart[3].append([effect,0,0,[]])
        presentations.append((body,[width,f32(f32(width)*f32(aspect))],0,0,0,1,[(tile.removeprefix('Extensions/'),[0,0])],[0,0],False))
        presentations.append((effect,[0,0],0,0,1,2,[('BulletEffect2.png',[0,0])],nozzle,bool(aft)))
        mount=a.get('aircraft_weapon_mounts',{}).get(name,'fixed_muzzle')
        if mount not in ('fixed_muzzle','part_sockets'):
            raise ValueError('Unknown aircraft weapon mounting: '+mount)
        # Visible rotating launchers use original MCH sockets. The earlier
        # aircraft preserve their audited fixed muzzle extension unchanged.
        flags=1|2|8|16|64|(32 if mount=='fixed_muzzle' else 0)
        behaviors.append((name,flags,bounds,-24,muzzle,[8,3,.5,20,12]))
    art={x[0].removeprefix('Extensions/'):x for x in a['ground_images']}
    for p in parts:
        if not p[0].startswith('GAA_'):continue
        base,kind=p[0].split('_M',1);kind=kind.split('_',1)[1]
        keys=[base+'_'+kind+'.png']
        if kind=='Body':keys.append(base+'_Body2.png')
        if kind=='Radar':keys=[base+'_Radar'+str(i)+'.png' for i in range(24)]
        frames=[(k,[-art[k][2],-art[k][3]]) for k in keys]
        presentations.append((p[0],art[keys[0]][1],int(kind=='Radar'),2 if kind=='Radar' else 0,0,1,frames,[0,0],False))
    for m in machines:
        if m[0].startswith('GAA_'):behaviors.append((m[0],1|2|4,[-54,0,43,55],58,[0,0],[0,0,.5,20,12]))
    for name,gauge in a.get('machine_gauge_offsets',{}).items():
        assert name in bymachine and math.isfinite(gauge),'Invalid gauge owner/offset'
        prior=next((i for i,b in enumerate(behaviors) if b[0]==name),None)
        if prior is None:behaviors.append((name,2,[0,0,0,0],gauge,[0,0],[0,0,.5,20,12]))
        else:
            b=list(behaviors[prior]);b[1]|=2;b[3]=gauge;behaviors[prior]=tuple(b)
    if 'part_record_order' in a:
        order=a['part_record_order'];defined={p[0]:p for p in parts}
        assert len(order)==len(set(order))==len(parts) and set(order)==set(defined),'Part record order must name every authored part exactly once'
        parts=[defined[name] for name in order]
    merged={
      'WeaponInfo.mch':append_children((original/'WeaponInfo.mch').read_bytes(),[weapon_bytes(w) for w in a['weapons']]),
      'PartsInfo.mch':append_children((original/'PartsInfo.mch').read_bytes(),[part_bytes(p) for p in parts]),
      'MachInfo.mch':append_children((original/'MachInfo.mch').read_bytes(),[node(3,text(m[0],24)+struct.pack('<iiB',*m[1:4]),[part_node(p) for p in m[4]]) for m in machines])}
    for src in sorted(original.glob('*.map')):
        raw=src.read_bytes(); additions=a['placements'].get(src.stem,[])
        if additions:
            parsed,end=read_node(raw);assert end==len(raw);groups=[];found=False
            for c in parsed['children']:
                if c['kind']==10:
                    assert not found;found=True
                    groups.append(node(10,c['payload'],[x['raw'] for x in c['children']]+[node(11,text(n,24)+floats(*xy)) for n,xy in additions],c['size']))
                else:groups.append(c['raw'])
            assert found;raw=node(parsed['kind'],parsed['payload'],groups,parsed['size'])
        merged[src.name]=raw
    ext=bytearray(b'HMEX0001'+struct.pack('<I',len(presentations)))
    for name,size,anim,period,effect,cols,frames,nozzle,aft in presentations:
        ext+=string(name)+floats(*size)+struct.pack('<BfBBI',anim,period,effect,cols,len(frames))
        for tile,offset in frames:ext+=string(tile)+floats(*offset)
        ext+=floats(*nozzle)+bytes([aft])+floats(12,20,4,45,1.2,6,.8)
    ext+=struct.pack('<I',len(behaviors))
    for name,flags,bounds,gauge,muzzle,pitch in behaviors:ext+=string(name)+struct.pack('<I',flags)+floats(*bounds,gauge,*muzzle,*pitch)
    extraweapons=[w for w in a['weapons'] if w[22:]!=[1,0,0]]
    ext+=struct.pack('<I',len(extraweapons))
    for w in extraweapons:ext+=string(w[0])+floats(w[22],w[23])+bytes([w[24]])
    ext+=struct.pack('<I',len(a['waves']))
    for key,boss,units in a['waves']:
        ext+=string(key)+string(boss)+floats(-2.5,-.5)+struct.pack('<I',len(units))
        for name,pos in units:ext+=string(name)+floats(*pos)
    for name,data in merged.items():verify_original_nodes((original/name).read_bytes(),data,name.endswith('.map'))
    merged['UnitExtensions.hmx']=bytes(ext)
    for name,data in merged.items():(output/name).write_bytes(data)
    manifest={'schema':1,'baseline':a['baseline'],'authoring_sha256':hashlib.sha256((root/'tools/prepare_unit_data_authoring.json').read_bytes()).hexdigest(),'original_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(original.iterdir()) if p.is_file()},'runtime_sha256':{k:hashlib.sha256(v).hexdigest() for k,v in sorted(merged.items())},'added':{'machines':len(machines),'parts':len(parts),'weapons':len(a['weapons']),'placements':sum(map(len,a['placements'].values())),'formations':len(a['waves'])}}
    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(json.dumps(manifest['added']))
if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--output',type=pathlib.Path);args=ap.parse_args();build(output=args.output)
