// Generated build constants are prepended by package.py. No game bytes are changed.
const base = self.registration.scope;
const PREFIX = 'heavymach-' + new URL(base).pathname + '-';
const CACHE = PREFIX + VERSION;
const url = path => new URL(path, base).href;
const hex = b => Array.from(new Uint8Array(b), n => n.toString(16).padStart(2,'0')).join('');
self.addEventListener('install', event => event.waitUntil((async () => {
  const cache = await caches.open(CACHE);
  // A worker activates only after every asset has been downloaded and verified.
  for (const file of FILES) {
    const response = await fetch(url(file.path), {cache:'no-store'});
    if (!response.ok) throw new Error('Download failed: '+file.path);
    const bytes = await response.clone().arrayBuffer();
    if (bytes.byteLength !== file.bytes || hex(await crypto.subtle.digest('SHA-256',bytes)) !== file.sha256)
      throw new Error('Download checksum failed: '+file.path);
    await cache.put(url(file.path),response);
  }
  await cache.put(url('.offline-ready'),new Response(VERSION));
  // Do not skipWaiting: a running game keeps its current code and assets.
})()));
self.addEventListener('activate',event=>event.waitUntil((async()=>{
  const current=await caches.open(CACHE);
  if(!await current.match(url('.offline-ready')))throw new Error('Incomplete offline download');
  // Scope prefix avoids touching caches belonging to other applications.
  await self.clients.claim();
  for (const key of await caches.keys()) if(key.startsWith(PREFIX)&&key!==CACHE) await caches.delete(key);
})()));
self.addEventListener('fetch',event=>{
  const request=event.request,u=new URL(request.url);
  if(request.method!=='GET'||!u.href.startsWith(base))return;
  // Direct game bookmarks return through the launcher and its single-window lock.
  if(request.mode==='navigate'&&request.destination==='document'&&u.pathname.includes('/game/')){
    event.respondWith(Promise.resolve(Response.redirect(base)));return;
  }
  event.respondWith((async()=>{
    const cache=await caches.open(CACHE);
    const key=u.href===base||u.href===base+'index.html'?url('index.html'):u.href;
    const hit=await cache.match(key);
    return hit||fetch(request);
  })());
});
self.addEventListener('message',event=>{
  if(event.data?.type==='status')event.waitUntil((async()=>{
    const cache=await caches.open(CACHE);
    event.ports[0]?.postMessage({version:VERSION,ready:!!await cache.match(url('.offline-ready'))});
  })());
});
