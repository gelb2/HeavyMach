import puppeteer from '../../../Web/tests/node_modules/puppeteer-core/lib/puppeteer/puppeteer-core.js';
import http from 'node:http';import fs from 'node:fs';import path from 'node:path';import assert from 'node:assert/strict';
const root=path.resolve('Distribution/Web/build/site'),out=path.resolve('build/save-transfer');fs.mkdirSync(out,{recursive:true});
let server;
if(!process.env.HM_URL){server=http.createServer((req,res)=>{const p=path.join(root,req.url.split('?')[0]==='/'?'index.html':decodeURIComponent(req.url.split('?')[0]));if(!p.startsWith(root)||!fs.existsSync(p)){res.writeHead(404).end();return;}res.setHeader('Content-Type',({'.html':'text/html','.js':'text/javascript','.wasm':'application/wasm','.css':'text/css','.png':'image/png','.webmanifest':'application/manifest+json'})[path.extname(p)]||'application/octet-stream');fs.createReadStream(p).pipe(res);});await new Promise(r=>server.listen(0,'127.0.0.1',r));}
const url=process.env.HM_URL||'http://127.0.0.1:'+server.address().port+'/';
const touch=process.env.HM_TOUCH==='1',installed=process.env.HM_INSTALLED==='1';
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:!installed,pipe:true,args:['--enable-unsafe-swiftshader'],defaultViewport:{width:1024,height:768,hasTouch:touch,isMobile:touch}});
let page=await browser.newPage(),game;const errors=[],steps=[];let report;const sleep=ms=>new Promise(r=>setTimeout(r,ms));
const watch=p=>p.on('pageerror',e=>errors.push(e.message));watch(page);
const ui=()=>game.evaluate(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())));
async function tap(x,y){
 if(installed){await page.bringToFront();await game.focus("#canvas");await sleep(100);}
 const p=await game.evaluate(([x,y])=>{const s=hmDebugState(),r=document.querySelector('#canvas').getBoundingClientRect(),[cw,ch]=s.canvas,k=Math.min(r.width/cw,r.height/ch);return {x:r.x+(r.width-cw*k)/2+x*k,y:r.y+(r.height-ch*k)/2+(ch-y)*k};},[x,y]);
 const f=await(await page.$('#game')).boundingBox();
 if(touch)await page.touchscreen.tap(f.x+p.x,f.y+p.y);else{await page.mouse.move(f.x+p.x,f.y+p.y);await page.mouse.down();await sleep(70);await page.mouse.up();}await sleep(120);
}
async function button(id){await game.waitForFunction(id=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).buttons.some(b=>b.id===id&&b.enabled),{},id);const b=(await ui()).buttons.find(b=>b.id===id);await tap(b.rect[0]+b.rect[2]/2,b.rect[1]+b.rect[3]/2);}
async function enterSlots(){
 await page.waitForSelector('#play:not([disabled])',{timeout:120000});await page.click('#play');await page.waitForSelector('#game:not([hidden])');game=await(await page.$('#game')).contentFrame();
 await game.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:90000});assert.equal((await ui()).buttons.some(b=>b.id===900),false);if(installed)await page.bringToFront();await game.focus('#canvas');await game.waitForFunction(()=>hmDebugState().time>=7.5,{timeout:60000});await tap(347,160);await game.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).buttons.some(b=>b.id===900&&b.enabled),{timeout:15000});
}
const save=()=>page.evaluate(async()=>{const m=await import('./saves.js');return JSON.stringify(await m.read());});
try{
 await browser.defaultBrowserContext().overridePermissions(new URL(url).origin,['clipboard-read','clipboard-write','clipboard-sanitized-write']);
 const downloadDir=fs.mkdtempSync(path.join(out,'button-download-'));const downloadSession=await browser.target().createCDPSession();await downloadSession.send('Browser.setDownloadBehavior',{behavior:'allow',downloadPath:downloadDir});
 await page.goto(url);
 if(installed){await page.waitForSelector('#play:not([disabled])',{timeout:120000});const cdp=await page.createCDPSession();await cdp.send('PWA.install',{manifestId:url});await cdp.send('PWA.changeAppUserSettings',{manifestId:url,displayMode:'standalone'});const old=page;await cdp.send('PWA.launch',{manifestId:url});page=await(await browser.waitForTarget(t=>t.type()==='page'&&t.url()===url&&t!==old.target())).page();await old.close();await page.setViewport({width:1024,height:768,hasTouch:touch,isMobile:touch});watch(page);assert(await page.evaluate(()=>matchMedia('(display-mode: standalone)').matches));steps.push('installed standalone PWA');}
 await enterSlots();steps.push('title hidden; original title button enters slots; game transfer visible');
 if(touch){await tap(200,200);await game.waitForFunction(()=>!!document.fullscreenElement);steps.push('touch game fullscreen active');}
 assert.equal(await game.evaluate(()=>document.querySelectorAll('.hm-transfer-button,.hm-transfer-dialog,#hm-hud-ui button').length),0);steps.push('no DOM game buttons or transfer dialog');
 await game.evaluate(()=>{const write=navigator.clipboard.writeText.bind(navigator.clipboard);navigator.clipboard.writeText=text=>write(text).catch(e=>{window.__clipboardError=String(e);throw e;});const action=Module.hmTransferAction;window.__transferActions=[];Module.hmTransferAction=id=>{window.__transferActions.push(id);return action(id);};const call=Module.ccall;Module.ccall=function(name,...args){if(name==='hm_transfer_state')window.__transferState=args.at(-1);return call.call(this,name,...args);};});const initial=await save();await button(900);assert.equal((await ui()).transfer,1);await button(901);await game.waitForFunction(()=>document.querySelector('textarea').value.startsWith('HM1-'));
 const ownCode=await game.$eval('textarea',e=>e.value);assert.equal(await save(),initial);steps.push('canvas CREATE CODE preserves saved data');
 await button(902);await sleep(200);assert.equal(await game.evaluate(()=>navigator.clipboard.readText()),ownCode);steps.push('canvas COPY writes recovery code to clipboard');
 await game.evaluate(()=>{window.__savedClipboardWrite=navigator.clipboard.writeText;navigator.clipboard.writeText=()=>new Promise(resolve=>window.__releaseClipboard=resolve);});
 await button(902);await button(909);await game.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).transfer===2);
 const confirmSummary=await game.evaluate(()=>window.__transferState.at(-1));await game.evaluate(()=>window.__releaseClipboard());await sleep(100);
 assert.equal(await game.evaluate(()=>window.__transferState.at(-1)),confirmSummary);assert.equal((await ui()).transfer,2);await button(912);
 await game.evaluate(()=>{navigator.clipboard.writeText=window.__savedClipboardWrite;});steps.push('late COPY completion preserves restore confirmation summary');

 await button(903);const downloaded=path.join(downloadDir,'HeavyMach.hmcode');for(let i=0;!fs.existsSync(downloaded)&&i<100;i++)await sleep(100);assert.equal(fs.readFileSync(downloaded,'utf8'),ownCode);steps.push('canvas SAVE CODE FILE downloads exact code');
 await game.$eval('textarea',e=>e.value='');const choosing=page.waitForFileChooser();await button(904);await(await choosing).accept([downloaded]);await game.waitForFunction(()=>document.querySelector('textarea').value.startsWith('HM1-'));assert.equal(await game.$eval('textarea',e=>e.value),ownCode);steps.push('canvas OPEN CODE FILE loads exact code');

 await game.focus('textarea');await page.keyboard.press('ArrowLeft');await page.keyboard.press('ArrowRight');assert(await game.evaluate(()=>Module._hm_transfer_available()));steps.push('text input keys do not select a slot');
 await page.screenshot({path:path.join(out,'slots-game-transfer.png')});
 await game.$eval('textarea',e=>e.value='HM1-INVALID');await button(908);await sleep(300);assert.equal((await ui()).transfer,1);assert.equal(await save(),initial);steps.push('invalid code rejected without save change');
 await game.$eval('textarea',(e,c)=>e.value=c,ownCode);await button(909);await game.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).transfer===2);await button(912);assert.equal((await ui()).transfer,1);assert.equal(await save(),initial);steps.push('game confirmation CANCEL preserves save');
 await button(905);await button(906);await button(907);await button(907);for(let i=0;i<3;i++){await button(905);await button(906);}steps.push('slot and HUD-control options operate as game buttons');
 if(touch){
  for(const held of ['button','blank']){
  const controls=await ui(),rect=id=>controls.buttons.find(b=>b.id===id).rect;
  const pos=r=>[r[0]+r[2]/2,r[1]+r[3]/2];
  const points=await game.evaluate(([first,second])=>{const s=hmDebugState(),r=document.querySelector('#canvas').getBoundingClientRect(),[cw,ch]=s.canvas,k=Math.min(r.width/cw,r.height/ch);return [first,second,[cw-58,21+(ch-320)/2]].map(([x,y])=>({x:r.x+(r.width-cw*k)/2+x*k,y:r.y+(r.height-ch*k)/2+(ch-y)*k}));},[pos(rect(910)),held==='button'?pos(rect(901)):[18,272]]);
  const frame=await(await page.$('#game')).boundingBox();points.forEach(p=>{p.x+=frame.x;p.y+=frame.y;});
  const cdp=await page.createCDPSession(),p0={...points[0],id:0},p1={...points[1],id:1};
  await cdp.send('Input.dispatchTouchEvent',{type:'touchStart',touchPoints:[p0,p1]});await sleep(70);
  await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[p0]});await sleep(120);assert.equal((await ui()).transfer,0);
  await cdp.send('Input.dispatchTouchEvent',{type:'touchMove',touchPoints:[{...points[2],id:1}]});await cdp.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});await sleep(300);
  assert(await game.evaluate(()=>Module._hm_transfer_available()));await button(900);steps.push(held+' finger released after CLOSE cannot activate underlying OK');
  }
 }

 await button(910);assert.equal((await ui()).transfer,0);assert(await game.$eval('textarea',e=>e.hidden));
 // Original slot/mode/mission buttons remain the route into gameplay.
 let s=await game.evaluate(()=>hmDebugState());await tap(s.canvas[0]-58,21+(s.canvas[1]-320)/2);await sleep(1600);assert.equal((await ui()).buttons.some(b=>b.id===900),false);
 await tap(434+s.canvas[0]/2-240,56+(s.canvas[1]-320)/2);await sleep(1600);assert.equal((await ui()).buttons.some(b=>b.id===900),false);
 await tap(438+s.canvas[0]/2-240,26+(s.canvas[1]-320)/2);await game.waitForFunction(()=>hmDebugState().screen==='game'&&!hmDebugState().loading,{timeout:20000});assert.equal((await ui()).buttons.some(b=>b.id===900),false);steps.push('original menus enter combat; transfer entry hidden');
 await game.focus('#canvas');await page.keyboard.press('Escape');await sleep(1200);assert(await game.evaluate(()=>hmDebugState().gameMenu));steps.push('original pause command works');
 await page.reload();await enterSlots();await game.focus("#canvas");await page.keyboard.press("F3");await game.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).transfer===1);steps.push("F3 opens game transfer screen");
 const fixture=fs.readFileSync(process.env.HM_TRANSFER_CODE_FILE||path.join(out,'native-result.txt'),'utf8').trim().split('\n').at(-1);
 await game.$eval('textarea',(e,c)=>e.value=c,fixture);await button(909);await game.waitForFunction(()=>JSON.parse(Module.UTF8ToString(Module._hm_game_ui())).transfer===2);
 await page.screenshot({path:path.join(out,'slots-game-confirm.png')});
 const navigation=page.waitForNavigation({waitUntil:'load',timeout:30000});await button(911);await navigation;await page.waitForSelector('#play:not([disabled])',{timeout:120000});
 const restored=()=>page.evaluate(async()=>{const {codec}=await import('./transfer/transfer.js');const {portable}=await import('./transfer/store.js');const saves=await import('./saves.js');return(await codec()).encode(portable(await saves.read()));});
 assert.equal(await restored(),fixture);steps.push('canvas RESTORE matches native fixture exactly');
 await page.setOfflineMode(true);await page.reload({waitUntil:'load'});await page.waitForSelector('#play:not([disabled])',{timeout:60000});assert.equal(await restored(),fixture);steps.push('offline reload preserves every recovered value');
 await enterSlots();await page.screenshot({path:path.join(out,'slots-game-offline.png')});steps.push('offline game opens restored slots and game transfer button');
 assert.equal(errors.length,0,errors.join('\n'));report={ok:true,url,gameURL:game.url(),touch,installed,steps};
}catch(e){report={ok:false,url,touch,installed,steps,error:String(e.stack),errors};try{report.ui=await ui();report.game=await game.evaluate(()=>({state:hmDebugState(),storage:hmStorageState(),ccall:typeof Module.ccall,entry:typeof Module._hm_transfer_state,available:Module._hm_transfer_available(),action:typeof Module.hmTransferAction,inputs:document.querySelectorAll("textarea").length,actions:window.__transferActions,transferState:window.__transferState,focus:document.activeElement.tagName,clipboardError:window.__clipboardError}));}catch{}await page.screenshot({path:path.join(out,'slots-game-failure.png')}).catch(()=>{});}
finally{await browser.close();if(server)await new Promise(r=>server.close(r));}
fs.writeFileSync(path.join(out,touch?'game-slots-touch-result.json':installed?'game-slots-installed-result.json':'game-slots-browser-result.json'),JSON.stringify(report,null,2));console.log(JSON.stringify(report));process.exit(report.ok?0:1);
