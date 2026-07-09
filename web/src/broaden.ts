// Detector energy-resolution models and Gaussian broadening of the raw
// energy-deposition histogram (applied client-side; instant re-render).

// FWHM(E) = a * sqrt(E) + b*E  (keV), rough literature values.
const FWHM_MODELS: Record<string, (e: number) => number> = {
  NaI: (e) => 0.92 * Math.sqrt(e) + 0.035 * e,   // ~7.5% @ 662 keV
  CsI: (e) => 1.0 * Math.sqrt(e) + 0.04 * e,     // ~8.5% @ 662
  LaBr3: (e) => 0.45 * Math.sqrt(e) + 0.01 * e,  // ~3% @ 662
  BGO: (e) => 1.3 * Math.sqrt(e) + 0.05 * e,     // ~10% @ 662
  HPGe: (e) => 0.05 * Math.sqrt(e) + 0.0004 * e, // ~1.5 keV @ 662
  CdTe: (e) => 0.3 * Math.sqrt(e) + 0.005 * e,
  PVT: (e) => 2.2 * Math.sqrt(e) + 0.06 * e,     // plastic: very poor resolution
};

export function fwhmAt(material: string, energyKeV: number): number {
  return (FWHM_MODELS[material] ?? FWHM_MODELS.NaI)(energyKeV);
}

export function broaden(raw: Float64Array, binWidth: number, material: string): Float64Array {
  const n = raw.length;
  const out = new Float64Array(n);
  for (let i = 0; i < n; i++) {
    const c = raw[i];
    if (c === 0) continue;
    const e = (i + 0.5) * binWidth;
    const sigma = fwhmAt(material, e) / 2.355 / binWidth; // in bins
    if (sigma < 0.4) { out[i] += c; continue; }
    const halfWidth = Math.ceil(4 * sigma);
    const norm = 1 / (sigma * Math.sqrt(2 * Math.PI));
    for (let j = Math.max(0, i - halfWidth); j <= Math.min(n - 1, i + halfWidth); j++) {
      const d = (j - i) / sigma;
      out[j] += c * norm * Math.exp(-0.5 * d * d);
    }
  }
  return out;
}
