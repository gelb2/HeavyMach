// Disposable Chrome profile; existing user's browser/game data is never touched.
import puppeteer from '../../../Web/tests/node_modules/puppeteer-core/lib/puppeteer/puppeteer-core.js';
import fs from 'node:fs';
const url=process.argv[2]||'http://127.0.0.1:48726/';
const out=new URL('../build/test/',import.meta.url);fs.mkdirSync(out,{recursive:true});
const gpu=process.env.HM_GPU==='1',installed=process.env.HM_INSTALLED==='1';
const report={url,gpu,installed,steps:[],errors:[],ok:false};const step=(name,data={})=>{report.steps.push({name,...data});console.log(name);};
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:!gpu,pipe:true,args:gpu?[]:['--enable-unsafe-swiftshader'],defaultViewport:{width:1200,height:800}});
const context=installed?browser.defaultBrowserContext():await browser.createBrowserContext();let page=await context.newPage();
const sleep=ms=>new Promise(r=>setTimeout(r,ms));let game;
async function title(){await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForSelector('#game:not([hidden])',{timeout:120000});game=await (await page.$('#game')).contentFrame();await game.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:60000});await game.focus('#canvas');}
const state=()=>game.evaluate(()=>hmDebugState());
async function tap(x,y){const s=await state(),[w,h]=s.canvas,scale=Math.min(1200/w,800/h);await page.mouse.click((1200-w*scale)/2+x*scale,(800-h*scale)/2+(h-y)*scale,{delay:65});}
async function waitScreen(name){await game.waitForFunction(name=>window.hmDebugState?.().screen===name&&!hmDebugState().loading,{timeout:20000},name);await sleep(1300);}
try{
 await page.goto(url);
 if(installed){
  await page.waitForSelector('#play:not([disabled])',{timeout:120000});
  const cdp=await page.createCDPSession();
  const manifest=await cdp.send('Page.getAppManifest');step('manifest',manifest);if(manifest.manifest?.id!==url)throw Error('PWA identity must match this application URL');step('installability',await cdp.send('Page.getInstallabilityErrors'));
  await cdp.send('PWA.install',{manifestId:url});
  step('pwa-installed',await cdp.send('PWA.getOsAppState',{manifestId:url}));
  // CDP installation defaults to browser display; select the manifest's standalone mode.
  await cdp.send('PWA.changeAppUserSettings',{manifestId:url,displayMode:'standalone'});
  await cdp.send('PWA.launch',{manifestId:url});
  const oldPage=page;
  page=await (await browser.waitForTarget(t=>t.type()==='page'&&t.url()===url&&t!==oldPage.target(),{timeout:30000})).page();
  await oldPage.close();await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.setViewport({width:1200,height:800});
  if(!await page.evaluate(()=>matchMedia('(display-mode: standalone)').matches))throw Error('PWA did not open standalone');step('pwa-standalone-window');
 }
 await title();step('offline-cache-and-title',{state:await state()});await sleep(6500);
 await tap(347,160);await waitScreen('menu');let s=await state();const w=s.canvas[0],dy=(s.canvas[1]-320)/2;
 await tap(w-58,21+dy);await sleep(1800);await tap(434+w/2-240,56+dy);await sleep(1800);await tap(438+w/2-240,26+dy);await waitScreen('game');
 await sleep(1500);step('mission-enter',{state:await state()});
 if(gpu){
  await page.keyboard.down('ArrowRight');await sleep(500);await page.keyboard.up('ArrowRight');await page.keyboard.down('KeyF');
  const timing=await game.evaluate(async()=>{const first=hmDebugState(),start=performance.now(),d=[];let last=start;await new Promise(resolve=>{function frame(now){d.push(now-last);last=now;if(now-start>15000)resolve();else requestAnimationFrame(frame);}requestAnimationFrame(frame);});const end=performance.now(),final=hmDebugState(),gl=document.querySelector('canvas').getContext('webgl2'),ext=gl.getExtension('WEBGL_debug_renderer_info');const a=d.slice(1).sort((a,b)=>a-b);return {mean_ms:a.reduce((s,n)=>s+n,0)/a.length,p95_ms:a[Math.floor(a.length*.95)],max_ms:a.at(-1),samples:a.length,gameTicksPerSecond:(final.time-first.time)/0.01666666753590107/((end-start)/1000),renderer:ext?gl.getParameter(ext.UNMASKED_RENDERER_WEBGL):gl.getParameter(gl.RENDERER),viewport:[innerWidth,innerHeight],first,final};});await page.keyboard.up('KeyF');step('gpu-frame-timing',timing);
 }

 await page.keyboard.down('ArrowRight');await sleep(300);await page.keyboard.up('ArrowRight');await page.keyboard.down('KeyF');let max=0;for(let i=0;i<15;i++){max=Math.max(max,(await state()).bullets);await sleep(80);}await page.keyboard.up('KeyF');if(!max)throw Error('No bullets');step('fire',{max});
 let before=await state();await page.keyboard.down('KeyD');await sleep(1300);await page.keyboard.up('KeyD');s=await state();if(s.playerX<=before.playerX)throw Error('No movement');step('move',{from:before.playerX,to:s.playerX});
 before=await state();await page.keyboard.press('Space');await sleep(300);s=await state();if(s.playerY<=before.playerY)throw Error('No jump');step('jump',{from:before.playerY,to:s.playerY});
 const aimState=await state();await tap((aimState.canvas[0]-480)/2+357,(aimState.canvas[1]-320)/2+33);await sleep(300);if((await state()).aimMode!==2)throw Error('Automatic aim did not enable');step('automatic-aim-enabled');
 await page.keyboard.press('Escape');await sleep(300);if(!(await state()).paused)throw Error('No pause');step('pause');
 await page.screenshot({path:new URL('game.png',out).pathname});
 const other=await context.newPage();await other.goto(url);await other.waitForFunction(()=>!document.getElementById('error').hidden);step('second-window-blocked',{message:await other.$eval('#error',e=>e.textContent)});await other.close();
 await game.waitForFunction(()=>{const s=hmStorageState();return s.loaded&&!s.syncing&&!s.dirty&&!s.blocked;});
 await page.reload();await page.waitForSelector('#play:not([disabled])');
 const saved=await page.evaluate(async()=>{const s=await import('./saves.js');return s.encode(await s.read());});const decoded=JSON.parse(saved);if(decoded.snapshot.settings['HeavyMach.AimModeV1']!=='2')throw Error('Automatic aim missing from export');if(!decoded.snapshot.records.some(r=>r.path==='/persist/HvM.dat'&&r.contents.length===3624))throw Error('No saved game');step('export-save',{bytes:saved.length});
 const corrupted=JSON.parse(saved);corrupted.digest='bad';const reject=await page.evaluate(async text=>{const s=await import('./saves.js');const before=await s.encode(await s.read());try{await s.restore(text);return false;}catch{return before===await s.encode(await s.read());}},JSON.stringify(corrupted));if(!reject)throw Error('Bad save was accepted');step('corrupt-import-rejected-data-unchanged');
 await page.evaluate(async text=>{const s=await import('./saves.js');await s.restore(text);},saved);const round=await page.evaluate(async()=>{const s=await import('./saves.js');return s.encode(await s.read());});if(round!==saved)throw Error('Import roundtrip mismatch');step('import-roundtrip');
 // Network disabled in this isolated browser; SW cache must serve every asset.
 await page.setOfflineMode(true);await page.reload({waitUntil:'load'});await title();step('offline-reload-game',{state:await state()});
 const size=await game.evaluate(()=>Module.FS.stat('/persist/HvM.dat').size);if(size!==3624)throw Error('Offline save lost');step('offline-progress-restored',{bytes:size});
 const expected=decoded.snapshot.records.find(r=>r.path==='/persist/HvM.dat').contents;
 const actual=await game.evaluate(()=>Array.from(Module.FS.readFile('/persist/HvM.dat')));
 if(JSON.stringify(actual)!==JSON.stringify(expected))throw Error('Offline save content changed');step('offline-save-content-matches');if((await state()).aimMode!==2)throw Error('Automatic aim preference lost');step('automatic-aim-restored');
 await sleep(6500);await tap(347,160);await waitScreen('menu');s=await state();
 await tap(s.canvas[0]-58,21+(s.canvas[1]-320)/2);await sleep(1800);
 await tap(434+s.canvas[0]/2-240,56+(s.canvas[1]-320)/2);await sleep(1800);
 await tap(438+s.canvas[0]/2-240,26+(s.canvas[1]-320)/2);await waitScreen('game');
 const offlineBefore=await state();await page.keyboard.down('KeyD');await sleep(1300);await page.keyboard.up('KeyD');
 const offlineAfter=await state();if(offlineAfter.playerX<=offlineBefore.playerX||offlineAfter.time<=offlineBefore.time)throw Error('Offline game did not advance');step('offline-mission-play',{from:offlineBefore.playerX,to:offlineAfter.playerX});
 await page.screenshot({path:new URL(installed?'installed-offline.png':'offline.png',out).pathname});report.ok=true;
}catch(e){report.errors.push(String(e.stack||e));await page.screenshot({path:new URL('failure.png',out).pathname}).catch(()=>{});}
await browser.close();fs.writeFileSync(new URL(installed?'installed-result.json':gpu?'gpu-result.json':'result.json',out),JSON.stringify(report,null,2));console.log(JSON.stringify(report,null,2));process.exit(report.ok?0:1);
