// Injected only by Cargo feature=validation. No production backup/storage hooks.
(() => {
 const wait=ms=>new Promise(resolve=>setTimeout(resolve,ms));
 const invoke=(name,args)=>window.__TAURI__.core.invoke(name,args);
 let running=false,closing=false;
 const state=()=>window.hmValidation.state();
 const report={platform:'tauri-wkwebview',units:[],weapons:[],errors:[],ok:false};
 async function emit(phase){report.phase=phase;report.timestamp=new Date().toISOString();await invoke('validation_report',{report:JSON.stringify(report)});}
 const check=(condition,message)=>{if(!condition)throw Error(message);};
 async function ready(){const limit=Date.now()+90000;while(!window.hmValidation||!window.hmDebugState||window.hmDebugState().screen==='loading'){if(Date.now()>limit)throw Error('Validation game did not become ready');await wait(100);}}
 async function foreground(){const limit=Date.now()+90000;while(!document.hasFocus()||document.visibilityState!=='visible'){if(Date.now()>limit)throw Error('Validation window remained inactive; normal gameplay is paused');await wait(100);}}
 async function run(){
  if(running)return;running=true;report.units=[];report.weapons=[];report.errors=[];report.ok=false;
  try{
   await ready();await foreground();report.foregroundAtStart={focused:document.hasFocus(),visibility:document.visibilityState};report.userAgent=navigator.userAgent;report.savePath=hmValidation.catalog().savePath;report.preferencePrefix=Module.hmPreferencePrefix;
   report.mounts={normal:Module.FS.analyzePath('/persist').exists,validation:Module.FS.analyzePath('/validation-persist').exists};
   check(!report.mounts.normal&&report.mounts.validation,'IDBFS mount is not isolated');check(report.savePath==='/validation-persist/HvM.dat','Wrong save path');check(report.preferencePrefix==='HeavyMach.Validation.','Wrong preference namespace');
   let productionBlocked=false;try{await invoke('backup_status');}catch(_){productionBlocked=true;}report.productionCommandsBlocked=productionBlocked;check(productionBlocked,'Production backup command accessible from validation host');
   report.catalog=hmValidation.catalog().weapons;check(report.catalog.length===20,'Original player weapon catalog is incomplete');await emit('started');
   const units=[['VTOL_Yak141','VTOL_Yak141.png',true],['VTOL_Harrier','VTOL_Harrier.png',true],['VTOL_F35B','VTOL_F35B.png',true],['Lux_F22','Lux_F22.png',true],['Lux_Su57','Lux_Su57.png',true],['Lux_MiG31','Lux_MiG31.png',true],['GAA_Shilka_M1','GAA_Shilka_Body.png',false],['GAA_Tunguska_M1','GAA_Tunguska_Body.png',false]];
   for(const [name,tile,air] of units){
    const row={name,tile,ok:false};report.units.push(row);
    try{
     await foreground();check(hmValidation.start({weapon:'P_Wpn_Gun06',level:30,grade:3,seed:12345})===1,'Fixture start failed: '+state().error);await wait(350);const before=state();
     check(hmValidation.spawn(name,before.game.playerX+210,before.game.playerY+(air?90:45))===1,'Fixture spawn failed: '+state().error);await wait(250);
     let s=state(),target=s.enemies.filter(e=>e.name===name).at(-1);check(target,'Enemy missing');const id=target.id;row.initialHP=target.hp;row.minHP=target.hp;row.maxBullets=0;row.seenSprite=false;row.shots=0;let firing=false;
     for(let i=0;i<80;i++){
      await foreground();s=state();target=s.enemies.find(e=>e.id===id);const sprite=s.sprites.find(d=>d.tile===tile||(tile.includes('Body.png')&&d.tile===tile.replace('Body.png','Body2.png')));
      if(sprite){row.seenSprite=true;hmValidation.touch(sprite.x,sprite.y,firing?1:0);firing=true;}
      if(target)row.minHP=Math.min(row.minHP,target.hp);row.maxBullets=Math.max(row.maxBullets,s.game.bullets);row.shots=s.shots;
      if(row.minHP<row.initialHP&&row.shots>0&&row.maxBullets>0)break;await wait(80);
     }
     hmValidation.touch(0,0,2);row.cheats=state().cheats;check(row.seenSprite&&row.shots>0&&row.maxBullets>0&&row.minHP<row.initialHP,'Sprite/shot/bullet/real collision was not observed');check(row.cheats.blockedHits===0&&row.cheats.refilledRounds===0,'Cheats unexpectedly enabled');row.ok=true;
    }catch(error){row.error=String(error);report.errors.push(name+': '+error);}
    await emit('unit-'+name);
   }
   // Every original selectable weapon, including the three Rader specials. This
   // checks selection and actual player fire. It is not an all-target damage proof.
   for(const weapon of report.catalog){
    const row={weapon,fixtureGrade:weapon==='P_Wpn_Gun01'?0:3,ok:false};report.weapons.push(row);
    try{
     await foreground();check(hmValidation.start({weapon,level:30,grade:row.fixtureGrade,seed:12345})===1,'Weapon fixture failed: '+state().error);await wait(300);let s=state();
     check(hmValidation.spawn('Lux_F22',s.game.playerX+180,s.game.playerY+70)===1,'Weapon target spawn failed');await wait(200);row.maxBullets=0;row.shots=0;let firing=false;
     for(let i=0;i<100;i++){
      await foreground();s=state();const sprite=s.sprites.find(d=>d.tile==='Lux_F22.png');if(sprite){hmValidation.touch(sprite.x,sprite.y,firing?1:0);firing=true;}row.maxBullets=Math.max(row.maxBullets,s.game.bullets);row.shots=s.shots;row.lastShot=s.lastShot;
      if(row.shots>0&&row.lastShot===weapon)break;await wait(80);
     }
     hmValidation.touch(0,0,2);check(row.shots>0&&row.lastShot===weapon,'Selected weapon did not fire through the player combat path');row.ok=true;
    }catch(error){row.error=String(error);report.errors.push(weapon+': '+error);}
    await emit('weapon-'+weapon);
   }
   report.saveBytes=Module.FS.stat('/validation-persist/HvM.dat').size;report.ok=report.errors.length===0&&report.units.every(r=>r.ok)&&report.weapons.every(r=>r.ok);
  }catch(error){report.errors.push(String(error));}
  finally{await emit('complete');running=false;}
 }
 async function requestClose(){
  if(closing)return;closing=true;
  try{
   if(window.Module?.hmSyncSave&&window.hmStorageState?.().loaded){await new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(Error('Validation save flush timed out')),10000);Module.hmSyncSave(ok=>{clearTimeout(timer);ok?resolve():reject(Error('Validation save flush failed'));});});}
   await invoke('validation_finish_close');
  }catch(error){closing=false;report.errors.push('close: '+error);await emit('close-failed');}
 }
 window.__hmValidationHost={run,requestClose};
 if(window.__hmValidationAutorun)run().catch(error=>console.error(error));
})();
