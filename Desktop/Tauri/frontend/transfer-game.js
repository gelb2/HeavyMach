import {attachTransfer} from './transfer/ui.js';
import {portable} from './transfer/store.js';
import {atSlots,bindSlots,requestRestore} from './transfer/slot-entry.js';
let busy=false;
const ui=attachTransfer({available:()=>atSlots(window),exclusive:async fn=>{if(busy)throw Error('Wait for the current operation.');busy=true;try{return await fn();}finally{busy=false;}},store:{read:async()=>{const s=await window.__hmDesktopStorage.committed(window);if(!s)throw Error('Wait for saving to finish.');return portable(s);}},restoreRequest:(code,options)=>requestRestore(code,options,()=>location.href='/index.html')});
bindSlots(ui,()=>window);
