// Emscripten 6.0.13 IDBFS adapter. Original game functions are never replaced.
(() => {
  'use strict';
  const KEYS=['HeavyMach.KeyboardBindingsV1','HeavyMach.AimModeV1'];
  const FILES=['/persist','/persist/HvM.dat','/persist/HUDLayoutV1-web.json'];
  const request = r => new Promise((resolve,reject)=>{r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);});
  function openDB(){return new Promise((resolve,reject)=>{
    const r=indexedDB.open('/persist',21);
    r.onupgradeneeded=e=>{if(e.oldVersion!==0){r.transaction.abort();return;}
      const s=r.result.createObjectStore('FILE_DATA');s.createIndex('timestamp','timestamp',{unique:false});};
    r.onsuccess=()=>resolve(r.result);r.onerror=()=>reject(r.error);r.onblocked=()=>reject(new Error('Close other game windows before opening saved data.'));
  });}
  function transactionDone(tx){return new Promise((r,j)=>{tx.oncomplete=r;tx.onabort=()=>j(tx.error||new Error('Storage transaction aborted'));tx.onerror=()=>{};});}
  function settings(){return Object.fromEntries(KEYS.map(k=>[k,localStorage.getItem(k)]));}
  async function read(){
    const before=settings();const db=await openDB();
    try{
      if(!db.objectStoreNames.contains('FILE_DATA'))throw new Error('Unsupported saved database.');
      const tx=db.transaction('FILE_DATA','readonly');const done=transactionDone(tx);const s=tx.objectStore('FILE_DATA');
      const [keys,values]=await Promise.all([request(s.getAllKeys()),request(s.getAll())]);await done;
      const records=keys.map((path,i)=>{
        const v=values[i];if(!FILES.includes(path))throw new Error('Unrecognized saved file: '+path);
        if(!(v.timestamp instanceof Date)||!Number.isFinite(v.timestamp.getTime()))throw new Error('Invalid saved timestamp.');
        return {path,timestamp:v.timestamp.getTime(),mode:v.mode,contents:v.contents?Array.from(new Uint8Array(v.contents.buffer,v.contents.byteOffset,v.contents.byteLength)):null};
      });
      const after=settings();if(JSON.stringify(before)!==JSON.stringify(after))throw new Error('Settings changed while reading.');
      return {format:1,records,settings:after};
    }finally{db.close();}
  }
  const empty=s=>s.records.every(r=>r.path==='/persist')&&Object.values(s.settings).every(v=>v===null);
  const JOURNAL='HeavyMach.DesktopRestoreV1';
  const pendingRestore=()=>localStorage.getItem(JOURNAL);
  async function restoreMissing(snapshot){
    // Only the launcher calls this, before the game is loaded. Never overwrite progress.
    const journal=pendingRestore(),encoded=JSON.stringify(snapshot);
    if(journal&&journal!==encoded)throw new Error('Incomplete restore does not match the verified backup.');
    if(!journal&&!empty(await read()))throw new Error('Existing progress was found. Restore cancelled.');
    localStorage.setItem(JOURNAL,encoded);
    const db=await openDB();
    try{
      const tx=db.transaction('FILE_DATA','readwrite');const done=transactionDone(tx);const s=tx.objectStore('FILE_DATA');
      const entries=await request(s.getAllKeys());if(entries.some(k=>!FILES.includes(k))){tx.abort();await done;}
      for(const r of snapshot.records){const v={timestamp:new Date(r.timestamp),mode:r.mode};if(r.contents!==null)v.contents=new Uint8Array(r.contents);s.put(v,r.path);}
      await done;
      for(const k of KEYS){const v=snapshot.settings[k];if(v===null)localStorage.removeItem(k);else localStorage.setItem(k,v);}
      localStorage.removeItem(JOURNAL);
    }finally{db.close();}
  }
  const idle=s=>s?.loaded&&!s.loading&&!s.syncing&&!s.dirty&&!s.blocked;
  async function committed(game){
    if(!idle(game.hmStorageState?.()))return null;
    const first=await read();
    if(!idle(game.hmStorageState?.()))return null;
    // A complete save can happen between async callbacks. Read twice before accepting it.
    const second=await read();
    if(!idle(game.hmStorageState?.())||JSON.stringify(first)!==JSON.stringify(second))return null;
    return second;
  }
  window.__hmDesktopStorage={read,empty,restoreMissing,pendingRestore,committed};
})();
