# Geant4Web — Gamma Detector Response in the Browser

Real **Geant4** compiled to **WebAssembly**, simulating the gamma-ray response of
scintillation and semiconductor detectors **entirely in the user's browser**.
Hosted as static files on GitHub Pages — no server, no installation.

**Scope (by design):** gamma sources only, simple geometries (cylinder / box / well),
a curated material list (NaI, CsI, LaBr₃, BGO, HPGe, CdTe, plastic). This restriction
is what makes it feasible to ship Geant4's data: the full ~2 GB of datasets shrinks
to a few tens of MB (G4EMLOW pruned to the needed processes and elements + G4ENSDFSTATE).

## What you get in the browser

- **Pulse-height spectrum** — raw energy deposition and detector-resolution-broadened
  view (photopeak, Compton continuum/edge, escape peaks), lin/log.
- **Efficiency numbers** — absolute & intrinsic full-energy-peak and total efficiency,
  peak-to-total ratio.
- **3D visualization** — three.js view of the geometry with sampled photon/electron
  tracks; interaction points colored by process (Compton / photoelectric / pair).

## Architecture

- `engine/` — C++ Geant4 application: parametric geometry, gamma-only EM physics
  (`G4EmStandardPhysics_option4`), event scoring, track recorder, and an
  [embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html)
  API (`configure` / `run` / `getSpectrum` / `getSummary` / `getTracks`).
  Also builds natively (`native_main.cpp`) for validation.
- `scripts/` — Emscripten toolchain + Geant4 static build, app build (packs the
  slimmed datasets into `geant4.data`), dataset slimming (`slim_datasets.py`).
- `web/` — Vite + TypeScript frontend; the WASM module runs in a **Web Worker**
  so the UI never blocks. Resolution broadening is applied client-side (instant slider).
- `validation/` — native-Geant4 reference runs over a material × energy matrix,
  executed against the *slimmed* data pack to prove it is sufficient.
- `.github/workflows/` — CI builds everything on Linux and deploys to GitHub Pages.
  **You never need Geant4, Emscripten, or Docker on your own machine.**

Single-threaded WASM (GitHub Pages cannot serve the COOP/COEP headers required for
threads); a typical 10⁵-history run takes seconds. A `coi-serviceworker` shim can
enable multithreading later.

## Physics notes & limitations

- Electromagnetic physics only: photoelectric effect, Compton (with Doppler
  broadening, Livermore/low-energy models), Rayleigh, pair production; e± transport
  with bremsstrahlung, ionization, annihilation, Goudsmit–Saunderson MSC.
- Monoenergetic point (isotropic, cone-biased with exact solid-angle bookkeeping)
  or parallel-beam sources. No radioactive decay schemes or coincidence summing (yet).
- Resolution broadening uses generic FWHM(E) models per material
  (`web/src/broaden.ts`) — adjust coefficients to match a specific instrument.
- The aluminum housing is a simple can; no reflector, PMT, or dead layers.
