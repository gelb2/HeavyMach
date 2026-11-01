import assert from 'node:assert/strict';
import fs from 'node:fs';
const root=process.argv[2];
const {atSlots,consumeRestoreRequest,requestRestore}=await import(root+'/web/slot-entry.js');
let screen=1,loading=false,dirty=false;
const game={Module:{_hm_transfer_available:()=>screen===3&&!loading},hmStorageState:()=>({loaded:true,loading:false,syncing:false,dirty,blocked:false})};
for(const s of [1,2,4,6]){screen=s;assert.equal(atSlots(game),false);}
screen=3;assert.equal(atSlots(game),true);loading=true;assert.equal(atSlots(game),false);loading=false;dirty=true;assert.equal(atSlots(game),false);dirty=false;
const data=new Map();globalThis.sessionStorage={getItem:k=>data.get(k)||null,setItem:(k,v)=>data.set(k,v),removeItem:k=>data.delete(k)};
const code=fs.readFileSync(root+'/native-result.txt','utf8').trim().split('\n').at(-1);
const {codec}=await import(root+'/web/transfer.js');const c=await codec();let current=c.decode(code),pending='',writes=0,left=false;
await requestRestore(code,{source:-1,target:-1,settings:true},()=>left=true);
assert(left);assert.equal(writes,0);assert(data.size===1);
const store={read:async()=>current,preserve:async()=>{},pending:async v=>pending=v,apply:async s=>{assert.equal(data.size,0);writes++;current=s;},finish:async()=>pending=''};
assert(await consumeRestoreRequest(store));assert.equal(c.encode(current),code);assert.equal(writes,1);assert.equal(pending,'');assert.equal(await consumeRestoreRequest(store),false);
console.log('PASS slot-only entry, pending saves blocked, no writes before leaving game, one restore after leaving');
