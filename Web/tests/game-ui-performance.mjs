// Same isolated browser and scene before/after showing the additional game controls.
import puppeteer from 'puppeteer-core';import fs from 'node:fs';
const url=process.argv[2]||'http://127.0.0.1:48953/',out=process.argv[3]||'build/game-buttons/web-ui-performance.json';
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader','--disable-background-timer-throttling','--disable-renderer-backgrounding'],defaultViewport:{width:912,height:420}}),page=await browser.newPage();
const sleep=ms=>new Promise(r=>setTimeout(r,ms));let report={url,environment:'Isolated desktop Chromium; renderer recorded below',samples:[]};
const state=()=>page.evaluate(()=>hmDebugState()),ui=()=>page.evaluate(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())));
const tap=async(x,y)=>{const s=await state(),[w,h]=s.canvas,k=Math.min(912/w,420/h);await page.mouse.click((912-w*k)/2+x*k,(420-h*k)/2+(h-y)*k,{delay:60});};
async function sample(name){await sleep(1000);const before=await ui(),a=await state(),start=Date.now();await sleep(5000);const after=await ui(),b=await state(),elapsed=(Date.now()-start)/1000;const number=(s,k)=>Number(new RegExp(k+'=([0-9]+)').exec(s)[1]);report.samples.push({name,elapsed,frames:number(after.graphics,'frames')-number(before.graphics,'frames'),fps:(number(after.graphics,'frames')-number(before.graphics,'frames'))/elapsed,drawCalls:number(after.graphics,'drawCalls'),decodes:b.decodes-a.decodes,uploads:number(after.graphics,'uploads')-number(before.graphics,'uploads'),graphics:after.graphics});}
try{
 await page.goto(url);await page.waitForFunction(()=>window.hmDebugState?.().time>=7.5,{timeout:90000});await tap(347,160);await sleep(2500);let s=await state(),my=(s.canvas[1]-320)/2,c=s.canvas[0]/2;
 await tap(s.canvas[0]-58,21+my);await sleep(2500);await tap(434+c-240,56+my);await sleep(2500);await tap(438+c-240,26+my);await page.waitForFunction(()=>hmDebugState().screen==='game'&&!hmDebugState().loading);
 await sample('combat');await page.keyboard.press('Escape');await sample('pause-menu');await tap(c,190+my);await page.waitForFunction(()=>hmDebugState().hudEditing);await sample('hud-editor-tools');
 const b=(await ui()).buttons.find(b=>b.id===803);await tap(b.rect[0]+b.rect[2]/2,b.rect[1]+b.rect[3]/2);await sample('hud-editor-preview');
 report.renderer=await page.evaluate(()=>{const gl=document.querySelector('#canvas').getContext('webgl2'),ext=gl?.getExtension('WEBGL_debug_renderer_info');return ext?gl.getParameter(ext.UNMASKED_RENDERER_WEBGL):'unavailable';});
 report.ok=report.samples.every(s=>s.frames>0&&(s.name==='combat'||(s.decodes===0&&s.uploads===0)));
 report.combatLoads='Combat can load newly spawned units and effects; UI samples must not decode or upload per frame.';
}catch(e){report.ok=false;report.error=String(e.stack);}finally{await browser.close();fs.writeFileSync(out,JSON.stringify(report,null,2));console.log(JSON.stringify(report));}process.exit(report.ok?0:1);
