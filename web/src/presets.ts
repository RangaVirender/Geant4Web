import type { SimConfig } from './types';

export interface Preset { name: string; config: Partial<SimConfig>; }

export const PRESETS: Preset[] = [
  { name: '3″×3″ NaI, Cs-137 (662 keV)', config: { material: 'NaI', shape: 'cylinder', diameter: 76.2, height: 76.2, energy: 662, sourceType: 'point', sourceDistance: 100 } },
  { name: '2″×2″ NaI, Co-60 (1332 keV)', config: { material: 'NaI', shape: 'cylinder', diameter: 50.8, height: 50.8, energy: 1332.5, sourceType: 'point', sourceDistance: 100 } },
  { name: 'Coax HPGe 60×60, Cs-137', config: { material: 'HPGe', shape: 'cylinder', diameter: 60, height: 60, energy: 662, sourceType: 'point', sourceDistance: 100 } },
  { name: '1.5″×1.5″ LaBr₃, Na-22 (511 keV)', config: { material: 'LaBr3', shape: 'cylinder', diameter: 38.1, height: 38.1, energy: 511, sourceType: 'point', sourceDistance: 50 } },
  { name: 'NaI well counter, Am-241 (59.5 keV)', config: { material: 'NaI', shape: 'well', diameter: 76.2, height: 76.2, wellDiameter: 16, wellDepth: 40, energy: 59.5, sourceType: 'point', sourceDistance: 50 } },
  { name: 'BGO 50×50, Th-232 (2615 keV)', config: { material: 'BGO', shape: 'cylinder', diameter: 50, height: 50, energy: 2614.5, sourceType: 'point', sourceDistance: 100 } },
  { name: 'Plastic slab, beam 662 keV', config: { material: 'PVT', shape: 'box', boxX: 100, boxY: 100, height: 50, energy: 662, sourceType: 'beam' } },
];
