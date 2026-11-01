// Touch/rotation host integration in an isolated Chromium profile; not Safari hardware evidence.
import puppeteer from '../../../Web/tests/node_modules/puppeteer-core/lib/puppeteer/puppeteer-core.js';
import fs from 'node:fs';import assert from 'node:assert/strict';
const url=process.argv[2]||'http://127.0.0.1:48726/',out=new URL('../build/mobile-test/',import.meta.url);fs.mkdirSync(out,{recursive:true});
const landscape={width:912,height:604,deviceScaleFactor:2,isMobile:true,hasTouch:true,isLandscape:true};
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader'],defaultViewport:landscape});const page=await browser.newPage();let game;
const report={environment:'Chromium touch emulation, SwiftShader',steps:[],ok:false},step=(name,data={})=>{report.steps.push({name,...data});console.log(name);},sleep=ms=>new Promise(r=>setTimeout(r,ms));
const state=()=>game.evaluate(()=>hmDebugState());
async function tap(x,y){const s=await state(),[w,h]=s.canvas,k=Math.min(landscape.width/w,landscape.height/h);await page.touchscreen.tap((landscape.width-w*k)/2+x*k,(landscape.height-h*k)/2+(h-y)*k);}
async function screen(name){await game.waitForFunction(name=>window.hmDebugState?.().screen===name&&!hmDebugState().loading,{timeout:60000},name);await sleep(1500);}
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])');await page.tap('#play');await page.waitForSelector('#game:not([hidden])',{timeout:120000});game=await(await page.$('#game')).contentFrame();await screen('title');await sleep(6000);
 await tap(347,160);await screen('menu');let s=await state(),my=(s.canvas[1]-320)/2,c=s.canvas[0]/2;
 await tap(s.canvas[0]-58,21+my);await sleep(2500);await tap(434+c-240,56+my);await sleep(2500);await tap(438+c-240,26+my);await screen('game');step('touch-enters-mission',{state:await state()});
 await page.screenshot({path:new URL('tablet-game.png',out).pathname});
 assert.equal((await state()).aimMode,1);
 await page.keyboard.down('ArrowRight');await sleep(600);await page.keyboard.up('ArrowRight');const aimX=(await state()).keyboardAimX;await page.screenshot({path:new URL('keyboard-aim-right.png',out).pathname});
 await page.keyboard.down('ArrowLeft');await sleep(400);await page.keyboard.up('ArrowLeft');assert((await state()).keyboardAimX<aimX);await page.screenshot({path:new URL('keyboard-aim-left.png',out).pathname});step('arrow-keys-move-manual-aim');
 await page.setViewport({...landscape,width:604,height:912,isLandscape:false});await sleep(800);const p1=await state();await sleep(600);const p2=await state();assert.equal(p2.orientationHold,1);assert.equal(p1.time,p2.time);step('portrait-holds-game');
 await page.setViewport(landscape);await sleep(800);assert.equal((await state()).orientationHold,0);assert((await state()).time>p2.time);step('landscape-resumes');
 await page.keyboard.press('Escape');await sleep(300);assert.equal((await state()).gameMenu,1);await tap(c-40,190+my);await sleep(800);assert.equal((await state()).hudEditing,1);step('pause-opens-hud-editor');
 await page.screenshot({path:new URL('tablet-hud-editor.png',out).pathname});
 const hx=((await state()).canvas[0]-480)/2;
 const css=(x,y)=>[(hx+x)*landscape.width/(480+2*hx),(320-y)*landscape.height/320];
 const [x,y]=css(51,306),[x2,y2]=css(111,236);
 await page.touchscreen.touchStart(x,y);
 for(let i=1;i<=10;i++){await page.touchscreen.touchMove(x+(x2-x)*i/10,y+(y2-y)*i/10);await sleep(30);}
 await page.touchscreen.touchEnd();assert.equal((await state()).hudDraftCustomized,1);
 async function control(label){const b=await game.evaluate(label=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).buttons.find(b=>b.label===label&&b.enabled),label);if(!b)throw Error('Missing game button '+label);await tap(b.rect[0]+b.rect[2]/2,b.rect[1]+b.rect[3]/2);await sleep(300);}
 await control('TOOLS');await control('APPLY');assert.equal((await state()).hudCustomized,1);
 const saved=await game.evaluate(()=>new TextDecoder().decode(Module.FS.readFile('/persist/HUDLayoutV1-web.json')));
 step('touch-hud-drag-and-save',{layout:JSON.parse(saved)});await sleep(2000);
 await page.reload();await page.waitForSelector('#play:not([disabled])');await page.tap('#play');await page.waitForSelector('#game:not([hidden])',{timeout:120000});game=await(await page.$('#game')).contentFrame();await screen('title');
 assert.equal(await game.evaluate(()=>new TextDecoder().decode(Module.FS.readFile('/persist/HUDLayoutV1-web.json'))),saved);step('hud-restored-after-reload');report.ok=true;
}catch(e){report.error=String(e.stack||e);await page.screenshot({path:new URL('failure.png',out).pathname}).catch(()=>{});}
await browser.close();fs.writeFileSync(new URL('result.json',out),JSON.stringify(report,null,2));console.log(JSON.stringify(report));process.exit(report.ok?0:1);
