const VERSION="01106ecd774f8f15";
const FILES=[{"path": "ASSET-LICENSES.en.md", "bytes": 3510, "sha256": "cbe261672dbf7d8da57210f245e45b2d7a42c47acc37bcb4e9ea998e8a0340f2"}, {"path": "CREDITS.en.md", "bytes": 3686, "sha256": "b054061eb146441840503e30422cc2d4586625c526fdf460fae373019b25bf92"}, {"path": "LICENSE", "bytes": 1070, "sha256": "33ee134b22e5be3986e829d5595d21ff2de970265825c21141c392870ee7a969"}, {"path": "LICENSE-SCOPE.en.md", "bytes": 2612, "sha256": "e2fdbaac055fce52424f242fa69abdcfde36e5ebc996815c16235b434eccd528"}, {"path": "LICENSES/OpenAL-Soft-LGPL-2.0.txt", "bytes": 23330, "sha256": "d808ce217e5b611854da622b57ec29fe545584c48bc5352fae72a4b6e5074a15"}, {"path": "LICENSES/engineering-skills-MIT.txt", "bytes": 1068, "sha256": "6f202f8bd568cd730dbb2b0d1f8e243bc74c2fa1f64dbce9b2c7ea08bd5c9fd7"}, {"path": "LICENSES/rust-sdl2-MIT.txt", "bytes": 1085, "sha256": "df7af208e28b219a02bfa6efb733ed70c96b35e13aa09064c3c999919790dd4c"}, {"path": "LICENSES/stb.txt", "bytes": 2510, "sha256": "bebfe904b14301657e4e5d655c811d51fd31b97c455b9cc2d8600d6bac6cff63"}, {"path": "LICENSES/touchHLE-MPL-2.0.txt", "bytes": 16727, "sha256": "66a3107d5ad6a058aab753eaac2047ccb2ed0e39465dd0fe5844da3e300d5172"}, {"path": "THIRD_PARTY_NOTICES.en.md", "bytes": 4072, "sha256": "027d9b55422c055d3a77bbb09c6d3bee18e5b1ea452b7f9a90f1b94d9585dcc0"}, {"path": "config.js", "bytes": 58, "sha256": "97686acc46342c425dc271c846bf1a3f23f6050d7bf1051a114576923dee767e"}, {"path": "game/fff3cea23a1d2bdf/hm-assets-1.data", "bytes": 20905496, "sha256": "8176c4e73c8cea236f692545d1e9ba707f0a17f689f1ec4034902da960d36a10"}, {"path": "game/fff3cea23a1d2bdf/hm-assets-2.data", "bytes": 20170652, "sha256": "93d5c87fefc1e93427ad101562cce24985327e7b6cc57c5d60fc76e12dda84f8"}, {"path": "game/fff3cea23a1d2bdf/hm-assets-3.data", "bytes": 20262890, "sha256": "45fe34130a1a1eb41f60ae8066ff93345119f513943f05b106af1e30805d3a4f"}, {"path": "game/fff3cea23a1d2bdf/hm-assets-4.data", "bytes": 11579833, "sha256": "330be5fcd2f4f399a859eec467c0536d2be4a4c73d2a4fb7fd2c316e8fb4f284"}, {"path": "game/fff3cea23a1d2bdf/hm-assets.js", "bytes": 95748, "sha256": "15162cf1e31380e1be9975496ff1b74450ff1bd431ad585233dad2d2fa3efdb7"}, {"path": "game/fff3cea23a1d2bdf/hm-assets.json", "bytes": 736, "sha256": "1a1e50cfda81e506f6890ecebd13b402fa7f7be844bc772a8034a06158ea38b1"}, {"path": "game/fff3cea23a1d2bdf/index.html", "bytes": 13627, "sha256": "799419943ead2be04a795732fc476beadfa65f418721c408eb495d46f70cbb6c"}, {"path": "game/fff3cea23a1d2bdf/index.js", "bytes": 126877, "sha256": "5f4636d886a6b3c9ea6fe9fd3ca405386d687b02c2fe0dca42d338948e6418ad"}, {"path": "game/fff3cea23a1d2bdf/index.wasm", "bytes": 710959, "sha256": "aa16a8457561c385c90ec58acc396eb20b5aa36024af5e6d17f2a52a08e35df9"}, {"path": "game/fff3cea23a1d2bdf/manifest.webmanifest", "bytes": 215, "sha256": "65c37e452de3dabbaa9971a8136a51497c8abf1a5034012fd15f56f7eb724e52"}, {"path": "host.css", "bytes": 789, "sha256": "33af2e51ee39f4ff58bbf2ccb8d1de6f1e91e121a7439201169590429d7223e3"}, {"path": "host.js", "bytes": 3374, "sha256": "b7535aef74ff4efdd292b5b183250a73d1026fefb854c503770df794fb0c5908"}, {"path": "icon-192.png", "bytes": 70614, "sha256": "273f52b6f9978647d02ddf5c34db81e9fe2abcb4f4ddddbed912cd0ba573e3fe"}, {"path": "icon-512.png", "bytes": 449608, "sha256": "6c25fc07df943e634ce4d38045a917dd2fad10431dbcd7f927731b499af26392"}, {"path": "index.html", "bytes": 1496, "sha256": "e36cde5d6d15330f1bca43fabf44123a927fcdc7de8f91ff8fd2b1df1b6f1dff"}, {"path": "manifest.webmanifest", "bytes": 320, "sha256": "adc870fb690cf1dfe18687312dbb7124dce759e29494f6868c35de5cb223164c"}, {"path": "saves.js", "bytes": 4961, "sha256": "955318c4b7117876e53f026179ee861fefeaea4bb39af2dabcaf97b934a41ca9"}];
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
