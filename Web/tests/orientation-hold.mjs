// Portrait hold check (touch device): passing through portrait holds the game
// without a pause menu and landscape continues; an interruption during the hold
// (window blur) becomes the ordinary pause with the menu.
//
//   node orientation-hold.mjs http://127.0.0.1:8765/
import puppeteer from 'puppeteer-core';

const url = process.argv[2] || 'http://127.0.0.1:8765/';
const chrome = process.env.CHROME || '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const landscape = { width: 912, height: 420, deviceScaleFactor: 2, isMobile: true, hasTouch: true, isLandscape: true };
const portraitView = { width: 420, height: 912, deviceScaleFactor: 2, isMobile: true, hasTouch: true, isLandscape: false };
const report = { url, steps: [], errors: [], ok: false };
const step = (name, data = {}) => { report.steps.push({ name, ...data }); console.error('step', name, JSON.stringify(data)); };
const sleep = ms => new Promise(r => setTimeout(r, ms));
const browser = await puppeteer.launch({ executablePath: chrome, headless: true,
  args: ['--enable-unsafe-swiftshader', '--disable-background-timer-throttling', '--disable-renderer-backgrounding'], defaultViewport: landscape });
const page = await browser.newPage();
page.on('pageerror', e => report.errors.push('pageerror: ' + e.message));
page.on('console', m => { if (m.type() === 'error') report.errors.push('console: ' + m.text()); });
const state = () => page.evaluate(() => window.hmDebugState ? hmDebugState() : { screen: 'loading' });
async function waitFor(pred, timeout, label) { const t0 = Date.now(); for (;;) { const s = await state(); if (pred(s)) return s; if (Date.now() - t0 > timeout) throw new Error('timeout: ' + label + ' ' + JSON.stringify(s).slice(0, 300)); await sleep(100); } }
async function tap(lx, ly) { const s = await state(); const [cw, ch] = s.canvas; const W = landscape.width, H = landscape.height; const k = Math.min(W / cw, H / ch); await page.touchscreen.tap((W - cw * k) / 2 + lx * k, (H - ch * k) / 2 + (ch - ly) * k); }
const pick = s => ({ active: s.active, hold: s.orientationHold, paused: s.paused, gameMenu: s.gameMenu, resumeRequired: s.resumeRequired, time: s.time });
try {
  await page.goto(url, { waitUntil: 'load' });
  await waitFor(s => s.screen === 'title', 60000, 'title'); await sleep(7500);
  await tap(347, 160); let s = await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'slots'); await sleep(600);
  const my = (s.canvas[1] - 320) / 2, c = s.canvas[0] / 2;
  await tap(s.canvas[0] - 58, 21 + my); await sleep(2500);
  await tap(434 + c - 240, 56 + my); await sleep(3000);
  await tap(438 + c - 240, 26 + my);
  s = await waitFor(s => s.screen === 'game' && !s.loading, 20000, 'game'); await sleep(3000);
  step('game', pick(s));

  await page.setViewport(portraitView); await sleep(800);
  const p1 = await state(); await sleep(1000); const p2 = await state();
  step('portrait', { ...pick(p2), timeFrozen: p1.time === p2.time });
  if (p2.active !== 0 || p2.orientationHold !== 1 || p2.gameMenu !== 0 || p1.time !== p2.time) throw new Error('portrait did not hold the game without a menu');

  await page.setViewport(landscape); await sleep(1500);
  s = await state(); step('landscape', pick(s));
  if (s.active !== 1 || s.orientationHold !== 0 || s.gameMenu !== 0 || s.paused !== 0 || s.time <= p2.time) throw new Error('landscape did not continue the game');

  await page.setViewport(portraitView); await sleep(800);
  await page.evaluate(() => window.dispatchEvent(new Event('blur'))); await sleep(500);
  s = await state(); step('portrait-then-blur', pick(s));
  if (s.gameMenu !== 1 || s.resumeRequired !== 1 || s.orientationHold !== 0) throw new Error('blur during the hold did not become an ordinary pause');
  await page.setViewport(landscape); await sleep(1500);
  s = await state(); step('landscape-after-blur', pick(s));
  if (s.gameMenu !== 1 || s.paused !== 1) throw new Error('ordinary pause did not keep its menu after landscape');
  report.ok = report.errors.length === 0;
} catch (e) { report.errors.push(String(e && e.stack || e)); }
await browser.close();
console.log(JSON.stringify(report, null, 2));
process.exit(report.ok ? 0 : 1);
