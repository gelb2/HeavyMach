// Natural campaign generation and attack; only the starting position is selected.
// No validationSpawn, forced damage, weapon state, AI or RNG overrides.
import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import puppeteer from 'puppeteer-core';
const url=process.argv[2],out=path.resolve(process.argv[3]||'kamov-armament-campaign');
if(!url)throw Error('Usage: kamov-armament-campaign.mjs validation-url output-directory');
fs.mkdirSync(out,{recursive:true});
const check=(yes,message)=>{if(!yes)throw Error(message);},wait=ms=>new Promise(r=>setTimeout(r,ms));
const mapBytes=fs.readFileSync(new URL('../../Resources/RuntimeData/Stage01.map',import.meta.url));
function node(b,at=0){const kind=b.readUInt32LE(at+4),size=b.readUInt32LE(at+8),payload=b.subarray(at+12,at+12+size);let p=at+12+size,count=b.readUInt32LE(p);p+=4;const children=[];for(let i=0;i<count;i++){const [child,end]=node(b,p);children.push(child);p=end;}return [{kind,payload,children},p];}
const [map]=node(mapBytes),enemies=map.children.find(n=>n.kind===10).children.map(n=>({name:n.payload.subarray(0,24).toString().split('\0')[0],x:n.payload.readFloatLE(24),y:n.payload.readFloatLE(28)}));
const ka=enemies.filter(e=>e.name==='Ka52');check(ka.length===1,'Stage01 must retain one authored Ka52');
const x=ka[0].x-220,heights=[];
for(const poly of map.children.find(n=>n.kind===3).children){const p=poly.children.map(n=>[n.payload.readFloatLE(0),n.payload.readFloatLE(4)]);for(let i=1;i<p.length;i++){const [a,b]=[p[i-1],p[i]];if(a[0]<=x&&x<=b[0]&&b[0]>a[0])heights.push(a[1]+(x-a[0])*(b[1]-a[1])/(b[0]-a[0]));}}
const y=(heights.length?Math.max(...heights):ka[0].y-130)+40;
const report={url,scope:'Stage01 natural MAP generation, original AI, real missile travel and player damage; explicit start position only',fixture:{mission:0,stage:0,level:99,grade:6,seed:12345,x,y},mapSha256:crypto.createHash('sha256').update(mapBytes).digest('hex'),ok:false,errors:[]};
const browser=await puppeteer.launch({executablePath:process.env.CHROME||'/Applications/Google Chrome.app/Contents/MacOS/Google Chrome',headless:true,args:['--enable-unsafe-swiftshader','--autoplay-policy=no-user-gesture-required','--disable-background-timer-throttling'],defaultViewport:{width:1200,height:720}});
try{
 const context=await browser.createBrowserContext(),page=await context.newPage();page.on('pageerror',e=>report.errors.push(e.message));
 await page.goto(url);await page.waitForFunction(()=>window.hmValidation&&hmDebugState().screen==='title',{timeout:60000});report.browser=await browser.version();
 check(await page.evaluate(f=>hmValidation.start({weapon:'P_Wpn_Gun06',position:true,...f}),report.fixture)===1,'Campaign fixture rejected');
 const initial=await page.evaluate(()=>hmValidation.state());report.initial=initial.enemies.filter(e=>e.name==='Ka52');check(report.initial.length===1&&report.initial[0].hp===300,'Natural map Ka52 not created with original HP');
 const frames=[],starts=new Set(),rotors={upper:new Set(),lower:new Set()};let assembled=false,hit=false;
 const deadline=Date.now()+30000;
 while(Date.now()<deadline){
  const s=await page.evaluate(()=>hmValidation.state());frames.push(s);check(!s.sprites.some(d=>d.tile==='Ka52_Cannon.png')&&!s.bullets.some(b=>b.weapon==='Ka52_Cannon'),'Removed cannon in natural campaign');
  check(s.enemies.filter(e=>e.name==='Ka52'&&!e.removed).length===1,'Natural campaign Ka52 count changed');
  for(const b of s.bullets)if(b.toPlayer&&b.weapon==='Ka52_Missile'){check(Number.isFinite(b.startTime),'Missing actual projectile birth observation');starts.add(b.startTime);}
  for(const d of s.sprites){if(d.tile==='Ka52_RotorUpper.png')rotors.upper.add(d.frame);if(d.tile==='Ka52_RotorLower.png')rotors.lower.add(d.frame);}
  assembled||=['Ka52_Body.png','Ka52_RotorUpper.png','Ka52_RotorLower.png','Ka52_Missile.png'].every(t=>s.sprites.some(d=>d.tile===t));
  hit||=s.damage.some(d=>d.target===1&&d.weapon==='Ka52_Missile'&&d.before-d.after===25);
  const times=[...starts].sort((a,b)=>a-b);
  if(times.length>=2&&s.game.time>=4.3&&s.game.time-times[1]>.75&&assembled&&hit&&rotors.upper.size===2&&rotors.lower.size===2){
   const firstBurst=times.filter(t=>t-times[0]<.75);check(firstBurst.length===2,'Natural Ka52 first burst does not contain exactly two missiles');const interval=times[1]-times[0];check(interval>=.499&&interval<=.518,'Natural Ka52 burst spacing differs');
   report.firstBurst={startTimes:firstBurst,interval};report.damage=s.damage.filter(d=>d.weapon==='Ka52_Missile');report.rotorFrames={upper:[...rotors.upper],lower:[...rotors.lower]};report.parts=s.sprites.filter(d=>d.tile.startsWith('Ka52_'));report.cheats=s.cheats;
   check(s.cheats.blockedHits===0&&s.cheats.refilledRounds===0&&s.cheats.fallRescues===0,'Cheats modified campaign combat');
   await page.screenshot({path:path.join(out,'Stage01-Ka52.png')});report.ok=report.errors.length===0;break;
  }
  check(s.game.hp>0,'Player died before campaign observations completed');await wait(16);
 }
 fs.writeFileSync(path.join(out,'trace.jsonl'),frames.map(f=>JSON.stringify(f)).join('\n')+'\n');check(report.ok,'Natural campaign assembly, burst or damage not observed');
}catch(e){report.errors.push(e.stack||String(e));}finally{fs.writeFileSync(path.join(out,'report.json'),JSON.stringify(report,null,2)+'\n');await browser.close();}
console.log(JSON.stringify(report));if(!report.ok)process.exitCode=1;
