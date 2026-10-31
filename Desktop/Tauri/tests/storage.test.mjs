import test from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import fs from 'node:fs';
import { IDBFactory } from 'fake-indexeddb';
const code=fs.readFileSync(new URL('../frontend/storage.js',import.meta.url),'utf8');
function env(){
 const values=new Map();let fail=null;
 const ls={getItem:k=>values.get(k)??null,setItem:(k,v)=>{if(k===fail)throw new Error('QuotaExceededError');values.set(k,String(v));},removeItem:k=>values.delete(k)};
 const c={indexedDB:new IDBFactory(),localStorage:ls,Date,Uint8Array,console};c.window=c;vm.runInNewContext(code,c);
 return {api:c.__hmDesktopStorage,values,ls,db:c.indexedDB,setFail:k=>{fail=k;}};
}
function snapshot(n=1){return {format:1,records:[{path:'/persist/HvM.dat',timestamp:1234,mode:33206,contents:Array(3624).fill(n)}],settings:{'HeavyMach.KeyboardBindingsV1':null,'HeavyMach.AimModeV1':'1'}};}
const plain=x=>JSON.parse(JSON.stringify(x));
const idle=()=>({loaded:true,loading:false,syncing:false,dirty:false,blocked:false});
test('initial empty profile, restore exact bytes and IDBFS Date timestamps',async()=>{
 const e=env();assert.ok(e.api.empty(await e.api.read()));await e.api.restoreMissing(snapshot());assert.deepEqual(plain(await e.api.read()),snapshot());assert.equal(e.api.pendingRestore(),null);
});
test('existing progress cannot be replaced',async()=>{const e=env();await e.api.restoreMissing(snapshot(7));await assert.rejects(e.api.restoreMissing(snapshot(8)),/Existing progress/);assert.deepEqual(plain(await e.api.read()),snapshot(7));});
test('pending, blocked and dirty saves are never collected',async()=>{const e=env();for(const patch of [{loaded:false},{syncing:true},{dirty:true},{blocked:true}])assert.equal(await e.api.committed({hmStorageState:()=>({...idle(),...patch})}),null);});
test('stable committed save is read without changes',async()=>{const e=env();await e.api.restoreMissing(snapshot());const before=plain(await e.api.read());assert.deepEqual(plain(await e.api.committed({hmStorageState:idle})),before);assert.deepEqual(plain(await e.api.read()),before);});
test('save becoming dirty during collection is skipped',async()=>{const e=env();let calls=0;assert.equal(await e.api.committed({hmStorageState:()=>({...idle(),dirty:++calls>1})}),null);});
test('settings quota failure retains restore journal and resumes exact backup',async()=>{
 const e=env();e.setFail('HeavyMach.AimModeV1');await assert.rejects(e.api.restoreMissing(snapshot()),/Quota/);assert.ok(e.api.pendingRestore());e.setFail(null);await e.api.restoreMissing(snapshot());assert.deepEqual(plain(await e.api.read()),snapshot());assert.equal(e.api.pendingRestore(),null);
});
test('unrelated backup cannot overwrite an interrupted restore',async()=>{const e=env();e.setFail('HeavyMach.AimModeV1');await assert.rejects(e.api.restoreMissing(snapshot()));e.setFail(null);await assert.rejects(e.api.restoreMissing(snapshot(2)),/does not match/);});
test('journal quota failure causes no progress writes',async()=>{const e=env();e.setFail('HeavyMach.DesktopRestoreV1');await assert.rejects(e.api.restoreMissing(snapshot()));assert.ok(e.api.empty(await e.api.read()));});
test('Emscripten signed Int8Array preserves every byte in OS backup',async()=>{
 const e=env();await e.api.read();
 const db=await new Promise((r,j)=>{const q=e.db.open('/persist',21);q.onsuccess=()=>r(q.result);q.onerror=()=>j(q.error);});
 const bytes=new Int8Array(3624);bytes.set([0,127,-128,-1,37,-7,-27,106]);
 const tx=db.transaction('FILE_DATA','readwrite');tx.objectStore('FILE_DATA').put({timestamp:new Date(1234),mode:33188,contents:bytes},'/persist/HvM.dat');
 await new Promise((r,j)=>{tx.oncomplete=r;tx.onabort=j;});db.close();
 const s=await e.api.committed({hmStorageState:idle});
 assert.deepEqual(Array.from(s.records[0].contents.slice(0,8)),[0,127,128,255,37,249,229,106]);
 assert.equal(s.records[0].contents.length,3624);
});
test('explicit restore removes stale HUD, retains journal until native commit',async()=>{const e=env();const original=snapshot(7);original.records.push({path:'/persist/HUDLayoutV1-web.json',timestamp:1234,mode:33206,contents:[123,125]});await e.api.restoreMissing(original);await e.api.restoreSelected(snapshot(2));assert.deepEqual(plain(await e.api.read()),snapshot(2));assert.ok(e.api.pendingRestore());e.api.finishSelected();assert.equal(e.api.pendingRestore(),null);});
test('selected restore retries exact data after settings quota failure',async()=>{const e=env();await e.api.restoreMissing(snapshot(7));e.setFail('HeavyMach.AimModeV1');await assert.rejects(e.api.restoreSelected(snapshot(2)),/Quota/);assert.ok(e.api.pendingRestore());e.setFail(null);await e.api.restoreSelected(snapshot(2));assert.deepEqual(plain(await e.api.read()),snapshot(2));});
test('selected restore journal failure leaves previous progress unchanged',async()=>{const e=env();await e.api.restoreMissing(snapshot(7));e.setFail('HeavyMach.DesktopRestoreV1');await assert.rejects(e.api.restoreSelected(snapshot(2)),/Quota/);assert.deepEqual(plain(await e.api.read()),snapshot(7));});
