// Date-shift regression. This changes page Date only, not the browser's eviction clock.
// Uses disposable persistent profiles; never reads the user's browser or game save.
import puppeteer from '../../../Web/tests/node_modules/puppeteer-core/lib/puppeteer/puppeteer-core.js';
import fs from 'node:fs';
import http from 'node:http';
import path from 'node:path';
import assert from 'node:assert/strict';
import crypto from 'node:crypto';

const root=new URL('../build/site/',import.meta.url).pathname;
const out=new URL('../build/date-retention/',import.meta.url).pathname;
fs.mkdirSync(out,{recursive:true});
const run=fs.mkdtempSync(path.join(out,'run-'));
const report={kind:'page-date-shift-persistent-profile',browserEvictionClockChanged:false,
 osClockChanged:false,realElapsedDays:0,requestedDays:[7,14,30,40],cases:[],ok:false};
const base=Date.now(),day=86400000;
const digest=s=>crypto.createHash('sha256').update(JSON.stringify(s)).digest('hex');
const server=http.createServer((req,res)=>{
 const pathname=decodeURIComponent(new URL(req.url,'http://localhost').pathname);
 const p=path.resolve(root,'.'+(pathname==='/'?'/index.html':pathname));
 if(!p.startsWith(root)||!fs.existsSync(p)){res.writeHead(404).end();return;}
 res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm',
 '.css':'text/css','.webmanifest':'application/manifest+json','.png':'image/png'})[path.extname(p)]||'application/octet-stream');
 fs.createReadStream(p).pipe(res);
});
await new Promise(r=>server.listen(0,'127.0.0.1',r));
const url='http://127.0.0.1:'+server.address().port+'/';report.origin=url;
let browser;
async function launch(profile){
 browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',
 userDataDir:profile,headless:false,pipe:true,args:['--enable-unsafe-swiftshader','--disable-session-crashed-bubble'],
 defaultViewport:{width:1200,height:800}});
 report.browser=await browser.version();return browser;
}
async function open(mode,offset,offline=false){
 let page=await browser.newPage();
 if(mode==='pwa'){
  const cdp=await page.createCDPSession();await cdp.send('PWA.launch',{manifestId:url});
  const target=await browser.waitForTarget(t=>t.type()==='page'&&t.url()===url&&t!==page.target(),{timeout:30000});
  const app=await target.page();await page.close();page=app;
 }
 await page.evaluateOnNewDocument(epoch=>{
  const RealDate=Date,delta=epoch-RealDate.now();
  function ShiftedDate(...args){if(!new.target)return new RealDate(RealDate.now()+delta).toString();
   return new RealDate(...(args.length?args:[RealDate.now()+delta]));}
  Object.setPrototypeOf(ShiftedDate,RealDate);ShiftedDate.prototype=RealDate.prototype;
  ShiftedDate.now=()=>RealDate.now()+delta;globalThis.Date=ShiftedDate;
 },base+offset*day);
 if(offline)await page.setOfflineMode(true);
 await page.goto(url,{waitUntil:'load'});await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 assert.equal(await page.evaluate(()=>matchMedia('(display-mode: standalone)').matches),mode==='pwa');
 return page;
}
const snapshot=p=>p.evaluate(async()=>{const s=await import('./saves.js');return s.read();});
const apply=(p,s)=>p.evaluate(async s=>{const m=await import('./saves.js');await m.restore(await m.encode(s));},s);
const storage=p=>p.evaluate(async()=>({persisted:await navigator.storage.persisted(),
 periodicSync:'periodicSync' in (await navigator.serviceWorker.ready),date:new Date().toISOString()}));
function fixture(){
 const bytes=new Uint8Array(3624),v=new DataView(bytes.buffer);bytes[8]=1;
 v.setInt32(0x20,12345,true);v.setInt32(0x28,10,true);v.setInt32(0x30,2,true);
 bytes.set(new TextEncoder().encode('P_Wpn_Gun01'),0x34);v.setInt32(0x4c,99,true);
 bytes.set(new TextEncoder().encode('P_Wpn_Missile01'),0x50);v.setInt32(0x68,17,true);
 const hud=new TextEncoder().encode(JSON.stringify({version:1,positions:[[.25,.75],...Array(9).fill(null)]}));
 return {format:1,records:[{path:'/persist',timestamp:base,mode:16877,contents:null},
  {path:'/persist/HUDLayoutV1-web.json',timestamp:base,mode:33188,contents:Array.from(hud)},
  {path:'/persist/HvM.dat',timestamp:base,mode:33188,contents:Array.from(bytes)}],
  settings:{'HeavyMach.KeyboardBindingsV1':null,'HeavyMach.AimModeV1':'2'}};
}
async function gameRead(page,expected){
 await page.click('#play');await page.waitForSelector('#game:not([hidden])',{timeout:120000});
 const frame=await(await page.$('#game')).contentFrame();
 await frame.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:90000});
 const bytes=await frame.evaluate(()=>Array.from(Module.FS.readFile('/persist/HvM.dat')));
 assert.deepEqual(bytes,expected.records.find(r=>r.path.endsWith('HvM.dat')).contents);
 assert.equal(await frame.evaluate(()=>hmDebugState().aimMode),2);
 return {gameLoaded:true,bytes:bytes.length,gameDate:await frame.evaluate(()=>new Date().toISOString())};
}
try{
 for(const mode of ['browser','pwa']){
  const profile=path.join(run,mode);await launch(profile);
  let page=await open('browser',0);
  if(mode==='pwa'){
   const cdp=await page.createCDPSession();await cdp.send('PWA.install',{manifestId:url});
   await cdp.send('PWA.changeAppUserSettings',{manifestId:url,displayMode:'standalone'});await page.close();
   page=await open(mode,0);
  }
  await apply(page,fixture());let expected=await snapshot(page);await gameRead(page,expected);
  await browser.close();browser=null;
  for(const days of report.requestedDays){
   await launch(profile);page=await open(mode,days,true);
   assert.deepEqual(await snapshot(page),expected);
   const future={...await storage(page),...await gameRead(page,expected)};
   await page.reload();await page.waitForSelector('#play:not([disabled])');
   // A subsequent save at the future date must survive clock rollback too.
   const changed=await snapshot(page),record=changed.records.find(r=>r.path.endsWith('HvM.dat'));
   const b=new Uint8Array(record.contents),view=new DataView(b.buffer);view.setInt32(0x20,view.getInt32(0x20,true)+1,true);
   record.contents=Array.from(b);record.timestamp=base+days*day;await apply(page,changed);expected=await snapshot(page);
   await browser.close();browser=null;
   await launch(profile);page=await open(mode,0,true);assert.deepEqual(await snapshot(page),expected);
   const returned={...await storage(page),...await gameRead(page,expected)};
   report.cases.push({mode,days,future,returned,saveDigest:digest(expected),ok:true});
   console.log(JSON.stringify({mode,days,ok:true}));await browser.close();browser=null;
  }
  await launch(profile);page=await browser.newPage();
  if(mode==='pwa')await(await page.createCDPSession()).send('PWA.uninstall',{manifestId:url});
  await browser.close();browser=null;
 }
 report.ok=true;
}catch(e){report.error=String(e.stack||e);console.error(report.error);}
finally{if(browser)await browser.close();await new Promise(r=>server.close(r));
 fs.writeFileSync(path.join(out,'result.json'),JSON.stringify(report,null,2));}
console.log(JSON.stringify({ok:report.ok,cases:report.cases.length,result:path.join(out,'result.json')}));
process.exit(report.ok?0:1);
