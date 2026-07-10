export interface SimConfig {
  material: string;
  shape: string;
  diameter: number;
  height: number;
  boxX: number;
  boxY: number;
  wellDiameter: number;
  wellDepth: number;
  housingThickness: number;
  sourceType: string;
  sourceDistance: number;
  energy: number;
  nBins?: number;
  eMax?: number;
  trackSampleRate?: number;
  maxTrackedEvents?: number;
}

export interface Summary {
  nEvents: number;
  nDeposits: number;
  nFullPeak: number;
  solidAngleFraction: number;
  binWidth: number;
  energy: number;
  intrinsicTotalEff: number;
  intrinsicPeakEff: number;
  absoluteTotalEff: number;
  absolutePeakEff: number;
  peakToTotal: number;
}

export interface TrackStep {
  ev: number;
  tid: number;
  pdg: number;
  p0: [number, number, number];
  p1: [number, number, number];
  proc: string;
}

export interface TracksPayload {
  tracks: TrackStep[];
  geometry: {
    shape: string; diameter: number; height: number; boxX: number; boxY: number;
    wellDiameter: number; wellDepth: number; housingThickness: number; sourceZ: number;
  };
}

export type WorkerMsg =
  | { type: 'ready' }
  | { type: 'load'; loaded: number; total: number }
  | { type: 'progress'; done: number; total: number; spectrum: Float64Array; binWidth: number }
  | { type: 'result'; spectrum: Float64Array; binWidth: number; summary: Summary; tracks: TracksPayload }
  | { type: 'error'; message: string };

export type MainMsg =
  | { type: 'run'; config: SimConfig; nEvents: number }
  | { type: 'abort' };
