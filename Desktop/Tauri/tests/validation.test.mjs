import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
const script=fs.readFileSync(new URL('../frontend/validation.js',import.meta.url),'utf8');
function host(){let save;const calls=[];const Module={hmSyncSave:callback=>{save=callback;}};const window={Module,hmStorageState:()=>({loaded:true}),__TAURI__:{core:{invoke:async(name,args)=>calls.push({name,args})}}};const context=vm.createContext({window,Module,console,setTimeout,clearTimeout,Date,Promise});vm.runInContext(script,context);return {window,calls,complete:ok=>save(ok)};}
test('validation close waits for its own flush and invokes only validation exit',async()=>{const h=host();const pending=h.window.__hmValidationHost.requestClose();assert.equal(h.calls.length,0);h.complete(true);await pending;assert.deepEqual(h.calls.map(x=>x.name),['validation_finish_close']);});
test('failed validation flush keeps the app open and reports only to validation storage',async()=>{const h=host();const pending=h.window.__hmValidationHost.requestClose();h.complete(false);await pending;assert.deepEqual(h.calls.map(x=>x.name),['validation_report']);assert.equal(JSON.parse(h.calls[0].args.report).phase,'close-failed');});
