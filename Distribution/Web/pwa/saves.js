// Reads the existing game's IDBFS database. Only the launcher imports files.
const KEYS=['HeavyMach.KeyboardBindingsV1','HeavyMach.AimModeV1'];
const JOURNAL='HeavyMach.WebImportV1',PREVIOUS='HeavyMach.WebBeforeImportV1';
const req=r=>new Promise((resolve,reject)=>{r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});
const done=tx=>new Promise((r,j)=>{tx.oncomplete=r;tx.onabort=()=>j(tx.error||Error('Save transaction aborted'));tx.onerror=()=>{};});
async function open(){
 const r=indexedDB.open('/persist',21);
 r.onupgradeneeded=e=>{if(e.oldVersion!==0){r.transaction.abort();return;}r.result.createObjectStore('FILE_DATA').createIndex('timestamp','timestamp',{unique:false});};
 return req(r);
}
export function validate(s){
 if(!s||s.format!==1||!Array.isArray(s.records)||s.records.length>3||!s.settings||Object.keys(s.settings).sort().join()!==[...KEYS].sort().join())throw Error('Unsupported save file');
 const paths=new Set();
 for(const r of s.records){
  if(paths.has(r.path)||!Number.isSafeInteger(r.timestamp)||r.timestamp<0||!Number.isInteger(r.mode))throw Error('Invalid save record');paths.add(r.path);
  if(r.path==='/persist'){if((r.mode&0o170000)!==0o040000||r.contents!==null)throw Error('Invalid save directory');continue;}
  if((r.mode&0o170000)!==0o100000||!Array.isArray(r.contents)||r.contents.some(n=>!Number.isInteger(n)||n<0||n>255))throw Error('Invalid saved bytes');
  if(r.path==='/persist/HvM.dat'){if(r.contents.length!==3624)throw Error('Invalid game save size');}
  else if(r.path==='/persist/HUDLayoutV1-web.json'){
   if(r.contents.length>32768)throw Error('HUD save too large');
   const v=JSON.parse(new TextDecoder('utf-8',{fatal:true}).decode(new Uint8Array(r.contents)));
   if(v.version!==1||!Array.isArray(v.positions)||v.positions.length>10||v.positions.some(p=>p!==null&&(!Array.isArray(p)||p.length!==2||p.some(n=>!Number.isFinite(n)||n<0||n>1))))throw Error('Invalid HUD layout');
  }else throw Error('Unknown saved file');
 }
 const aim=s.settings[KEYS[1]],binding=s.settings[KEYS[0]];
 if(aim!==null&&!['0','1','2'].includes(aim))throw Error('Invalid aim setting');
 if(binding!==null){if(typeof binding!=='string'||!/^\d+(,\d+){14,15}$/.test(binding))throw Error('Invalid keyboard bindings');const codes=binding.split(',').map(Number);if(new Set(codes).size!==codes.length||codes.some(n=>n<=0||n>=256||n===227||n===231))throw Error('Invalid keyboard bindings');}
 return s;
}
export async function read(){
 const db=await open();try{
  const tx=db.transaction('FILE_DATA','readonly'),finished=done(tx),store=tx.objectStore('FILE_DATA');
  const [keys,values]=await Promise.all([req(store.getAllKeys()),req(store.getAll())]);await finished;
  return validate({format:1,records:keys.map((path,i)=>{const v=values[i];return {path,timestamp:v.timestamp.getTime(),mode:v.mode,contents:v.contents?Array.from(new Uint8Array(v.contents.buffer,v.contents.byteOffset,v.contents.byteLength)):null};}),settings:Object.fromEntries(KEYS.map(k=>[k,localStorage.getItem(k)]))});
 }finally{db.close();}
}
const digest=async text=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',new TextEncoder().encode(text))),n=>n.toString(16).padStart(2,'0')).join('');
export async function encode(snapshot){validate(snapshot);return JSON.stringify({kind:'HeavyMachWebSave',version:1,digest:await digest(JSON.stringify(snapshot)),snapshot});}
export async function decode(text){
 if(typeof text!=='string'||text.length>262144)throw Error('Save file is too large');
 const e=JSON.parse(text);if(e.kind!=='HeavyMachWebSave'||e.version!==1||await digest(JSON.stringify(e.snapshot))!==e.digest)throw Error('Save checksum does not match');return validate(e.snapshot);
}
async function apply(s){
 validate(s);const db=await open();try{
  const tx=db.transaction('FILE_DATA','readwrite'),finished=done(tx),store=tx.objectStore('FILE_DATA');
  store.clear();for(const r of s.records){const value={timestamp:new Date(r.timestamp),mode:r.mode};if(r.contents!==null)value.contents=new Uint8Array(r.contents);store.put(value,r.path);}await finished;
  for(const k of KEYS){if(s.settings[k]===null)localStorage.removeItem(k);else localStorage.setItem(k,s.settings[k]);}
 }finally{db.close();}
}
export async function recover(){const journal=localStorage.getItem(JOURNAL);if(journal){await apply(await decode(journal));localStorage.removeItem(JOURNAL);}}
export async function restore(text){
 const snapshot=await decode(text); // Validate completely before writing any storage.
 localStorage.setItem(PREVIOUS,await encode(await read()));
 localStorage.setItem(JOURNAL,await encode(snapshot));
 await recover();
}
export function download(text,name='HeavyMach-save.json'){
 const url=URL.createObjectURL(new Blob([text],{type:'application/json'})),a=document.createElement('a');a.href=url;a.download=name;a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
}
export const previous=()=>localStorage.getItem(PREVIOUS);
