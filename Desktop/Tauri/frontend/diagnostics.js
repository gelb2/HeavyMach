// Opt-in observation only: never replaces the game's frame callback or save API.
(() => {
 let running=false;
 window.__hmMeasure=async()=>{
  if(running||typeof window.hmDebugState!=='function')return;
  running=true;
  const first=window.hmDebugState(),start=performance.now(),intervals=[];let last=start,hidden=0,maxBullets=0;
  const stats=a=>{const b=[...a].sort((x,y)=>x-y);return {samples:a.length,mean_ms:a.reduce((s,n)=>s+n,0)/a.length,p95_ms:b[Math.floor(b.length*.95)],max_ms:Math.max(...b)};};
  await new Promise(resolve=>{function sample(now){intervals.push(now-last);last=now;if(document.hidden)hidden++;maxBullets=Math.max(maxBullets,window.hmDebugState().bullets||0);if(now-start>=15000)resolve();else requestAnimationFrame(sample);}requestAnimationFrame(sample);});
  const end=performance.now(),final=window.hmDebugState(),canvas=document.querySelector('canvas'),gl=canvas?.getContext('webgl2');
  const result={platform:'tauri-wkwebview',seconds:(end-start)/1000,viewport:[innerWidth,innerHeight],pixelRatio:devicePixelRatio,canvas:[canvas?.width,canvas?.height],renderer:gl?.getParameter(gl.RENDERER),raf:stats(intervals.slice(1)),gameTicksPerSecond:(final.time-first.time)/0.01666666753590107/((end-start)/1000),hiddenSamples:hidden,maxBullets,first,final,storage:window.hmStorageState?.()};
  try{await window.__TAURI__.core.invoke('diagnostics_write',{report:JSON.stringify(result)});}finally{running=false;}
 };
})();
