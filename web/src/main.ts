import { broaden } from './broaden';
import { SpectrumPlot } from './plot';
import { PRESETS } from './presets';
import type { SimConfig, Summary, WorkerMsg } from './types';
import { View3D } from './view3d';

const $ = <T extends HTMLElement>(id: string) => document.getElementById(id) as T;

const worker = new Worker(new URL('./worker.ts', import.meta.url), { type: 'module' });
const plot = new SpectrumPlot($('spectrum') as HTMLCanvasElement);
const view = new View3D($('view3d') as HTMLCanvasElement);

let rawSpectrum: Float64Array | null = null;
let binWidth = 1;
let showBroadened = true;

function readConfig(): SimConfig {
  const num = (id: string) => parseFloat(($(id) as HTMLInputElement).value) || 0;
  const sel = (id: string) => ($(id) as HTMLSelectElement).value;
  return {
    material: sel('material'),
    shape: sel('shape'),
    diameter: num('diameter'),
    height: num('height'),
    boxX: num('boxX'),
    boxY: num('boxY'),
    wellDiameter: num('wellDiameter'),
    wellDepth: num('wellDepth'),
    housingThickness: num('housing'),
    sourceType: sel('sourceType'),
    sourceDistance: num('distance'),
    energy: num('energy'),
    eMax: num('eMax'),
    nBins: 2048,
    trackSampleRate: 200,
    maxTrackedEvents: 40,
  };
}

function applyPreset(p: Partial<SimConfig>) {
  const set = (id: string, v: unknown) => {
    if (v !== undefined) ($(id) as HTMLInputElement).value = String(v);
  };
  set('material', p.material); set('shape', p.shape); set('diameter', p.diameter);
  set('height', p.height); set('boxX', p.boxX); set('boxY', p.boxY);
  set('wellDiameter', p.wellDiameter); set('wellDepth', p.wellDepth);
  set('energy', p.energy); set('sourceType', p.sourceType); set('distance', p.sourceDistance);
  onShapeChange();
  view.setGeometry(readConfig());
}

function onShapeChange() {
  const shape = ($('shape') as HTMLSelectElement).value;
  $('cylDims').style.display = shape === 'box' ? 'none' : 'flex';
  $('boxDims').style.display = shape === 'box' ? 'flex' : 'none';
  $('wellDims').style.display = shape === 'well' ? 'flex' : 'none';
}

function redraw() {
  if (!rawSpectrum) return;
  const data = showBroadened
    ? broaden(rawSpectrum, binWidth, ($('material') as HTMLSelectElement).value)
    : rawSpectrum;
  plot.set(data, binWidth);
}

function showSummary(s: Summary) {
  const pct = (v: number) => (100 * v).toPrecision(3) + '%';
  $('summary').innerHTML =
    `histories: <b>${s.nEvents.toLocaleString()}</b><br>` +
    `full-energy peak eff (abs): <b>${pct(s.absolutePeakEff)}</b><br>` +
    `total eff (abs): <b>${pct(s.absoluteTotalEff)}</b><br>` +
    `intrinsic peak eff: <b>${pct(s.intrinsicPeakEff)}</b><br>` +
    `peak-to-total: <b>${s.peakToTotal.toFixed(3)}</b><br>` +
    `solid angle / 4π: <b>${s.solidAngleFraction.toExponential(2)}</b>`;
}

worker.onmessage = (ev: MessageEvent<WorkerMsg>) => {
  const msg = ev.data;
  const runBtn = $('runBtn') as HTMLButtonElement;
  if (msg.type === 'ready') {
    runBtn.disabled = false;
    runBtn.textContent = 'Run simulation';
    $('status').textContent = 'Geant4 initialized.';
  } else if (msg.type === 'progress') {
    ($('prog') as HTMLProgressElement).value = msg.done / msg.total;
    $('status').textContent = `running… ${msg.done.toLocaleString()} / ${msg.total.toLocaleString()}`;
    rawSpectrum = msg.spectrum;
    binWidth = msg.binWidth;
    redraw();
  } else if (msg.type === 'result') {
    rawSpectrum = msg.spectrum;
    binWidth = msg.binWidth;
    redraw();
    showSummary(msg.summary);
    view.setTracks(msg.tracks);
    finishRun('done.');
  } else if (msg.type === 'error') {
    finishRun(`error: ${msg.message}`);
  }
};

function finishRun(status: string) {
  $('status').textContent = status;
  ($('runBtn') as HTMLButtonElement).disabled = false;
  $('abortBtn').style.display = 'none';
  ($('prog') as HTMLProgressElement).style.display = 'none';
}

$('runBtn').addEventListener('click', () => {
  const config = readConfig();
  view.setGeometry(config);
  ($('runBtn') as HTMLButtonElement).disabled = true;
  $('abortBtn').style.display = 'block';
  const prog = $('prog') as HTMLProgressElement;
  prog.style.display = 'block';
  prog.value = 0;
  $('status').textContent = 'configuring…';
  worker.postMessage({
    type: 'run', config,
    nEvents: parseInt(($('nEvents') as HTMLSelectElement).value, 10),
  });
});
$('abortBtn').addEventListener('click', () => worker.postMessage({ type: 'abort' }));
$('shape').addEventListener('change', onShapeChange);

const presetSel = $('preset') as HTMLSelectElement;
presetSel.innerHTML = PRESETS.map((p, i) => `<option value="${i}">${p.name}</option>`).join('');
presetSel.addEventListener('change', () => applyPreset(PRESETS[+presetSel.value].config));

for (const [btn, other, fn] of [
  ['linBtn', 'logBtn', () => { plot.logY = false; }],
  ['logBtn', 'linBtn', () => { plot.logY = true; }],
  ['rawBtn', 'broadBtn', () => { showBroadened = false; }],
  ['broadBtn', 'rawBtn', () => { showBroadened = true; }],
] as [string, string, () => void][]) {
  $(btn).addEventListener('click', () => {
    (fn as () => void)();
    $(btn).classList.add('active');
    $(other).classList.remove('active');
    redraw();
    plot.draw();
  });
}

applyPreset(PRESETS[0].config);
$('status').textContent = 'loading WASM module + datasets…';
