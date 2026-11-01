import {transfer} from './transfer-host.js';
(() => {
 const $=id=>document.getElementById(id),storage=window.__hmDesktopStorage;
 const invoke=(name,args)=>window.__TAURI__.core.invoke(name,args);
 let latest,choice,busy=false;
 function failure(e){$('error').textContent=String(e);$('error').hidden=false;$('retry').hidden=false;$('start').hidden=true;$('status').textContent='Could not open saved progress.';}
 function setBusy(value){busy=value;for(const id of ['start','retry','export','import','recover','apply-restore'])$(id).disabled=value;}
 async function completeRestore(pending){
  const current=await storage.restoreSelected(pending.chosen.snapshot);
  await invoke('restore_finish',{current});storage.finishSelected();
 }
 async function prepare(){
  $('retry').hidden=true;$('start').hidden=true;$('error').hidden=true;setBusy(true);
  try{
   const pending=await invoke('restore_status');if(pending)await completeRestore(pending);
   await transfer.recover();
   const backups=await invoke('backup_status');latest=backups.latest;
   $('recover').hidden=!latest;$('backup-info').textContent=backups.invalid.length?'Damaged backup: '+backups.invalid.join(', ')+'. Import a save or restore the last valid backup.':'Progress is backed up to this computer. Export a copy to keep elsewhere.';
   if(backups.invalid.length){$('recover').closest('details').open=true;throw Error('Choose a backup to recover your progress.');}
   if(storage.pendingRestore()&&!latest)throw Error('Restore needs its original backup.');
   const current=await storage.read();
   if(latest&&!storage.empty(current)&&storage.missing(current,latest.snapshot)){ $('recover').closest('details').open=true;throw Error('Some saved progress or controls are missing. Restore the last backup before playing.');}
   if((storage.pendingRestore()||storage.empty(current))&&latest){await storage.restoreMissing(latest.snapshot);$('status').textContent='Saved progress restored.';}
   else $('status').textContent='Ready to play.';
   $('start').hidden=false;
  }catch(e){failure(e);}finally{setBusy(false);}
 }
 function choose(envelope){choice=envelope;$('confirm-restore').showModal();}
 $('start').onclick=()=>{if(!busy)location.href='game/index.html';};$('retry').onclick=prepare;
 $('export').onclick=async()=>{setBusy(true);try{if(await invoke('export_save',{snapshot:await storage.read()}))$('status').textContent='Save exported.';}catch(e){failure(e);}finally{setBusy(false);}};
 $('import').onclick=async()=>{setBusy(true);try{const selected=await invoke('import_save');if(selected)choose(selected);}catch(e){failure(e);}finally{setBusy(false);}};
 $('recover').onclick=()=>{if(latest)choose(latest);};
 $('cancel-restore').onclick=()=>{choice=null;$('confirm-restore').close();};
 $('apply-restore').onclick=async()=>{if(!choice||busy)return;setBusy(true);$('confirm-restore').close();try{
  const pending=await invoke('restore_begin',{chosen:choice,current:await storage.read()});choice=null;await completeRestore(pending);await prepare();
 }catch(e){failure(e);}finally{setBusy(false);}};
 prepare();
})();
