// Real WebAssembly/WebGL game, isolated HM_WEB_VALIDATION app and task-owned Chrome.
// Reads frame/weapon/HP evidence; never overrides game RNG, damage, AI or hit results.
// node kamov-eurofighter-validation.mjs validation-url output-dir [normal-url]
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import puppeteer from 'puppeteer-core';

const url=process.argv[2]||'http://127.0.0.1:8891/';
const out=path.resolve(process.argv[3]||'kamov-eurofighter-results');
const normalURL=process.argv[4];
const reuseWeapons=process.env.REUSE_WEAPONS;
const sha=bytes=>crypto.createHash('sha256').update(bytes).digest('hex');
fs.mkdirSync(out,{recursive:true});
const report={url,normalURL,platform:'Chrome WebGL2 / WebAssembly',headless:process.env.HEADFUL!=='1',scope:'Cheat fixture spawning with cheats disabled; real animation, enemy fire, player fire, bullet flight and damage',units:[],weapons:[],errors:[],ok:false};
const check=(condition,message)=>{if(!condition)throw Error(message);};
const wait=ms=>new Promise(resolve=>setTimeout(resolve,ms));
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:report.headless,args:['--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required','--disable-background-timer-throttling','--disable-renderer-backgrounding'],defaultViewport:{width:1200,height:720}});
const context=await browser.createBrowserContext();
const page=await context.newPage();
page.on('pageerror',error=>report.errors.push(error.message));
const state=()=>page.evaluate(()=>hmValidation.state());
const release=()=>page.evaluate(()=>{const p=window.__validationLastTouch;if(p)hmValidation.touch(p.x,p.y,2);});
async function until(predicate,timeout,label){const end=Date.now()+timeout;for(;;){const s=await state();if(predicate(s))return s;if(Date.now()>=end)throw Error(label+'; last='+JSON.stringify({game:s.game,error:s.error}));await wait(40);}}
async function start(){check(await page.evaluate(()=>hmValidation.start({weapon:'P_Wpn_Gun06',level:30,grade:6,seed:12345}))===1,'Scenario start rejected');await until(s=>!s.game.loading&&!s.game.paused&&s.game.hp>0,10000,'Scenario did not run');await wait(4200);}
async function spawn(name){const s=await state();check(await page.evaluate(({name,x,y})=>hmValidation.spawn(name,x,y),{name,x:s.game.playerX+300,y:s.game.playerY+90})===1,'Spawn rejected '+name);return (await until(s=>s.enemies.some(e=>e.name===name),3000,'Spawn missing')).enemies.filter(e=>e.name===name).at(-1);}
async function record(name,id){await page.evaluate(({name,id})=>{
 window.__unitTrace={name,id,frames:[],running:true};
 const step=()=>{const trace=window.__unitTrace;if(!trace.running)return;const s=hmValidation.state();const target=s.enemies.find(e=>e.id===id);trace.frames.push({time:s.game.time,hp:s.game.hp,paused:s.game.paused,gameState:s.game.gameState,shots:s.shots,damage:s.damage.filter(d=>d.time>(trace.lastDamageTime??-1)),target:target||null,bullets:s.bullets,sprites:s.sprites.filter(d=>d.tile.startsWith(name==='Ka52'?'Ka52':'Lux_Euro')||d.tile==='BulletEffect2.png'),cheats:s.cheats});if(s.damage.length)trace.lastDamageTime=s.damage.at(-1).time;requestAnimationFrame(step);};requestAnimationFrame(step);
 },{name,id});}
async function finishTrace(file){const trace=await page.evaluate(()=>{window.__unitTrace.running=false;return window.__unitTrace;});fs.writeFileSync(path.join(out,file),trace.frames.map(row=>JSON.stringify(row)).join('\n')+'\n');return trace.frames;}
function animationEvidence(frames,name){
 const byTile={};for(const f of frames)for(const sprite of f.sprites){const row=byTile[sprite.tile]??={frames:new Set(),scales:new Set(),positions:[]};row.frames.add(sprite.frame);row.scales.add(sprite.scaleX+','+sprite.scaleY);row.positions.push([sprite.x,sprite.y]);}
 return Object.fromEntries(Object.entries(byTile).map(([tile,r])=>[tile,{frames:[...r.frames],scales:[...r.scales],first:r.positions[0],last:r.positions.at(-1)}]));
}
try{
 await page.evaluateOnNewDocument(()=>localStorage.setItem('HeavyMach.validation-sentinel','preserve-normal'));
 await page.goto(url,{waitUntil:'load'});await page.waitForFunction(()=>window.hmValidation&&window.hmDebugState().screen==='title',{timeout:60000});await page.focus('#canvas');
 report.browser=await browser.version();report.userAgent=await page.evaluate(()=>navigator.userAgent);
 const catalog=await page.evaluate(()=>hmValidation.catalog());report.catalog=catalog;
 check(catalog.savePath==='/validation-persist/HvM.dat','Wrong validation save path');check(catalog.weapons.length===20,'Original weapon catalog incomplete');
 check(await page.evaluate(()=>hmValidation.start({weapon:'not-a-weapon'}))===0,'Invalid weapon accepted');
 for(const name of ['Ka52','Lux_Eurofighter']){
  const row={name,ok:false};report.units.push(row);
  try{
   check(catalog.machines.includes(name),'Missing machine '+name);await start();const target=await spawn(name);row.id=target.id;row.initialHP=target.hp;await record(name,target.id);
   await wait(1200);await page.screenshot({path:path.join(out,name+'-assembled.png')});
   const before=await state();row.initialPlayerHP=before.game.hp;
   // Wait for the machine's actual two-way combat. No player shot during this phase.
   const attackEnd=Date.now()+30000;
   for(;;){
    const s=await state();const firing=await page.evaluate(()=>[...new Set(window.__unitTrace.frames.flatMap(f=>f.bullets.filter(b=>b.toPlayer).map(b=>b.weapon)))]);
    row.enemyWeapons=firing;row.minPlayerHP=s.game.hp;
    const prefix=name==='Ka52'?'Ka52':'Lux_Euro',expected=name==='Ka52'?2:1;
    row.enemyDamage=s.damage.filter(d=>d.target===1&&d.weapon.startsWith(prefix)&&d.after<d.before);const hitWeapons=new Set(row.enemyDamage.map(d=>d.weapon));
    const moved=await page.evaluate(()=>{const rows=window.__unitTrace.frames.filter(f=>f.target);if(!rows.length)return false;const first=rows[0].target;return rows.some(f=>Math.hypot(f.target.x-first.x,f.target.y-first.y)>1);});
    if(firing.filter(n=>n.startsWith(prefix)).length>=expected&&hitWeapons.size>=expected&&moved)break;
    check(s.game.hp>0,'Player died before all enemy weapons were observed');check(Date.now()<attackEnd,'Enemy weapon, attributed damage or movement was not observed');await wait(50);
   }
   await page.screenshot({path:path.join(out,name+'-enemy-fire.png')});
   row.attackTrace=name+'-enemy-fire.jsonl';const attackFrames=await finishTrace(row.attackTrace);row.animation=animationEvidence(attackFrames,name);
   const positions=attackFrames.filter(f=>f.target).map(f=>f.target);row.movement={x:Math.max(...positions.map(e=>e.x))-Math.min(...positions.map(e=>e.x)),y:Math.max(...positions.map(e=>e.y))-Math.min(...positions.map(e=>e.y))};check(Math.max(row.movement.x,row.movement.y)>1,'Machine did not move');
   if(name==='Ka52'){
    const upper=Object.entries(row.animation).find(([tile])=>tile.includes('RotorUpper'));
    const lower=Object.entries(row.animation).find(([tile])=>tile.includes('RotorLower'));
    check(upper&&lower,'Both coaxial rotor sprites must be visible');check(upper[1].frames.length>=2&&lower[1].frames.length>=2,'Both rotors must animate');
    const dual=attackFrames.find(f=>f.sprites.some(s=>s.tile===upper[0])&&f.sprites.some(s=>s.tile===lower[0]));const a=dual.sprites.find(s=>s.tile===upper[0]),b=dual.sprites.find(s=>s.tile===lower[0]);row.rotorSeparation=Math.hypot(a.x-b.x,a.y-b.y);check(row.rotorSeparation>0,'Coaxial rotors share the same socket');
    check(Object.keys(row.animation).some(t=>t.includes('Cannon'))&&Object.keys(row.animation).some(t=>t.includes('Missile')),'Cannon or missile mount is invisible');
   }else{check(Object.keys(row.animation).some(t=>t.includes('Missile')),'Eurofighter missile mount is invisible');check(Object.keys(row.animation).includes('BulletEffect2.png'),'Eurofighter exhaust effect not visible');}
   // Pause through the actual keyboard path and compare cached frame/positions.
   await page.keyboard.press('Escape');await until(s=>s.game.paused,3000,'Escape did not pause');const pausedA=await state();await wait(300);const pausedB=await state();row.pauseStable=JSON.stringify(pausedA.sprites)===JSON.stringify(pausedB.sprites)&&JSON.stringify(pausedA.enemies)===JSON.stringify(pausedB.enemies);check(row.pauseStable,'Parts moved/animated while paused');
   // Separate fresh fixture for destruction, with original HP and finite ammunition.
   await start();const victim=await spawn(name);await record(name,victim.id);let pressed=false;const killEnd=Date.now()+25000;
   for(;;){const s=await state();const body=s.sprites.find(d=>d.tile===(name==='Ka52'?'Ka52_Body.png':'Lux_Eurofighter.png'));
    if(body){await page.evaluate(({x,y,phase})=>{window.__validationLastTouch={x,y};return hmValidation.touch(x,y,phase);},{x:body.x,y:body.y,phase:pressed?1:0});pressed=true;}
    const killed=await page.evaluate(()=>window.__unitTrace.frames.some(f=>f.target&&(f.target.hp===0||f.target.removed)));
    if(killed){row.destroyed=true;break;}check(Date.now()<killEnd,'Actual player fire did not destroy '+name);check(s.game.hp>0,'Player died during destruction trial');await wait(35);
   }
   await release();row.damageTrace=name+'-player-fire.jsonl';const damageFrames=await finishTrace(row.damageTrace);row.minTargetHP=Math.min(...damageFrames.filter(f=>f.target).map(f=>f.target.hp));row.playerShots=Math.max(...damageFrames.map(f=>f.shots));
   check(row.playerShots>0&&row.minTargetHP===0,'Missing actual shot/HP-zero evidence');check(damageFrames.every(f=>f.cheats.blockedHits===0&&f.cheats.refilledRounds===0&&f.cheats.fallRescues===0),'Unrequested cheats affected combat');
   await page.screenshot({path:path.join(out,name+'-destroyed.png')});row.ok=true;
  }catch(error){row.error=String(error);report.errors.push(name+': '+error);try{if(await page.evaluate(()=>!!window.__unitTrace?.running))await finishTrace(name+'-failed.jsonl');}catch{}}
  console.log(JSON.stringify(row));
 }
 // Same viewport and logical scale: approved Lux aircraft beside the new one.
 await page.setViewport({width:1800,height:600});await start();let comparisonStart=await state();
 const comparisonNames=['Lux_F22','Lux_Su57','Lux_MiG31','Lux_Eurofighter'];
 for(let i=0;i<comparisonNames.length;i++)check(await page.evaluate(({name,x,y})=>hmValidation.spawn(name,x,y),{name:comparisonNames[i],x:comparisonStart.game.playerX+260+i*145,y:comparisonStart.game.playerY+115})===1,'Comparison spawn rejected');
 await wait(350);const comparison=await state();
 report.liveryComparison={file:'lux-family-same-scale.png',canvas:comparison.game.canvas,order:comparisonNames,sprites:comparison.sprites.filter(d=>comparisonNames.some(n=>d.tile===n+'.png'))};
 check(report.liveryComparison.sprites.length===4,'All four Lux aircraft must be visible together');
 check(report.liveryComparison.sprites.every(d=>d.x>58&&d.x<comparison.game.canvas[0]-58),'Comparison aircraft clipped by viewport');
 await page.screenshot({path:path.join(out,report.liveryComparison.file)});fs.writeFileSync(path.join(out,'lux-family-same-scale.json'),JSON.stringify(report.liveryComparison,null,2)+'\n');
 await start();comparisonStart=await state();const helicopterNames=['Heli001','Heli006','Heli010','Ka52'];
 for(let i=0;i<helicopterNames.length;i++)check(await page.evaluate(({name,x,y})=>hmValidation.spawn(name,x,y),{name:helicopterNames[i],x:comparisonStart.game.playerX+250+i*200,y:comparisonStart.game.playerY+115})===1,'Helicopter comparison spawn rejected '+helicopterNames[i]);
 await wait(350);const heliComparison=await state();
 const heliTiles=['E_Heli_Body_01.png','E_Heli_Body_06.png','E_Heli_Body_10.png','Ka52_Body.png'];
 report.helicopterComparison={file:'helicopter-family-same-scale.png',canvas:heliComparison.game.canvas,order:helicopterNames,sprites:heliComparison.sprites.filter(d=>heliTiles.includes(d.tile))};
 check(report.helicopterComparison.sprites.length===4,'All four helicopter bodies must be visible together');
 await page.screenshot({path:path.join(out,report.helicopterComparison.file)});fs.writeFileSync(path.join(out,'helicopter-family-same-scale.json'),JSON.stringify(report.helicopterComparison,null,2)+'\n');
 await page.setViewport({width:1200,height:720});
 // All original player weapons including three support attacks. Selection and
 // actual launch are checked; this is not all-weapon/all-target hit coverage.
 for(const weapon of reuseWeapons?[]:catalog.weapons){const row={weapon,ok:false};report.weapons.push(row);try{
  check(await page.evaluate(({weapon})=>hmValidation.start({weapon,level:30,grade:weapon==='P_Wpn_Gun01'?0:3,seed:12345}),{weapon})===1,'Weapon fixture rejected');await wait(300);row.initialCharge=(await state()).weaponState.charge;await spawn('Lux_Eurofighter');let pressed=false;const deadline=Date.now()+10000;
  for(;;){const s=await state();const body=s.sprites.find(d=>d.tile==='Lux_Eurofighter.png');if(body){await page.evaluate(({x,y,phase})=>{window.__validationLastTouch={x,y};return hmValidation.touch(x,y,phase);},{x:body.x,y:body.y,phase:pressed?1:0});pressed=true;}row.shots=s.shots;row.lastShot=s.lastShot;if(s.shots>0&&s.lastShot===weapon)break;check(Date.now()<deadline,'Selected weapon did not fire');await wait(50);}
  await release();const ended=await until(s=>s.weaponState.fireCount===0&&!s.weaponState.firing,10000,'Burst did not finish after release');row.shots=ended.shots;row.finalCharge=ended.weaponState.charge;row.chargeUsed=row.initialCharge-row.finalCharge;check(weapon==='P_Wpn_Gun01'?row.chargeUsed===0:row.chargeUsed>0,'Original ammunition consumption missing');row.ok=true;
 }catch(error){row.error=String(error);report.errors.push(weapon+': '+error);}console.log(JSON.stringify(row));}
 if(reuseWeapons){
  const previous=JSON.parse(fs.readFileSync(reuseWeapons,'utf8')),previousBuilds=JSON.parse(fs.readFileSync(path.join(path.dirname(reuseWeapons),'builds.json'),'utf8'));
  check(previous.ok&&previous.weapons.length===20&&previous.weapons.every(w=>w.ok),'Reused weapon evidence is not a complete passing run');
  const currentWasm=sha(Buffer.from(await (await fetch(new URL('index.wasm',url))).arrayBuffer()));
  check(currentWasm===previousBuilds['web-validation'].files['build/web-validation/dist/index.wasm'].sha256,'Weapon reuse requires an identical validation executable');
  for(const file of ['Resources/RuntimeData/manifest.json','tools/prepare_unit_data_authoring.json'])check(sha(fs.readFileSync(new URL('../../'+file,import.meta.url)))===previousBuilds.provenance.sha256[file],'Weapon reuse requires unchanged game data: '+file);
  report.weaponEvidenceReuse={source:path.resolve(reuseWeapons),sourceSha256:sha(fs.readFileSync(reuseWeapons)),weaponsExecuted:0,weaponsReused:20,reason:'Only the Eurofighter body PNG changed; validation WebAssembly and all runtime game data are byte-identical to the previous successful run',validationWasmSha256:currentWasm};
  report.weapons=previous.weapons;
 }
 report.storage=await page.evaluate(()=>({normalSentinel:localStorage.getItem('HeavyMach.validation-sentinel'),normalMount:Module.FS.analyzePath('/persist').exists,validationMount:Module.FS.analyzePath('/validation-persist').exists,saveBytes:Module.FS.stat('/validation-persist/HvM.dat').size,prefix:Module.hmPreferencePrefix}));
 check(report.storage.normalSentinel==='preserve-normal'&&!report.storage.normalMount&&report.storage.validationMount&&report.storage.saveBytes===3624&&report.storage.prefix==='HeavyMach.Validation.','Storage isolation failed');
 check(await page.evaluate(()=>hmValidation.start({weapon:'P_Wpn_Gun06',position:true,x:1e9,y:1e9}))===0,'Out-of-map fixture accepted');
 if(normalURL){const normal=await context.newPage();await normal.goto(normalURL,{waitUntil:'load'});await normal.waitForFunction(()=>window.hmDebugState?.().screen==='title',{timeout:60000});report.normal=await normal.evaluate(()=>({bridge:typeof window.hmValidation,export:typeof Module._hm_validation_start,prefix:Module.hmPreferencePrefix,validation:Module.hmValidation}));check(report.normal.bridge==='undefined'&&report.normal.export==='undefined'&&report.normal.validation===false,'Normal build exposes cheat bridge');await normal.close();}
 report.ok=report.errors.length===0&&report.units.every(r=>r.ok)&&report.weapons.every(r=>r.ok);
}catch(error){report.errors.push(error.stack||String(error));}
finally{fs.writeFileSync(path.join(out,'report.json'),JSON.stringify(report,null,2)+'\n');await browser.close();}
console.log(JSON.stringify({ok:report.ok,units:report.units.length,weapons:report.weapons.length,errors:report.errors}));if(!report.ok)process.exitCode=1;
