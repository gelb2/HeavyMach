import {codec} from './transfer.js';
// Game buttons are drawn and hit-tested by the original C++ button path.
// This bridge owns only text entry, clipboard/files, and the existing transactions.
export function attachTransfer({store,exclusive,onDone=()=>{},resume,available=()=>true,restoreRequest}){
 let game=null,code=null,file=null,mode=0,busy=false,source=-1,target=-1,settings=true,confirmedCode='',revision=0;
 let status='Create a code or paste one from another device.',ready=false,lastState='';
 const inputStyle='position:fixed;z-index:20;box-sizing:border-box;resize:none;background:#161b17;color:#f0d18a;border:1px solid #827b5c;padding:6px;font:12px monospace;';
 function publish(){
  if(!game?.Module?.ccall)return;
  const state=JSON.stringify([ready,mode,busy,source,target,settings,status]);if(state===lastState)return;lastState=state;
  game.Module.ccall('hm_transfer_state',null,['number','number','number','number','number','number','string'],[+ready,mode,+busy,source,target,+settings,status]);
  if(code){code.hidden=mode!==1;code.readOnly=busy;}
 }
 function mount(current){
  if(game===current)return;
  code?.remove();file?.remove();game=current;lastState='';++revision;
  code=game.document.createElement('textarea');code.className='hm-transfer-code';code.setAttribute('aria-label','Recovery code');code.spellcheck=false;code.autocapitalize='off';code.autocomplete='off';code.maxLength=24000;code.placeholder='Paste an HM1- code';code.style.cssText=inputStyle;code.hidden=true;
  file=game.document.createElement('input');file.type='file';file.accept='.hmcode,text/plain';file.hidden=true;
  for(const event of ['keydown','keyup','pointerdown','pointerup','pointermove','touchstart','touchend'])code.addEventListener(event,e=>e.stopPropagation());
  code.addEventListener('focus',()=>game.Module?._hm_keyboard_blur?.());
  code.addEventListener('keydown',e=>{if(e.key==='Escape'){e.preventDefault();code.blur();game.document.querySelector('#canvas')?.focus();}});
  game.document.body.append(code,file);
  game.Module.hmTransferAction=action;
  game.Module.hmTransferField=(x,y,w,h,visible)=>{if(!code)return;code.hidden=!visible||mode!==1;Object.assign(code.style,{left:x+'px',top:y+'px',width:w+'px',height:h+'px'});};
  file.onchange=()=>run(async c=>{const selected=file.files[0];file.value='';if(!selected)return;if(selected.size>24000)throw Error('Recovery code too long.');code.value=await selected.text();status=c.decode(code.value).summary;});
  publish();
 }
 async function run(fn){
  if(busy)return;busy=true;publish();
  try{if(!available())throw Error('Open save slot selection first.');await exclusive(async()=>fn(await codec()));}
  catch(e){status=e.message||String(e);mode=1;}
  finally{busy=false;publish();}
 }
 function action(id){
  if(busy)return;
  const operation=++revision;
  if(id===900){if(!ready||!available())return;mode=1;status='Create a code or paste one from another device.';publish();return;}
  if(!mode)return;
  if(id===910){mode=0;code.blur();game.document.querySelector('#canvas')?.focus();publish();return;}
  if(id===912){mode=1;publish();return;}
  if(id===905){source=source===2?-1:source+1;publish();return;}
  if(id===906){target=target===2?-1:target+1;publish();return;}
  if(id===907){settings=!settings;publish();return;}
  if(id===902){
   const value=code.value,copyMode=mode;
   const current=()=>revision===operation&&mode===copyMode&&code.value===value;
   const fallback=()=>{if(!current())return;code.focus();code.select();status='Select and copy the code.';publish();};
   if(game.navigator.clipboard?.writeText)game.navigator.clipboard.writeText(value).then(()=>{if(!current())return;status='Code copied.';publish();}).catch(fallback);else fallback();return;
  }
  if(id===904){file.click();return;}
  run(async c=>{
   if(id===901){code.value=c.encode(await store.read());status=c.decode(code.value).summary;}
   if(id===908)status=c.decode(code.value).summary;
   if(id===903){c.decode(code.value);const a=game.document.createElement('a');a.href=URL.createObjectURL(new Blob([code.value],{type:'text/plain'}));a.download='HeavyMach.hmcode';a.click();setTimeout(()=>URL.revokeObjectURL(a.href),1000);status='Code file saved.';}
   if(id===909){status=c.decode(code.value).summary;confirmedCode=code.value;mode=2;code.blur();game.document.querySelector('#canvas')?.focus();}
   if(id===911){
    c.decode(confirmedCode);const options={source,target,settings};
    if(restoreRequest){await restoreRequest(confirmedCode,options);return;}
    await c.restore(store,confirmedCode,options);status='Progress restored.';mode=1;await onDone();
   }
  });
 }
 return {mount,setAvailable(value){ready=!!value;publish();},async recover(){const pending=await resume?.();if(pending){const c=await codec();await c.resume(store,pending);}},show(){action(900);}};
}
