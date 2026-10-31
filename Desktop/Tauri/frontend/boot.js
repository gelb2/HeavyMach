(() => {
  const status=document.getElementById('status'),error=document.getElementById('error'),retry=document.getElementById('retry'),start=document.getElementById('start');
  async function prepare(){
    retry.hidden=true;start.hidden=true;error.hidden=true;
    try{
      const backups=await window.__TAURI__.core.invoke('backup_status');
      if(backups.invalid.length)throw new Error('A backup needs recovery. Existing files were kept.\n'+backups.invalid.join('\n'));
      const storage=window.__hmDesktopStorage;
      if(storage.pendingRestore()&&!backups.latest)throw new Error('Restore needs its original backup.');
      if((storage.pendingRestore()||storage.empty(await storage.read()))&&backups.latest){await storage.restoreMissing(backups.latest.snapshot);status.textContent='Saved progress restored.';}
      else status.textContent='Ready to play.';
      start.hidden=false;
    }catch(e){error.textContent=String(e);error.hidden=false;retry.hidden=false;status.textContent='Could not open saved progress.';}
  }
  start.onclick=()=>{location.href='game/index.html';};retry.onclick=prepare;prepare();
})();
