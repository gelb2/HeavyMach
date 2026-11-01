// Regression: pending-EXP upgrade, IndexedDB failures, HUD commit acknowledgement,
// and WebGL context restoration. All saves live in a disposable browser profile.
import puppeteer from 'puppeteer-core';
import fs from 'node:fs';
import assert from 'node:assert/strict';
const url=process.argv[2]||'http://127.0.0.1:48724/',out=process.argv[3]||'recovery-out';
fs.mkdirSync(out,{recursive:true});
const sleep=ms=>new Promise(r=>setTimeout(r,ms)),checks=[],errors=[];
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required','--disable-background-timer-throttling','--disable-renderer-backgrounding'],defaultViewport:{width:912,height:420}});
let page=await browser.newPage();page.on('pageerror',e=>errors.push(e.message));
const state=()=>page.evaluate(()=>hmDebugState());
const check=(name,ok,data={})=>{checks.push({name,ok:!!ok,...data});assert.ok(ok,name);};
const wait=async fn=>{for(let i=0;i<400;i++){const s=await state();if(fn(s))return s;await sleep(100);}throw Error('state timeout '+JSON.stringify(await state()));};
const tap=async(x,y)=>{const s=await state(),[w,h]=s.canvas,k=Math.min(912/w,420/h);await page.mouse.click((912-w*k)/2+x*k,(420-h*k)/2+(h-y)*k,{delay:60});};
async function enter(){await page.goto(url);await page.focus('#canvas');await wait(s=>s.screen==='title');await sleep(7500);await tap(347,160);let s=await wait(s=>s.screen==='menu'&&!s.loading);await sleep(600);const [w,h]=s.canvas,y=(h-320)/2,x=w/2-240;await tap(w-58,21+y);await sleep(2600);await tap(434+x,56+y);await sleep(3000);await tap(438+x,26+y);await wait(s=>s.screen==='game'&&!s.loading);await sleep(1200);}
const saved=()=>page.evaluate(()=>Array.from(Module.FS.readFile('/persist/HvM.dat')));
try{
 await enter();
 await page.evaluate(async()=>{const b=Module.FS.readFile('/persist/HvM.dat'),v=new DataView(b.buffer,b.byteOffset+0x4b8,0x4b8);v.setInt32(0x20,2415,true);v.setInt32(0x28,3,true);v.setInt32(0xc,1,true);v.setInt32(0x10,0,true);Module.FS.writeFile('/persist/HvM.dat',b);await new Promise((r,j)=>Module.FS.syncfs(false,e=>e?j(e):r()));});
 await enter();let a=await state();await sleep(900);let b=await state();
 check('pending EXP pauses combat',a.paused&&b.paused&&a.hp===b.hp&&a.playerX===b.playerX,{a,b});
 await page.keyboard.press('Escape');await page.keyboard.press('KeyF');b=await state();check('keyboard cannot bypass upgrade',b.paused&&!b.gameMenu);
 await page.evaluate(()=>{Module._hm_active(0);Module._hm_active(1);});b=await state();check('foreground retains upgrade',b.paused&&!b.gameMenu);
 const dx=(b.canvas[0]-480)/2,dy=(b.canvas[1]-320)/2;
 await tap(139+dx,172+dy);await tap(294+dx,95+dy);await sleep(1400);b=await state();check('upgrade selection and OK resume combat',!b.paused&&!b.inputBlocked,{state:b});await page.screenshot({path:out+'/upgrade-fixed.png'});
 // Repeated context loss must preserve the session and require explicit Resume.
 for(let i=0;i<2;i++){
  await page.evaluate(()=>{window.recoveryGL=document.querySelector('#canvas').getContext('webgl2');window.recoveryExt=recoveryGL.getExtension('WEBGL_lose_context');recoveryExt.loseContext();});await sleep(300);a=await state();await sleep(600);b=await state();check('context loss freezes session '+i,!b.active&&b.paused&&a.time===b.time&&a.hp===b.hp,{a,b});
  await page.evaluate(()=>recoveryExt.restoreContext());await wait(s=>s.active===1);b=await state();check('context restored with same session '+i,b.paused&&b.gameMenu&&a.playerX===b.playerX&&a.hp===b.hp&&a.mission===b.mission,{state:b});
  check('WebGL valid after restore '+i,await page.evaluate(()=>!recoveryGL.isContextLost()));
  await page.screenshot({path:out+'/restored-'+i+'.png'});await page.keyboard.press('Escape');await sleep(400);check('explicit resume after restore '+i,!(await state()).paused);
 }
 // HUD APPLY remains open while the async write is pending or fails.
 await page.keyboard.press('Escape');await sleep(300);await page.evaluate(()=>Module._hm_hud_control(-1));await wait(s=>s.hudEditing===1);
 const hs=await state(),k=420/hs.canvas[1],hx=(hs.canvas[0]-480)/2,hy=(hs.canvas[1]-320)/2;
 await page.mouse.move((hx+51)*k,(hs.canvas[1]-hy-306)*k);await page.mouse.down();
 await page.mouse.move((hx+111)*k,(hs.canvas[1]-hy-236)*k,{steps:10});await page.mouse.up();
 await page.evaluate(()=>{window.realSync=FS.syncfs;window.delayed=[];FS.syncfs=(populate,cb)=>delayed.push(cb);Module._hm_hud_control(2);});
 check('HUD waits for commit', (await state()).hudEditing===1 && await page.evaluate(()=>delayed.length===1));
 await page.keyboard.press('Escape');check('pending HUD save cannot close editor',(await state()).hudEditing===1);
 await page.evaluate(()=>delayed.shift()(new Error('quota failure')));
 check('HUD write failure keeps draft', (await state()).hudEditing===1 && await page.evaluate(()=>hmStorageState().dirty&&hmStorageState().blocked));
 await page.evaluate(()=>FS.syncfs=realSync);await page.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).buttons.some(b=>b.id===1030));const retry=await page.evaluate(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).buttons.find(b=>b.id===1030));await tap(retry.rect[0]+retry.rect[2]/2,retry.rect[1]+retry.rect[3]/2);await page.waitForFunction(()=>!hmStorageState().dirty);await page.evaluate(()=>Module._hm_hud_control(2));await wait(s=>!s.hudEditing);check('HUD retry commits draft',(await state()).hudCustomized===1);
 // A failed startup read must not launch or overwrite the existing saved game.
 await page.waitForFunction(()=>!hmStorageState().dirty);const expected=await saved();await page.close();page=await browser.newPage();
 await page.evaluateOnNewDocument(()=>{const open=indexedDB.open.bind(indexedDB);let first=true;indexedDB.open=(...a)=>{if(first){first=false;throw new Error('transient read failure');}return open(...a);};});
 await page.goto(url);await page.waitForFunction(()=>window.hmStorageState&&hmStorageState().blocked);check('failed read blocks startup',(await state()).screen==='loading');
 await page.click('#hm-storage-retry');await wait(s=>s.screen==='title');check('read retry preserves exact save',JSON.stringify(await saved())===JSON.stringify(expected));
 await page.screenshot({path:out+'/read-recovered.png'});
 check('no runtime JS exceptions',errors.length===0,{errors});
}catch(e){checks.push({name:'exception',ok:false,error:String(e.stack||e)});process.exitCode=1;}
finally{fs.writeFileSync(out+'/results.json',JSON.stringify({checks,errors,ok:checks.every(c=>c.ok)},null,2));await browser.close();console.log(JSON.stringify(checks,null,2));}
