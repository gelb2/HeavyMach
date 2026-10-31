import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
const code=fs.readFileSync(new URL('../pwa/saves.js',import.meta.url),'utf8');
const {validate,encode,decode}=await import('data:text/javascript;base64,'+Buffer.from(code).toString('base64'));
const snapshot=aim=>({format:1,records:[],settings:{'HeavyMach.AimModeV1':aim,'HeavyMach.KeyboardBindingsV1':null}});
test('automatic aim export/import retains the game enum value 2',async()=>{
 const original=snapshot('2');assert.deepEqual(await decode(await encode(original)),original);
});
test('manual, off and unset preferences remain compatible',()=>{
 for(const mode of ['0','1',null])assert.equal(validate(snapshot(mode)).settings['HeavyMach.AimModeV1'],mode);
});
test('unknown aim values are rejected',()=>{
 for(const mode of ['3','-1','true',2])assert.throws(()=>validate(snapshot(mode)),/aim/);
});
