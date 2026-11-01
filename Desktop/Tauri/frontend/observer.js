(() => {
  'use strict';
  if(!['tauri:','http:'].includes(location.protocol)||!['localhost','tauri.localhost'].includes(location.hostname))return;
  const invoke=(name,args)=>window.__TAURI__.core.invoke(name,args);
  let running=null,closing=false,lastError=null;
  async function backup(){
    if(running)return running;
    running=(async()=>{
      const s=await window.__hmDesktopStorage.committed(window);
      if(s===null)return false;
      await invoke('backup_write',{snapshot:s});lastError=null;return true;
    })();
    try{return await running;}finally{running=null;}
  }
  window.__hmDesktop={
    backup,
    get lastError(){return lastError;},
    async requestClose(){
      if(closing)return;closing=true;
      try{
        if(location.pathname.startsWith('/game/')){
          // Wait for the existing game's save queue. Do not force a new game save.
          let done=false;
          for(let i=0;i<30;i++){if(await backup()){done=true;break;}await new Promise(r=>setTimeout(r,100));}
          if(!done)throw new Error('The game has not finished saving. Keep it open and retry.');
        }
        await invoke('finish_close');
      }catch(e){
        lastError=String(e);console.error('HeavyMach desktop backup:',lastError);
        await invoke('close_error',{message:lastError});
      }
      finally{closing=false;}
    }
  };
  if(location.pathname.startsWith('/game/')){
    setInterval(()=>backup().catch(e=>{lastError=String(e);console.error('HeavyMach desktop backup:',lastError);}),5000);
    window.addEventListener('focus',()=>backup().catch(e=>{lastError=String(e);}));
  }
})();
