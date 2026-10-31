// Isolated regression tests for installation-first navigation and cache recovery.
import puppeteer from '../../../Web/tests/node_modules/puppeteer-core/lib/puppeteer/puppeteer-core.js';
import fs from 'node:fs';import http from 'node:http';import path from 'node:path';import assert from 'node:assert/strict';
const root=new URL('../build/site/',import.meta.url).pathname,out=new URL('../build/launcher-test/',import.meta.url);fs.mkdirSync(out,{recursive:true});
let requests=[],interrupt=false;
const server=http.createServer((req,res)=>{const name=decodeURIComponent(req.url.split('?')[0]),p=path.join(root,name==='/'?'index.html':name);requests.push(name);
 if(!p.startsWith(root)||!fs.existsSync(p)){res.writeHead(404).end();return;}res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.png':'image/png','.webmanifest':'application/manifest+json'})[path.extname(p)]||'application/octet-stream');
 if(interrupt&&name.endsWith('.data')){res.write(Buffer.alloc(100));res.end();return;}fs.createReadStream(p).pipe(res);
});await new Promise(r=>server.listen(0,'127.0.0.1',r));const url='http://127.0.0.1:'+server.address().port+'/';
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader']});const context=await browser.createBrowserContext(),page=await context.newPage(),report={steps:[],ok:false};
const step=name=>{report.steps.push(name);console.log(name);};
try{
 await page.goto(url);await page.waitForSelector('#play:not([disabled])');assert(!requests.some(p=>p.startsWith('/game/')));step('first-visit-downloads-launcher-only');
 assert(await page.$eval('#install',e=>!e.hidden&&e.classList.contains('primary')));step('install-is-primary-before-game-download');
 // Safari fallback instructions: simulate only the API surface, not native installation.
 await page.evaluate(()=>{Object.defineProperty(navigator,'platform',{value:'MacIntel'});Object.defineProperty(navigator,'maxTouchPoints',{value:5});});
 await page.click('#install');assert(await page.$eval('#install-help',e=>e.textContent.includes('Home Screen')));await page.click('dialog button');step('ipad-install-instructions');
 for(const [w,h] of [[390,844],[844,390],[1024,768],[1200,800]]){
 await page.setViewport({width:w,height:h});assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth));await page.screenshot({path:new URL(`launcher-${w}x${h}.png`,out).pathname,fullPage:true});}step('mobile-tablet-desktop-layouts');
 await page.evaluate(()=>{window.downloadEvents=[];navigator.serviceWorker.addEventListener('message',e=>{if(e.data?.type==='progress')downloadEvents.push(e.data);});});
 interrupt=true;await page.click('#play');await page.waitForSelector('#error:not([hidden])',{timeout:120000});assert(await page.$eval('#game',e=>e.hidden));step('interrupted-download-does-not-start-game');
 const count=requests.length;interrupt=false;await page.focus('#retry');await page.keyboard.press('Enter');await page.waitForFunction(()=>document.getElementById('status').textContent==='Ready to play offline.',{timeout:120000});
 assert(!requests.slice(count).some(p=>p.startsWith('/skin/')));step('retry-reuses-verified-files');
 const events=await page.evaluate(()=>downloadEvents);assert(events.some(e=>e.received>0&&e.verified<e.total));assert(events.some(e=>e.phase==='ready'&&e.verified===e.total));step('byte-progress-and-verification');
 requests=[];await page.reload();await page.waitForSelector('#play:not([disabled])');assert(!requests.some(p=>p.startsWith('/game/')));step('reopen-does-not-redownload-game');
 await page.setOfflineMode(true);await page.reload();await page.waitForSelector('#play:not([disabled])');await page.click('#play');await page.waitForSelector('#game:not([hidden])');const game=await (await page.$('#game')).contentFrame();await game.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:60000});step('offline-start-after-retry');report.ok=true;
}catch(e){report.error=String(e.stack||e);report.state=await page.evaluate(()=>({status:document.getElementById('status').textContent,error:document.getElementById('error').textContent,events:window.downloadEvents?.slice(-3)}));report.requests=requests;await page.screenshot({path:new URL('failure.png',out).pathname}).catch(()=>{});}
await browser.close();await new Promise(r=>server.close(r));fs.writeFileSync(new URL('result.json',out),JSON.stringify(report,null,2));console.log(JSON.stringify(report));process.exit(report.ok?0:1);
