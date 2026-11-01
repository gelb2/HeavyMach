import {codec} from './transfer/transfer.js';
import {consumeRestoreRequest} from './transfer/slot-entry.js';
import {portable,stored} from './transfer/store.js';
const storage=window.__hmDesktopStorage;
const invoke=(name,args)=>window.__TAURI__.core.invoke(name,args);
let busy=false;
const exclusive=async fn=>{if(busy)throw Error('Wait for the current save operation.');busy=true;try{return await fn();}finally{busy=false;}};
export const store={read:async()=>portable(await storage.read()),preserve:code=>invoke('transfer_file',{action:'before',code}),pending:code=>invoke('transfer_file',{action:'pending',code}),apply:s=>storage.restoreSelected(stored(s)),finish:async()=>{await invoke('backup_write',{snapshot:await storage.read()});storage.finishSelected();await invoke('transfer_file',{action:'finish',code:''});}};
export const transfer={async recover(){await exclusive(async()=>{const pending=await invoke('transfer_file',{action:'read',code:''});if(pending)await(await codec()).resume(store,pending);await consumeRestoreRequest(store);});}};
