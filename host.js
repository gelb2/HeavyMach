import {GAME_URL} from './config.js';
import * as saves from './saves.js';
const $=id=>document.getElementById(id),status=$('status'),error=$('error');
const fail=e=>{error.textContent=e.message||String(e);error.hidden=false;};
let release,registration;
async function exclusive(fn){
 if(!navigator.locks)throw Error('This browser needs Web Locks support. Please update your browser.');
 return navigator.locks.request('HeavyMach-save-owner',{ifAvailable:true},async lock=>{if(!lock)throw Error('The game is open in another window. Close it and try again.');return fn();});
}
async function workerStatus(worker){return new Promise((resolve,reject)=>{const c=new MessageChannel(),timeout=setTimeout(()=>reject(Error('Offline check timed out')),10000);c.port1.onmessage=e=>{clearTimeout(timeout);resolve(e.data);};worker.postMessage({type:'status'},[c.port2]);});}
async function ready(){
 await exclusive(()=>saves.recover());
 registration=await navigator.serviceWorker.register('./sw.js',{scope:'./',updateViaCache:'none'});
 registration.addEventListener('updatefound',()=>{const installing=registration.installing;installing.addEventListener('statechange',()=>{if(installing.state==='installed'&&navigator.serviceWorker.controller)status.textContent='Update downloaded. Close all game windows to use it next time.';if(installing.state==='redundant')fail(Error('Download failed. Reopen this page to retry.'));});});
 const active=(await navigator.serviceWorker.ready).active;
 const result=await workerStatus(active);if(!result.ready)throw Error('Offline download is incomplete');
 status.textContent='Ready. You can play offline.';
 $('play').disabled=false;$('export').disabled=false;$('import').disabled=false;
}
$('play').onclick=()=>exclusive(async()=>{
 await saves.recover();
 // Browser decides whether to grant persistent storage. A denial does not block play.
 navigator.storage?.persist?.().catch(()=>{});
 const frame=$('game');frame.src=GAME_URL;frame.hidden=false;$('launcher').hidden=true;
 frame.onload=()=>{frame.contentWindow.focus();frame.contentDocument?.getElementById('canvas')?.focus();};
 await new Promise(r=>{release=r;});
}).catch(fail);
$('export').onclick=()=>exclusive(async()=>{await saves.recover();saves.download(await saves.encode(await saves.read()));}).catch(fail);
$('import').onchange=async event=>{const file=event.target.files[0];event.target.value='';if(!file)return;
 try{if(file.size>262144)throw Error('Save file is too large');const text=await file.text();await saves.decode(text);
 if(!confirm('Replace saved progress with this file? A copy of your current progress will be kept.'))return;
 await exclusive(async()=>{await saves.restore(text);const old=saves.previous();if(old)saves.download(old,'HeavyMach-before-import.json');});status.textContent='Save imported. Ready to play.';
 }catch(e){fail(e);}
};
window.addEventListener('beforeinstallprompt',e=>{e.preventDefault();$('install').hidden=false;$('install').onclick=async()=>{await e.prompt();$('install').hidden=true;};});
window.addEventListener('pagehide',()=>{release?.();});
// The iframe owns game pause/resume handling. No game functions or saves are replaced.
if(!('serviceWorker' in navigator))fail(Error('Open this game over HTTPS in a current browser.'));else ready().catch(fail);
