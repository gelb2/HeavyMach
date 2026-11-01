import {atSlots,bindSlots,requestRestore,consumeRestoreRequest} from './transfer/slot-entry.js';
import {attachTransfer} from './transfer/ui.js';
import {portable,stored} from './transfer/store.js';
import {GAME_URL} from './config.js';
import * as saves from './saves.js';
const $=id=>document.getElementById(id),status=$('status'),error=$('error');
const standalone=()=>matchMedia('(display-mode: standalone)').matches||navigator.standalone===true;
const transferStore={read:async()=>portable(await saves.read()),preserve:async code=>{localStorage.setItem('HeavyMach.TransferBefore',code);},pending:async code=>{localStorage.setItem('HeavyMach.TransferPending',code);},apply:async s=>saves.apply(stored(s)),finish:async()=>localStorage.removeItem('HeavyMach.TransferPending')};
const gameWindow=()=> $('game').hidden?null:$('game').contentWindow;
let transferBusy=false;
const transfer=attachTransfer({parent:document.body,available:()=>atSlots(gameWindow()),exclusive:async fn=>{if(transferBusy)throw Error('Wait for the current operation.');transferBusy=true;try{return await fn();}finally{transferBusy=false;}},store:transferStore,resume:async()=>localStorage.getItem('HeavyMach.TransferPending'),restoreRequest:(code,options)=>requestRestore(code,options,()=>location.reload())});
bindSlots(transfer,gameWindow);

let registration,offlineReady=false,busy=false,installPrompt,activeVersion;
const fail=e=>{error.textContent=e.message||String(e);error.hidden=false;$('retry').hidden=false;};
async function exclusive(fn){
 if(!navigator.locks)throw Error('Please update your browser to play.');
 return navigator.locks.request('HeavyMach-save-owner',{ifAvailable:true},async lock=>{if(!lock)throw Error('The game is open in another window. Close it and try again.');return fn();});
}
function message(worker,type,timeout=10000){return new Promise((resolve,reject)=>{
 const channel=new MessageChannel(),timer=setTimeout(()=>{channel.port1.close();reject(Error('Download stopped. Try again.'));},timeout);
 channel.port1.onmessage=e=>{clearTimeout(timer);channel.port1.close();type==='prepare'&&e.data.error?reject(Error(e.data.error)):resolve(e.data);};worker.postMessage({type},[channel.port2]);
});}
const mb=n=>(n/1048576).toFixed(1)+' MB';
function showProgress(data){
 if(!busy||data.version!==activeVersion)return;
 $('progress').hidden=false;$('progress-label').hidden=false;
 $('progress').max=data.total;$('progress').value=data.verified;
 const percent=data.phase==='ready'?100:Math.min(99,Math.floor(100*data.verified/data.total));
 $('progress-label').textContent=mb(data.received)+' / '+mb(data.total)+' · '+percent+'% verified';
 status.textContent=data.phase==='verifying'?'Checking game files…':data.phase==='ready'?'Ready to play offline.':'Downloading game…';
}
function readyLabel(){status.textContent=offlineReady?'Ready to play offline.':'Install the game, or play in this browser.';}
async function prepare(){
 if(offlineReady)return;
 busy=true;$('play').disabled=true;error.hidden=true;$('retry').hidden=true;
 try{const worker=registration.active;const result=await message(worker,'status');activeVersion=result.version;
 if(result.ready){offlineReady=true;return;}
 showProgress({...result,phase:'downloading'});
 await message(worker,'prepare',600000);offlineReady=true;
 }finally{busy=false;$('play').disabled=false;if(offlineReady)readyLabel();}
}
async function initialize(){
 await exclusive(async()=>{await saves.recover();await transfer.recover();await consumeRestoreRequest(transferStore);});
 registration=await navigator.serviceWorker.register('./sw.js',{scope:'./',updateViaCache:'none'});
 registration.addEventListener('updatefound',()=>{const worker=registration.installing;worker.addEventListener('statechange',()=>{
 if(worker.state==='installed'&&navigator.serviceWorker.controller)status.textContent='Update ready. Close all game windows to use it next time.';
 if(worker.state==='redundant')fail(Error('Update download failed. You can keep playing this version.'));
 });});
 const worker=(await navigator.serviceWorker.ready).active;
 const result=await message(worker,'status');activeVersion=result.version;offlineReady=result.ready;
 readyLabel();
 if(standalone())await exclusive(()=>prepare());
 $('play').disabled=false;
}
$('play').onclick=()=>exclusive(async()=>{
 await saves.recover();await transfer.recover();await consumeRestoreRequest(transferStore);await prepare();
 navigator.storage?.persist?.().catch(()=>{});
 const frame=$('game');frame.src=GAME_URL;frame.hidden=false;$('launcher').hidden=true;document.body.classList.add('playing');
 frame.onload=()=>{frame.contentWindow.focus();frame.contentDocument?.getElementById('canvas')?.focus();};
 await new Promise(()=>{});
}).catch(fail);
$('retry').onclick=()=>registration?.active?exclusive(()=>prepare()).catch(fail):location.reload();
function installState(){const installed=standalone();$('install').hidden=installed;$('install-storage').hidden=installed;$('play').classList.toggle('primary',installed);$('play').textContent=installed?'Play':'Play in browser';if(installed)$('intro').textContent='Download once, then play offline. Updates download when you are online.';}
$('install').onclick=async()=>{
 if(installPrompt){const prompt=installPrompt;installPrompt=null;await prompt.prompt();const choice=await prompt.userChoice;if(choice.outcome==='accepted')status.textContent='Open Heavy Mach from its new icon.';return;}
 const appleMobile=/iPhone|iPad|iPod/.test(navigator.userAgent)||(navigator.platform==='MacIntel'&&navigator.maxTouchPoints>1);
 $('install-help').textContent=appleMobile?'In Safari, tap Share, then Add to Home Screen. Leave Open as Web App on and tap Add.':/Safari/.test(navigator.userAgent)&&!/Chrome|Chromium|Edg/.test(navigator.userAgent)?'In Safari, choose File → Add to Dock.':'Open your browser menu and choose Install Heavy Mach or Install this page as an app.';
 $('install-dialog').showModal();
};
window.addEventListener('beforeinstallprompt',e=>{e.preventDefault();installPrompt=e;});
window.addEventListener('appinstalled',()=>{installPrompt=null;status.textContent='Open Heavy Mach from its new icon.';});
window.addEventListener('pageshow',e=>{if(e.persisted)location.reload();});
installState();
if(!('serviceWorker' in navigator))fail(Error('Open this game over HTTPS in a current browser.'));else{
 navigator.serviceWorker.addEventListener('message',e=>{if(e.data?.type==='progress')showProgress(e.data);});
 initialize().catch(fail);
}
