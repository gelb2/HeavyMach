// Browser smoke test for the HeavyMach WebAssembly build.
// Drives a real Chrome through title -> slot -> mode -> mission -> stage 1-1,
// exercises keyboard move/fire/jump, then reloads and checks the IDBFS save.
//
//   cd Web/tests && npm install && node browser-smoke.mjs http://127.0.0.1:8765/ out-dir
//
// Prints a JSON report. Screenshots are written to out-dir. Exit code 1 on failure.
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const url = process.argv[2] || 'http://127.0.0.1:8765/';
const out = process.argv[3] || 'smoke-out';
const touch = process.env.TOUCH === '1'; // phone emulation: coarse pointer + touch taps
const [vw, vh] = (process.env.VIEWPORT || '912x420').split('x').map(Number); // CSS window size
const chrome = process.env.CHROME || '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
fs.mkdirSync(out, { recursive: true });

const report = { url, touch, steps: [], errors: [], ok: false };
const step = (name, data = {}) => { report.steps.push({ name, ...data }); console.error('step', name, JSON.stringify(data)); };
const sleep = ms => new Promise(r => setTimeout(r, ms));

const browser = await puppeteer.launch({
  executablePath: chrome, headless: true,
  args: ['--enable-unsafe-swiftshader', '--autoplay-policy=no-user-gesture-required', '--disable-background-timer-throttling', '--disable-renderer-backgrounding'],
  defaultViewport: { width: vw, height: vh, deviceScaleFactor: touch ? 2 : 1, isMobile: touch, hasTouch: touch, isLandscape: vw > vh },
});
const userData = await browser.createBrowserContext();
let page = await userData.newPage();
const watch = p => {
  p.on('pageerror', e => report.errors.push('pageerror: ' + e.message));
  p.on('console', m => { if (m.type() === 'error') report.errors.push('console: ' + m.text()); });
};
watch(page);

const state = () => page.evaluate(() => window.hmDebugState ? window.hmDebugState() : { screen: 'loading' });
async function waitFor(pred, timeout, label) {
  const t0 = Date.now();
  for (;;) {
    const s = await state();
    if (pred(s)) return s;
    if (Date.now() - t0 > timeout) throw new Error('timeout: ' + label + ' last=' + JSON.stringify(s));
    await sleep(100);
  }
}
// Logical y-up canvas point -> CSS pixel. The canvas is aspect-fit and centred;
// windows taller than 4:3 are letterboxed above and below.
async function tap(lx, ly) {
  const s = await state();
  const [cw, ch] = s.canvas;
  const scale = Math.min(vw / cw, vh / ch);
  const x = (vw - cw * scale) / 2 + lx * scale, y = (vh - ch * scale) / 2 + (ch - ly) * scale;
  if (touch) { await page.touchscreen.touchStart(x, y); await sleep(60); await page.touchscreen.touchEnd(); return; }
  await page.mouse.move(x, y); await page.mouse.down(); await sleep(60); await page.mouse.up();
}
const shot = name => page.screenshot({ path: path.join(out, name + '.png') });

try {
  await page.goto(url, { waitUntil: 'load' });
  await page.focus('#canvas');
  let s = await waitFor(s => s.screen === 'title', 60000, 'title');
  const t0 = s.time, w0 = Date.now();
  await sleep(2000);
  s = await state();
  const fps = (s.time - t0) / 0.01666666753590107 / ((Date.now() - w0) / 1000);
  step('title', { canvas: s.canvas, gameFramesPerSecond: Number(fps.toFixed(1)), callbackInterval: s.callbackInterval, ticksPerCallback: s.ticksPerCallback });
  await sleep(5000); await shot('01-title');

  await tap(347, 160);
  s = await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'slot menu');
  await sleep(500); await shot('02-slots'); step('slots');

  const cw = s.canvas[0];
  // Menus and the HUD keep the 480x320 layout centred in a taller (4:3) canvas.
  const my = (s.canvas[1] - 320) / 2;
  await tap(cw - 58, 21 + my);                       // OK (safe area = full canvas here)
  s = await waitFor(s => s.screen === 'menu' && !s.loading && s.time > 0, 15000, 'mode menu');
  await sleep(1500); await shot('03-mode'); step('mode');

  const center = cw / 2;
  await tap(434 + center - 240, 56 + my);            // Mode OK (scenario is default)
  await sleep(1200);
  s = await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'mission menu');
  await sleep(1500); await shot('04-mission'); step('mission');

  await tap(438 + center - 240, 26 + my);            // Mission OK
  s = await waitFor(s => s.screen === 'game' && !s.loading, 20000, 'game');
  await sleep(4500); s = await state(); await shot('05-game'); step('game-start', s);

  // Fire at the open start position, before driving under the stage-1 tower.
  // Bullets are short-lived; sample during a 1.2 s hold and keep the maximum.
  // The keyboard aim starts at player+100, on the original setTap (0x1d300)
  // 100-unit fire threshold measured from the moving muzzle. Move the aim
  // further right with the aim key first, as a player would, then fire.
  async function fire(label) {
    await page.keyboard.down('ArrowRight'); await sleep(300); await page.keyboard.up('ArrowRight');
    await page.keyboard.down('KeyF');
    let maxBullets = 0;
    for (let i = 0; i < 24; i++) { maxBullets = Math.max(maxBullets, (await state()).bullets); if (i === 10) await shot(label); await sleep(50); }
    await page.keyboard.up('KeyF');
    return maxBullets;
  }
  const startBullets = await fire('06-fire');
  step('fire', { maxBullets: startBullets });
  if (!(startBullets > 0)) throw new Error('no bullets while firing');

  const x0 = (await state()).playerX;
  await page.keyboard.down('KeyD'); await sleep(2500); await page.keyboard.up('KeyD');
  s = await state(); step('move-right', { from: x0, to: s.playerX });
  if (!(s.playerX > x0 + 5)) throw new Error('player did not move right');
  step('fire-after-move', { maxBullets: await fire('06a-fire-after-move'), state: await state() });

  const y0 = (await state()).playerY;
  await page.keyboard.press('Space'); await sleep(350);
  const jump = await state(); step('jump', { from: y0, to: jump.playerY });
  if (!(jump.playerY > y0 + 1)) throw new Error('jump did not raise player');

  // Auto-aim A button: HUD rail rect (334,10,46,46) shifted by the centred HUD offset.
  const hudX = (cw - 480) / 2;
  const aimBefore = (await state()).aimMode;
  await tap(hudX + 357, 33 + my); await sleep(300);
  const aimOn = await state();
  await page.keyboard.down('KeyF');
  let autoFire = await state(), autoBullets = 0;
  for (let i = 0; i < 20; i++) { autoFire = await state(); autoBullets = Math.max(autoBullets, autoFire.bullets); await sleep(50); }
  await shot('06b-auto-aim');
  await page.keyboard.up('KeyF');
  await tap(hudX + 357, 33 + my); await sleep(300);
  const aimOff = await state();
  step('auto-aim', { before: aimBefore, on: aimOn.aimMode, target: autoFire.aimTarget, bulletsWhileFiring: autoBullets, off: aimOff.aimMode });
  if (aimOn.aimMode !== 2 || aimOff.aimMode !== 1) throw new Error('A button did not toggle automatic/manual aim');

  // Pause menu button 100 at HUD (38,279), then "return to menu" 102 writes the save.
  const saveBefore = await page.evaluate(() => { try { return Module.FS.stat('/persist/HvM.dat').size; } catch (e) { return -1; } });
  step('save-after-slot-select', { bytes: saveBefore });
  if (saveBefore !== 3624) throw new Error('slot selection did not write a 3624-byte save');

  await page.keyboard.press('Escape'); await sleep(800); await shot('07-pause');
  const paused = await state(); step('pause', { paused: paused.paused });
  if (!paused.paused) throw new Error('Escape did not pause');

  // Allow the IDBFS sync to finish, then reload with the same browser context.
  await sleep(1500);
  await page.close();
  page = await userData.newPage(); watch(page);
  await page.goto(url, { waitUntil: 'load' });
  await page.focus('#canvas');
  await waitFor(s => s.screen === 'title', 60000, 'title after reload');
  const persisted = await page.evaluate(() => { try { return Module.FS.stat('/persist/HvM.dat').size; } catch (e) { return -1; } });
  step('reload-save', { bytes: persisted });
  if (persisted !== 3624) throw new Error('save not restored from IndexedDB');
  await sleep(6500); await tap(347, 160);
  await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'slot menu after reload');
  await sleep(500); await shot('08-slots-after-reload'); step('slots-after-reload');

  report.ok = report.errors.length === 0;
} catch (e) {
  report.errors.push(String(e && e.stack || e));
  try { await shot('failure'); } catch (_) {}
}
await browser.close();
console.log(JSON.stringify(report, null, 2));
process.exit(report.ok ? 0 : 1);
