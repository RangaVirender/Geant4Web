/// <reference lib="webworker" />
// Web Worker hosting the Geant4 WASM module.
import type { MainMsg, SimConfig } from './types';

interface Geant4Module {
  init(): void;
  configure(json: string): void;
  run(nEvents: number, chunkSize: number, progress: (done: number) => void): void;
  getSpectrum(): Float64Array;
  getBinWidth(): number;
  getSummary(): string;
  getTracks(): string;
}

let g4: Geant4Module | null = null;
let aborted = false;
let running = false;

async function load() {
  // geant4.mjs/.wasm/.data live in public/wasm/, copied verbatim by Vite.
  const url = new URL('../wasm/geant4.mjs', self.location.href).href;
  const factory = (await import(/* @vite-ignore */ url)).default;
  g4 = (await factory({
    locateFile: (p: string) => new URL(`../wasm/${p}`, self.location.href).href,
    print: (t: string) => console.log('[geant4]', t),
    printErr: (t: string) => console.warn('[geant4]', t),
  })) as Geant4Module;
  g4.init();
  postMessage({ type: 'ready' });
}

async function run(config: SimConfig, nEvents: number) {
  if (!g4 || running) return;
  running = true;
  aborted = false;
  try {
    g4.configure(JSON.stringify(config));
    const chunk = Math.max(500, Math.min(10000, Math.floor(nEvents / 50)));
    let done = 0;
    while (done < nEvents && !aborted) {
      const n = Math.min(chunk, nEvents - done);
      g4.run(n, n, () => {});
      done += n;
      postMessage({
        type: 'progress', done, total: nEvents,
        spectrum: g4.getSpectrum().slice(), binWidth: g4.getBinWidth(),
      });
      // Yield so an 'abort' message can be processed.
      await new Promise((r) => setTimeout(r, 0));
    }
    postMessage({
      type: 'result',
      spectrum: g4.getSpectrum().slice(),
      binWidth: g4.getBinWidth(),
      summary: JSON.parse(g4.getSummary()),
      tracks: JSON.parse(g4.getTracks()),
    });
  } catch (e) {
    postMessage({ type: 'error', message: String(e) });
  } finally {
    running = false;
  }
}

self.onmessage = (ev: MessageEvent<MainMsg>) => {
  const msg = ev.data;
  if (msg.type === 'run') void run(msg.config, msg.nEvents);
  else if (msg.type === 'abort') aborted = true;
};

void load().catch((e) => postMessage({ type: 'error', message: `load failed: ${e}` }));
