import test from 'node:test';
import assert from 'node:assert/strict';
import vm from 'node:vm';
import fs from 'node:fs';
const code=fs.readFileSync(new URL('../frontend/observer.js',import.meta.url),'utf8');
function env(committed,invoke){
 const alerts=[],timers=[];
 const c={location:{protocol:'tauri:',hostname:'localhost',pathname:'/game/index.html'},console:{error(){}},setInterval:f=>timers.push(f),setTimeout:f=>f(),alert:s=>alerts.push(s),__TAURI__:{core:{invoke}},__hmDesktopStorage:{committed},addEventListener(){}};
 c.window=c;vm.runInNewContext(code,c);return {api:c.__hmDesktop,alerts,timers};
}
test('quit waits for native backup completion before exit',async()=>{
 let release,entered;const pending=new Promise(r=>{release=r;});const started=new Promise(r=>{entered=r;});const calls=[];
 const e=env(async()=>({format:1}),async(name)=>{calls.push(name);if(name==='backup_write'){entered();await pending;}});
 const close=e.api.requestClose();await started;assert.deepEqual(calls,['backup_write']);
 release();await close;assert.deepEqual(calls,['backup_write','finish_close']);
});
test('failed OS backup keeps window open and reports the failure',async()=>{
 const calls=[];const e=env(async()=>({format:1}),async name=>{calls.push(name);throw Error('Disk full');});
 await e.api.requestClose();assert.deepEqual(calls,['backup_write']);assert.match(e.alerts[0],/Disk full/);
});
test('pending game save retries without forcing game writes',async()=>{
 let tries=0;const calls=[];const e=env(async()=>++tries<3?null:{format:1},async name=>calls.push(name));
 await e.api.requestClose();assert.equal(tries,3);assert.deepEqual(calls,['backup_write','finish_close']);
});
