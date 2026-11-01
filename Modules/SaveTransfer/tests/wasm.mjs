import fs from 'node:fs';import assert from 'node:assert/strict';
const root=process.argv[2];const {codec}=await import(root+'/web/transfer.js');const c=await codec();const native=fs.readFileSync(root+'/native-result.txt','utf8').trim().split('\n').at(-1);
const s=c.decode(native);assert.equal(s.aim,2);assert.equal(s.keys.length,16);assert.equal(c.encode(s),native);let state=structuredClone(s),journal='',before='';
const store={read:async()=>state,preserve:async s=>{before=s;},pending:async s=>{journal=s;},apply:async s=>{state=structuredClone(s);},finish:async()=>{journal='';}};
await c.restore(store,native);assert.equal(c.encode(state),native);assert.equal(journal,'');assert.equal(before,native);
for(let i=4;i<native.length;i++){const b=native.slice(0,i)+(native[i]==='A'?'B':'A')+native.slice(i+1);assert.throws(()=>c.decode(b));}
console.log('PASS native/Wasm identical code, restoration transaction, all single-character mutations');
