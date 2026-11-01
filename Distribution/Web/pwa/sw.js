// Generated constants are prepended by package.py. Game files remain unchanged.
const base=self.registration.scope;
const PREFIX='heavymach-'+new URL(base).pathname+'-',CACHE=PREFIX+VERSION;
const url=path=>new URL(path,base).href;
const hex=b=>Array.from(new Uint8Array(b),n=>n.toString(16).padStart(2,'0')).join('');
const total=FILES.reduce((n,f)=>n+f.bytes,0);
let progress={type:'progress',version:VERSION,received:0,verified:0,total,phase:'idle'},pending;
async function notify(){for(const client of await self.clients.matchAll({includeUncontrolled:true}))client.postMessage(progress);}
async function putVerified(cache,file,download){
 const cached=await cache.match(url(file.path));
 if(cached){const bytes=await cached.arrayBuffer();if(bytes.byteLength===file.bytes&&hex(await crypto.subtle.digest('SHA-256',bytes))===file.sha256){progress.received+=file.bytes;progress.verified+=file.bytes;await notify();return;}}
 if(!download)throw Error('Missing resource');
 const response=await fetch(url(file.path),{cache:'no-store'});
 if(!response.ok)throw Error('Download failed: '+file.path);
 const reader=response.body?.getReader(),chunks=[];let length=0,last=0;
 if(reader){while(true){const {done,value}=await reader.read();if(done)break;length+=value.length;if(length>file.bytes){await reader.cancel();throw Error('Unexpected download size: '+file.path);}chunks.push(value);progress.received+=value.length;if(Date.now()-last>100){await notify();last=Date.now();}}}
 else{const bytes=new Uint8Array(await response.arrayBuffer());chunks.push(bytes);length=bytes.length;progress.received+=length;}
 const bytes=new Uint8Array(length);let offset=0;for(const chunk of chunks){bytes.set(chunk,offset);offset+=chunk.length;}
 progress.phase='verifying';await notify();
 if(length!==file.bytes||hex(await crypto.subtle.digest('SHA-256',bytes))!==file.sha256)throw Error('Download checksum failed: '+file.path);
 await cache.put(url(file.path),new Response(bytes,{status:200,headers:response.headers}));
 progress.verified+=length;progress.phase='downloading';await notify();
}
function prepare(){
 if(pending)return pending;
 pending=(async()=>{const cache=await caches.open(CACHE);progress={type:'progress',version:VERSION,received:0,verified:0,total,phase:'downloading'};
 try{for(const file of FILES)await putVerified(cache,file,true);await cache.put(url('.offline-ready'),new Response(VERSION));progress.phase='ready';await notify();return {ready:true,version:VERSION};}
 catch(e){progress.phase='failed';progress.error=e.message;await notify();throw e;}
 finally{pending=null;}
 })();return pending;
}
self.addEventListener('install',event=>event.waitUntil((async()=>{
 const cache=await caches.open(CACHE);
 // First visit downloads only the launcher. An existing game gets a complete update.
 let hasGame=false;for(const key of await caches.keys())if(key.startsWith(PREFIX)&&await (await caches.open(key)).match(url('.offline-ready')))hasGame=true;
 if(hasGame)await prepare();else for(const file of FILES.filter(f=>!f.path.startsWith('game/')))await putVerified(cache,file,true);
 await cache.put(url('.shell-ready'),new Response(VERSION));
 // No skipWaiting: open game windows keep their current version.
})()));
self.addEventListener('activate',event=>event.waitUntil((async()=>{
 const cache=await caches.open(CACHE);if(!await cache.match(url('.shell-ready')))throw Error('Incomplete launcher');
 await self.clients.claim();for(const key of await caches.keys())if(key.startsWith(PREFIX)&&key!==CACHE)await caches.delete(key);
})()));
self.addEventListener('fetch',event=>{
 const request=event.request,u=new URL(request.url);if(request.method!=='GET'||!u.href.startsWith(base))return;
 if(request.mode==='navigate'&&request.destination==='document'&&u.pathname.includes('/game/')){event.respondWith(Promise.resolve(Response.redirect(base)));return;}
 event.respondWith((async()=>{const cache=await caches.open(CACHE);const key=u.href===base||u.href===base+'index.html'?url('index.html'):u.href;return await cache.match(key)||fetch(request);})());
});
self.addEventListener('message',event=>{
 if(event.data?.type==='status')event.waitUntil((async()=>{const cache=await caches.open(CACHE);event.ports[0]?.postMessage({...progress,version:VERSION,ready:!!await cache.match(url('.offline-ready'))});})());
 if(event.data?.type==='prepare')event.waitUntil(prepare().then(result=>event.ports[0]?.postMessage(result),e=>event.ports[0]?.postMessage({error:e.message})));
});
