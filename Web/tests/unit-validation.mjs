// Isolated validation app: real browser/WebGL, actual session fire/bullet/collision.
// node unit-validation.mjs validation-url output-directory [normal-url]
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';
const url=process.argv[2]||'http://127.0.0.1:8873/';
const out=process.argv[3]||'unit-validation-out';
const normalURL=process.argv[4];
fs.mkdirSync(out,{recursive:true});
const report={url,normalURL,scope:'Chrome WebGL validation build; explicit enemy fixtures use unchanged session spawn/fire/collision',units:[],errors:[],ok:false};
const sleep=ms=>new Promise(r=>setTimeout(r,ms));
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required','--disable-background-timer-throttling','--disable-renderer-backgrounding'],defaultViewport:{width:1024,height:600}});
const context=await browser.createBrowserContext();
const page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
const assert=(value,message)=>{if(!value)throw new Error(message);};
const state=()=>page.evaluate(()=>hmValidation.state());
const units=[['VTOL_Yak141','VTOL_Yak141.png',true],['VTOL_Harrier','VTOL_Harrier.png',true],['VTOL_F35B','VTOL_F35B.png',true],['Lux_F22','Lux_F22.png',true],['Lux_Su57','Lux_Su57.png',true],['Lux_MiG31','Lux_MiG31.png',true],['GAA_Shilka_M1','GAA_Shilka_Body.png',false],['GAA_Tunguska_M1','GAA_Tunguska_Body.png',false]];
try {
 await page.evaluateOnNewDocument(()=>localStorage.setItem('HeavyMach.validation-sentinel','preserve-normal'));
 await page.goto(url,{waitUntil:'load'});await page.waitForFunction(()=>window.hmValidation&&window.hmDebugState().screen==='title',{timeout:60000});
 const catalog=await page.evaluate(()=>hmValidation.catalog());report.catalog={weapons:catalog.weapons,stageCount:catalog.stages.length,savePath:catalog.savePath};assert(catalog.savePath==='/validation-persist/HvM.dat','wrong validation save path');assert(catalog.weapons.length===20,'original weapon catalog incomplete');
 const bad=await page.evaluate(()=>hmValidation.start({weapon:'not-a-weapon'}));assert(bad===0,'invalid weapon accepted');assert((await state()).error.length>0,'invalid fixture not reported');
 for(const [name,tile,air] of units){
  assert(catalog.machines.includes(name),'missing machine '+name);
  const start=await page.evaluate(()=>hmValidation.start({weapon:'P_Wpn_Gun06',level:30,grade:3,seed:12345}));assert(start===1,'scenario rejected '+JSON.stringify(await state()));
  await sleep(350);const before=await state();
  const spawned=await page.evaluate(({name,x,y})=>hmValidation.spawn(name,x,y),{name,x:before.game.playerX+210,y:before.game.playerY+(air?90:45)});assert(spawned===1,'spawn rejected '+name);
  await sleep(250);let s=await state();let target=s.enemies.filter(e=>e.name===name).at(-1);assert(target,'enemy missing '+name);const targetID=target.id;const initialHP=target.hp;let minHP=initialHP,maxBullets=0,seenSprite=false,shots=0;let fired=false;
  await page.screenshot({path:path.join(out,name+'-before.png')});
  for(let tick=0;tick<60;tick++){
   s=await state();target=s.enemies.find(e=>e.id===targetID);const sprite=s.sprites.find(d=>d.tile===tile||(tile.includes('Body.png')&&d.tile===tile.replace('Body.png','Body2.png')));
   if(sprite){seenSprite=true;await page.evaluate(({x,y,phase})=>hmValidation.touch(x,y,phase),{x:sprite.x,y:sprite.y,phase:fired?1:0});fired=true;}
   maxBullets=Math.max(maxBullets,s.game.bullets);shots=s.shots;if(target)minHP=Math.min(minHP,target.hp);
   if(minHP<initialHP&&shots>0&&maxBullets>0)break;
   await sleep(80);
  }
  await page.evaluate(()=>hmValidation.touch(0,0,2));s=await state();await page.screenshot({path:path.join(out,name+'-after.png')});
  const row={name,tile,targetID,initialHP,minHP,shots,maxBullets,seenSprite,error:s.error,cheats:s.cheats};report.units.push(row);console.log(JSON.stringify(row));
  assert(!s.error,'validation error '+s.error);assert(seenSprite,'actual sprite not observed '+name);assert(shots>0&&maxBullets>0,'no actual shot/bullet '+name);assert(minHP<initialHP,'no real collision damage '+name);assert(s.cheats.blockedHits===0&&s.cheats.refilledRounds===0,'cheats silently enabled');
 }
 report.storage=await page.evaluate(()=>({normalSentinel:localStorage.getItem('HeavyMach.validation-sentinel'),normalMount:Module.FS.analyzePath('/persist').exists,validationMount:Module.FS.analyzePath('/validation-persist').exists,saveBytes:Module.FS.stat('/validation-persist/HvM.dat').size,prefix:Module.hmPreferencePrefix}));
 assert(report.storage.normalSentinel==='preserve-normal','normal preference modified');assert(!report.storage.normalMount&&report.storage.validationMount,'save roots not isolated');assert(report.storage.prefix==='HeavyMach.Validation.','preference namespace not isolated');
 // Invalid coordinates must be rejected through ValidationScenario, not clamp/teleport.
 assert(await page.evaluate(()=>hmValidation.start({weapon:'P_Wpn_Gun06',position:true,x:1e9,y:1e9}))===0,'out-of-map fixture accepted');
 if(normalURL){const normal=await context.newPage();await normal.goto(normalURL,{waitUntil:'load'});await normal.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:60000});report.normal=await normal.evaluate(()=>({bridge:typeof window.hmValidation,export:typeof Module._hm_validation_start,prefix:Module.hmPreferencePrefix,validation:Module.hmValidation}));assert(report.normal.bridge==='undefined'&&report.normal.export==='undefined'&&report.normal.validation===false,'normal build contains active validation entrypoint');await normal.close();}
 report.ok=report.errors.length===0;
} catch(error){report.errors.push(error.stack||String(error));}
finally {fs.writeFileSync(path.join(out,'report.json'),JSON.stringify(report,null,2)+'\n');await browser.close();}
console.log(JSON.stringify({ok:report.ok,units:report.units.length,errors:report.errors}));if(!report.ok)process.exitCode=1;
