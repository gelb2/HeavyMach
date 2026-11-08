// Install and launch the isolated validation build as a real Chrome PWA.
// Use a task-owned profile; no user Chrome profile or production PWA is touched.
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';
const url=process.argv[2]||'http://127.0.0.1:8873/';
const out=path.resolve(process.argv[3]||'pwa-validation-out');fs.mkdirSync(out,{recursive:true});
const browser=await puppeteer.launch({executablePath:'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:false,pipe:true,userDataDir:path.join(out,'chrome-profile'),defaultViewport:null,args:['--autoplay-policy=no-user-gesture-required','--disable-background-timer-throttling','--disable-renderer-backgrounding','--window-size=1200,800']});
// Chromium documents PWA CDP commands for pipe:true browser sessions:
// https://chromium.googlesource.com/chromium/src/+/main/chrome/browser/web_applications/docs/cdp-integration.md
const rootClient=await browser.target().createCDPSession();
const report={platform:'installed-chrome-pwa',url,ok:false,errors:[]};let installed=false;let manifestId;
try{
 const page=await browser.newPage();await page.goto(url);await page.waitForFunction(()=>window.hmValidation,{timeout:60000});
 const manifest=await page.evaluate(async()=>fetch(document.querySelector('link[rel=manifest]').href).then(r=>r.json()));manifestId=new URL(manifest.id,url).href;report.manifest=manifest;report.manifestId=manifestId;
 const client=rootClient;await client.send('PWA.install',{manifestId,installUrlOrBundleUrl:url});installed=true;report.installed=await client.send('PWA.getOsAppState',{manifestId});
 await client.send('PWA.changeAppUserSettings',{manifestId,displayMode:'standalone'});
 const priorTargets=new Set(browser.targets());
 const launched=await client.send('PWA.launch',{manifestId});report.targetId=launched.targetId;
 report.targetsAfterLaunch=await client.send('Target.getTargets');
 const target=await browser.waitForTarget(t=>!priorTargets.has(t)&&t.url()===url,{timeout:30000});const app=await target.page();await app.bringToFront();await app.waitForFunction(()=>window.hmValidation,{timeout:60000});
 report.display=await app.evaluate(()=>({standalone:matchMedia('(display-mode: standalone)').matches,fullscreen:matchMedia('(display-mode: fullscreen)').matches,url:location.href,ua:navigator.userAgent}));if(!report.display.standalone)throw Error('Installed app did not launch in standalone display mode');await page.close();
 let combat;await app.exposeFunction('capturePWAValidation',value=>{combat=JSON.parse(value);console.log(combat.phase);});
 // Reuse the actual combat assertions. Only the report sink is supplied by the
 // harness; PWA has no Tauri host IPC. No game/combat result is mocked.
 await app.evaluate(()=>{window.__TAURI__={core:{invoke:async(name,args)=>{if(name==='validation_report')return window.capturePWAValidation(args.report);throw Error('Native Tauri commands do not exist in the PWA');}}};window.__hmValidationAutorun=true;});
 await app.addScriptTag({content:fs.readFileSync(new URL('../../Desktop/Tauri/frontend/validation.js',import.meta.url),'utf8')});
 const deadline=Date.now()+180000;while(combat?.phase!=='complete'){if(Date.now()>deadline)throw Error('PWA combat validation timed out');await new Promise(r=>setTimeout(r,500));}
 delete combat.productionCommandsBlocked;combat.platform='installed-chrome-pwa';combat.nativeIPC='not applicable; only JSON report sink supplied by harness';report.combat=combat;
 await app.screenshot({path:path.join(out,'installed-app.png')});report.saveFlushed=await app.evaluate(()=>new Promise(resolve=>Module.hmSyncSave(resolve)));report.ok=combat.ok&&report.saveFlushed;
 await app.close();await client.send('PWA.uninstall',{manifestId});installed=false;report.cleanedUp=true;
}catch(error){report.errors.push(error.stack||String(error));}
finally{if(installed){try{const client=rootClient;await client.send('PWA.uninstall',{manifestId});report.cleanedUp=true;}catch(error){report.errors.push('cleanup: '+error);}}fs.writeFileSync(path.join(out,'report.json'),JSON.stringify(report,null,2)+'\n');await browser.close();}
console.log(JSON.stringify({ok:report.ok,display:report.display,units:report.combat?.units.length,weapons:report.combat?.weapons.length,errors:report.errors}));if(!report.ok)process.exitCode=1;
