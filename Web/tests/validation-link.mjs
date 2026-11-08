// URL delivery uses only existing validation APIs and never changes normal play.
// node validation-link.mjs validation-url output-directory normal-url
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';
const validation=process.argv[2]||'http://127.0.0.1:8891/';
const output=path.resolve(process.argv[3]||'validation-link-results');
const normal=process.argv[4]||'http://127.0.0.1:8892/';
fs.mkdirSync(output,{recursive:true});
const report={validation,normal,cases:[],errors:[],ok:false};
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required'],defaultViewport:{width:1200,height:720}});
const context=await browser.createBrowserContext();
const check=(ok,message)=>{if(!ok)throw Error(message);};
const target=(base,unit)=>{const url=new URL(base);url.searchParams.set('validation-unit',unit);return url.href;};
try{
 for(const name of ['Ka52','Lux_Eurofighter']){
  const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));await page.goto(target(validation,name),{waitUntil:'load'});
  await page.waitForFunction(name=>window.hmValidation?.linkFixture?.status==='spawned'&&hmValidation.state().enemies.some(e=>e.name===name),{timeout:60000},name);
  await page.waitForFunction(name=>hmValidation.state().sprites.some(s=>s.tile===(name==='Ka52'?'Ka52_Body.png':'Lux_Eurofighter.png')),{timeout:5000},name);
  await new Promise(r=>setTimeout(r,4500));
  const row=await page.evaluate(()=>({fixture:hmValidation.linkFixture,savePath:hmValidation.catalog().savePath,state:hmValidation.state(),normalMount:Module.FS.analyzePath('/persist').exists}));
  check(row.fixture.status==='spawned'&&row.state.game.screen==='game','Link did not reach battle');check(row.savePath==='/validation-persist/HvM.dat'&&!row.normalMount,'Link used normal storage');
  check(row.state.cheats.blockedHits===0&&row.state.cheats.refilledRounds===0,'Link silently enabled cheats');
  report.cases.push({name,url:page.url(),ok:true,fixture:row.fixture,parts:row.state.sprites.filter(s=>s.tile.startsWith(name==='Ka52'?'Ka52':'Lux_Euro')),savePath:row.savePath});
  await page.screenshot({path:path.join(output,name+'-link.png')});await page.close();
 }
 for(const name of ['unknown-machine','','Ka52,Ka52','Ka52,Heli001,Heli002,Heli003,Heli004']){
  const page=await context.newPage();await page.goto(target(validation,name));await page.waitForFunction(()=>window.hmValidation?.linkFixture?.status==='rejected',{timeout:60000});const row=await page.evaluate(()=>({fixture:hmValidation.linkFixture,game:hmDebugState(),enemies:hmValidation.state().enemies}));
  check(row.game.screen==='title'&&row.enemies.length===0,'Rejected fixture started a game or spawned an enemy');report.cases.push({name,ok:true,rejected:row.fixture.error});await page.close();
 }
 const many=await context.newPage();const names=['Lux_F22','Lux_Su57','Lux_MiG31','Lux_Eurofighter'];await many.goto(target(validation,names.join(',')));await many.waitForFunction(()=>window.hmValidation?.linkFixture?.status==='spawned',{timeout:60000});const batch=await many.evaluate(()=>({fixture:hmValidation.linkFixture,enemies:hmValidation.state().enemies.map(e=>e.name)}));check(names.every(n=>batch.enemies.includes(n)),'Four-unit fixture incomplete');report.cases.push({name:'four-catalog-units',ok:true,fixture:batch.fixture});await many.close();
 const regular=await context.newPage();await regular.goto(target(normal,'Ka52'));await regular.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:60000});const clean=await regular.evaluate(()=>({game:hmDebugState(),bridge:typeof window.hmValidation,export:typeof Module._hm_validation_start,validation:Module.hmValidation,prefix:Module.hmPreferencePrefix}));check(clean.bridge==='undefined'&&clean.export==='undefined'&&!clean.validation&&clean.game.screen==='title','Normal build acted on the validation URL');report.cases.push({name:'normal-ignores-query',ok:true,...clean});await regular.close();
 report.ok=report.errors.length===0;
}catch(error){report.errors.push(error.stack||String(error));}
finally{fs.writeFileSync(path.join(output,'report.json'),JSON.stringify(report,null,2)+'\n');await browser.close();}
console.log(JSON.stringify({ok:report.ok,cases:report.cases.length,errors:report.errors}));if(!report.ok)process.exitCode=1;
