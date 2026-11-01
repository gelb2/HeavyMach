import {codec} from './transfer.js';
const requestKey='HeavyMach.TransferRequest';
export function atSlots(game){
 const s=game?.hmStorageState?.();
 return !!(game?.Module?._hm_transfer_available?.()&&s?.loaded&&!s.loading&&!s.syncing&&!s.dirty&&!s.blocked);
}
export function bindSlots(ui,game){
 const update=()=>{ui.button.hidden=!atSlots(game());};
 update();const timer=setInterval(update,150);
 window.addEventListener('pagehide',()=>clearInterval(timer),{once:true});
 ui.button.style.cssText='position:fixed;right:max(12px,env(safe-area-inset-right));top:max(12px,env(safe-area-inset-top));z-index:30';
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
