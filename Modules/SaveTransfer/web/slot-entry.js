import {codec} from './transfer.js';
const requestKey='HeavyMach.TransferRequest';
export function atSlots(game){
 const s=game?.hmStorageState?.();
 return !!(game?.Module?._hm_transfer_available?.()&&s?.loaded&&!s.loading&&!s.syncing&&!s.dirty&&!s.blocked);
}
export function bindSlots(ui,game){
 const update=()=>{
  const current=game();
  if(current?.Module?._hm_transfer_state){ui.mount(current);ui.setAvailable(atSlots(current));}
 };
 update();const timer=setInterval(update,150);
 window.addEventListener('pagehide',()=>clearInterval(timer),{once:true});
}
export async function requestRestore(code,options,leave){
 const c=await codec();c.decode(code);
 sessionStorage.setItem(requestKey,JSON.stringify({code,options}));
 leave();
}
export async function consumeRestoreRequest(store){
 const value=sessionStorage.getItem(requestKey);if(!value)return false;
 const {code,options}=JSON.parse(value),c=await codec();c.decode(code);
 await c.restore({...store,pending:async text=>{await store.pending(text);sessionStorage.removeItem(requestKey);}},code,options);
 return true;
}
