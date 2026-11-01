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
  const missing=(current,previous)=>previous.records.some(r=>r.path!=='/persist'&&!current.records.some(c=>c.path===r.path))||KEYS.some(k=>previous.settings[k]!==null&&current.settings[k]===null);
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
  async function restoreSelected(snapshot){
    // Only a native-validated, durable restore intent reaches this method.
    const encoded=JSON.stringify(snapshot);
    localStorage.setItem(JOURNAL,encoded);
    const db=await openDB();
    try{
      const tx=db.transaction('FILE_DATA','readwrite'),done=transactionDone(tx),store=tx.objectStore('FILE_DATA');
      const keys=await request(store.getAllKeys());
      if(keys.some(k=>!FILES.includes(k))){tx.abort();await done;}
      store.clear();
      for(const r of snapshot.records){const value={timestamp:new Date(r.timestamp),mode:r.mode};if(r.contents!==null)value.contents=new Uint8Array(r.contents);store.put(value,r.path);}
      await done;
      for(const k of KEYS){const value=snapshot.settings[k];if(value===null)localStorage.removeItem(k);else localStorage.setItem(k,value);}
      const actual=await read();const sorted=s=>JSON.stringify({...s,records:[...s.records].sort((a,b)=>a.path.localeCompare(b.path)),settings:Object.fromEntries(KEYS.map(k=>[k,s.settings[k]]))});
      if(sorted(actual)!==sorted(snapshot))throw Error('Restore verification failed.');
      // Native finish commits the OS backup before clearing this journal.
      return actual;
    }finally{db.close();}
  }
  function finishSelected(){localStorage.removeItem(JOURNAL);}
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
  window.__hmDesktopStorage={read,empty,missing,restoreMissing,restoreSelected,finishSelected,pendingRestore,committed};
})();
