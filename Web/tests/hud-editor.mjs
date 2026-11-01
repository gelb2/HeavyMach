// HUD layout editor check for the HeavyMach WebAssembly build (v0.15 HUD editor).
// Mission selection has no editor entry (upstream #23). Enters stage 1-1, opens
// SET GAME UI from the pause menu, drags LEVEL / EXP, applies, reloads and checks
// the saved layout, presses a moved HUD button, then opens the editor from the
// pause menu again: SAMPLE SCENE, RESET LAYOUT + APPLY restores the original layout.
//
//   node hud-editor.mjs http://127.0.0.1:8765/ out-dir
//
// Prints a JSON report; exit code 1 on failure.
import fs from 'node:fs';
import path from 'node:path';
import puppeteer from 'puppeteer-core';

const url = process.argv[2] || 'http://127.0.0.1:8765/';
const out = process.argv[3] || 'hud-editor-out';
const [vw, vh] = (process.env.VIEWPORT || '912x420').split('x').map(Number);
const chrome = process.env.CHROME || '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
fs.mkdirSync(out, { recursive: true });
const report = { url, steps: [], errors: [], ok: false };
const step = (name, data = {}) => { report.steps.push({ name, ...data }); console.error('step', name, JSON.stringify(data)); };
const sleep = ms => new Promise(r => setTimeout(r, ms));

const browser = await puppeteer.launch({
  executablePath: chrome, headless: true,
  args: ['--enable-unsafe-swiftshader', '--autoplay-policy=no-user-gesture-required', '--disable-background-timer-throttling', '--disable-renderer-backgrounding'],
  defaultViewport: { width: vw, height: vh, deviceScaleFactor: 1 },
});
const context = await browser.createBrowserContext();
let page = await context.newPage();
const watch = p => {
  p.on('pageerror', e => report.errors.push('pageerror: ' + e.message));
  p.on('console', m => { if (m.type() === 'error') report.errors.push('console: ' + m.text()); });
};
watch(page);
const state = () => page.evaluate(() => window.hmDebugState ? window.hmDebugState() : { screen: 'loading' });
async function waitFor(pred, timeout, label) {
  const t0 = Date.now();
  for (;;) { const s = await state(); if (pred(s)) return s; if (Date.now() - t0 > timeout) throw new Error('timeout: ' + label + ' last=' + JSON.stringify(s)); await sleep(100); }
}
const css = async (lx, ly) => { const s = await state(); const [cw, ch] = s.canvas; const k = Math.min(vw / cw, vh / ch); return [(vw - cw * k) / 2 + lx * k, (vh - ch * k) / 2 + (ch - ly) * k]; };
async function tap(lx, ly) { const [x, y] = await css(lx, ly); await page.mouse.move(x, y); await page.mouse.down(); await sleep(60); await page.mouse.up(); }
async function drag(lx, ly, dx, dy) {
  const [x, y] = await css(lx, ly); const [x2, y2] = await css(lx + dx, ly + dy);
  await page.mouse.move(x, y); await page.mouse.down();
  for (let i = 1; i <= 10; i++) { await page.mouse.move(x + (x2 - x) * i / 10, y + (y2 - y) * i / 10); await sleep(30); }
  await page.mouse.up();
}
const gameUI = () => page.evaluate(() => JSON.parse(Module.UTF8ToString(Module._hm_game_ui())));
const gameButton = async label => (await gameUI()).buttons.find(b => b.label === label);
async function clickGame(label) {
  const b = (await gameUI()).buttons.find(b => b.label === label && b.enabled);
  if (!b) throw new Error('no enabled game button ' + label);
  await tap(b.rect[0]+b.rect[2]/2,b.rect[1]+b.rect[3]/2);await sleep(300);
}
const layoutFile = () => page.evaluate(() => { try { return new TextDecoder().decode(Module.FS.readFile('/persist/HUDLayoutV1-web.json')); } catch (e) { return null; } });
const shot = name => page.screenshot({ path: path.join(out, name + '.png') });

async function enterGame() {
  await page.goto(url, { waitUntil: 'load' }); await page.focus('#canvas');
  await waitFor(s => s.screen === 'title', 60000, 'title'); await sleep(7500);
  await tap(347, 160);await page.keyboard.press('F2');await sleep(100);
  if((await gameUI()).keys)throw Error('F2 opened hidden key settings during door loading');
  let s = await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'slots'); await sleep(500);
  const cw = s.canvas[0], my = (s.canvas[1] - 320) / 2, center = cw / 2;
  await tap(cw - 58, 21 + my); await waitFor(s => s.screen === 'menu' && !s.loading && s.time > 0, 15000, 'mode'); await sleep(1500);
  await tap(434 + center - 240, 56 + my); await sleep(1200); await waitFor(s => s.screen === 'menu' && !s.loading, 15000, 'mission'); await sleep(1500);
  return { cw, my, center };
}

try {
  let g = await enterGame();
  // Mission selection has no editor entry (HMHUDEditor updateHUDEntry, v0.15).
  const entryOnMission = !!(await gameButton('SET GAME UI'));
  step('mission-no-entry', { visible: entryOnMission });
  if (entryOnMission) throw new Error('SET GAME UI entry is shown on mission selection');
  await page.evaluate(() => Module._hm_hud_control(-1)); await sleep(300);
  if ((await state()).hudEditing !== 0) throw new Error('editor opened from mission selection');
  await tap(438 + g.center - 240, 26 + g.my);
  let s = await waitFor(s => s.screen === 'game' && !s.loading, 20000, 'game'); await sleep(3000);
  const hudX = (s.canvas[0] - 480) / 2, hudY = (s.canvas[1] - 320) / 2;

  await page.keyboard.press('Escape'); await sleep(1200);
  s = await state(); step('pause', { paused: s.paused, gameMenu: s.gameMenu }); await shot('01-pause-menu');
  await tap(hudX + 240, hudY + 190); await sleep(500);              // SET GAME UI (104) in the pause menu
  s = await state(); step('open-editor', { hudEditing: s.hudEditing, selected: s.hudSelected });
  if (s.hudEditing !== 1) throw new Error('pause-menu SET GAME UI did not open the editor');
  await shot('02-editor');

  // LEVEL / EXP group: legacy (0,293,102,27). Drag its middle 70 units down and 60 right.
  const timeBefore = s.time;
  await drag(hudX + 51, hudY + 306, 60, -70); await sleep(300);
  s = await state(); step('drag', { selected: s.hudSelected, draftCustomized: s.hudDraftCustomized, timeFrozen: s.time === timeBefore });
  if (s.hudSelected !== 1 || s.hudDraftCustomized !== 1) throw new Error('drag did not move LEVEL / EXP');
  await shot('03-dragged');
  await clickGame('TOOLS');                                          // dragging hides the tool panel (iOS _hudHideGuides)
  await clickGame('APPLY');
  s = await state(); const saved = await layoutFile();
  step('apply', { hudEditing: s.hudEditing, customized: s.hudCustomized, file: saved });
  if (s.hudEditing !== 0 || s.hudCustomized !== 1 || !saved) throw new Error('APPLY did not save the layout');
  const positions = JSON.parse(saved).positions;
  if (positions.length !== 10 || !positions[1] || positions.some((p, i) => i !== 1 && p !== null)) throw new Error('unexpected saved positions ' + saved);

  // Resume from the pause menu (RESUME 101 at x≈220,y235) and press MENU (100), which did not move.
  await tap(hudX + 220, hudY + 235); await sleep(1500);
  s = await state(); step('resume', { paused: s.paused, gameMenu: s.gameMenu });
  await shot('04-custom-hud');
  await tap(hudX + 38, hudY + 279); await sleep(800);
  s = await state(); step('menu-button-after-layout', { gameMenu: s.gameMenu, paused: s.paused });
  if (s.gameMenu !== 1) throw new Error('MENU button did not open the pause menu with a custom layout');

  await sleep(1500);
  await page.close(); page = await context.newPage(); watch(page);
  g = await enterGame();
  s = await state(); step('reload', { customized: s.hudCustomized, file: await layoutFile() });
  if (s.hudCustomized !== 1) throw new Error('layout not restored after reload');

  // Pause menu again: SET GAME UI -> SAMPLE SCENE -> RESET LAYOUT -> APPLY.
  await tap(438 + g.center - 240, 26 + g.my);
  await waitFor(s => s.screen === 'game' && !s.loading, 20000, 'game after reload'); await sleep(3000);
  await page.keyboard.press('Escape'); await sleep(1200);
  await tap(hudX + 240, hudY + 190); await sleep(500);
  s = await state(); step('reopen-from-pause', { hudEditing: s.hudEditing });
  if (s.hudEditing !== 1) throw new Error('pause-menu SET GAME UI did not reopen the editor');
  const beforeCancel=await layoutFile();
  await clickGame('PREVIEW');
  if((await gameUI()).buttons.length!==1)throw Error('PREVIEW did not hide tools');
  await clickGame('TOOLS');await clickGame('SAMPLE SCENE');await clickGame('CURRENT SCENE');
  await drag(hudX + 111, hudY + 236, 8, -8);await clickGame('TOOLS');await clickGame('CANCEL');
  if((await state()).hudEditing||await layoutFile()!==beforeCancel)throw Error('CANCEL changed saved layout');
  step('preview-current-scene-and-cancel-draft');
  await tap(hudX + 240, hudY + 190);await sleep(500);
  await clickGame('SAMPLE SCENE'); await sleep(500); await shot('05-sample-scene');
  step('sample-scene', { buttons: (await gameUI()).buttons.map(b=>b.label) });
  await clickGame('RESET LAYOUT'); await clickGame('APPLY');
  s = await state(); const reset = await layoutFile();
  step('reset', { hudEditing: s.hudEditing, customized: s.hudCustomized, file: reset });
  if (s.hudCustomized !== 0 || JSON.parse(reset).positions.some(p => p !== null)) throw new Error('RESET LAYOUT + APPLY did not restore the default');
  await page.focus('#canvas');await page.keyboard.press('F2');await sleep(300);
  if((await gameUI()).keys!==1)throw Error('key bindings did not open');
  for(let i=0;i<16;i++){
    const row=(await gameUI()).buttons.find(b=>b.id===1000+i);await tap(row.rect[0]+row.rect[2]/2,row.rect[1]+row.rect[3]/2);await sleep(30);
    const selected=await gameUI();if(selected.keySelection!==i||!selected.capturing)throw Error('key action selection '+i);
    await page.keyboard.press('Escape');
  }
  step('all-sixteen-key-actions-select-and-cancel');
  let row=(await gameUI()).buttons.find(b=>b.id===1000);await tap(row.rect[0]+row.rect[2]/2,row.rect[1]+row.rect[3]/2);await page.keyboard.press('KeyB');await sleep(100);
  if(!(await gameUI()).buttons.find(b=>b.id===1000).label.endsWith(': B'))throw Error('key remap not applied');
  await clickGame('RESET KEYS');if(!(await gameUI()).buttons.find(b=>b.id===1000).label.endsWith(': A'))throw Error('key reset not applied');
  await clickGame('CLOSE');if((await gameUI()).keys)throw Error('key settings not closed');step('key-remap-reset-and-close');
  report.ok = report.errors.length === 0;
} catch (e) {
  report.errors.push(String(e && e.stack || e));
  try { report.hudUi = await gameUI(); } catch (_) {}
  try { await shot('failure'); } catch (_) {}
}
await browser.close();
console.log(JSON.stringify(report, null, 2));
process.exit(report.ok ? 0 : 1);
