# Web Geant4 — Gamma Detector Response in the Browser

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

## How to publish (one-time setup)

1. Install [Git for Windows](https://git-scm.com/download/win) if you don't have it.
2. Create an empty repository on GitHub (e.g. `web-geant4`), **without** a README.
3. In this folder:
   ```
   git init
   git add .
   git commit -m "Web Geant4: gamma detector response simulator"
   git branch -M main
   git remote add origin https://github.com/<your-username>/web-geant4.git
   git push -u origin main
   ```
4. On GitHub: **Settings → Pages → Source: GitHub Actions.**
5. The push triggers the *Build & Deploy* workflow (Actions tab). The **first run
   takes ~1–2 hours** (it compiles Geant4 with Emscripten); the result is cached,
   so later pushes take a few minutes.
6. Your app is live at `https://<your-username>.github.io/web-geant4/`.

## Local development (optional)

Only needed to tweak the UI. Install Node.js, then:

```
cd web
npm install
npm run dev
```

The WASM module must exist in `web/public/wasm/` — download it from a completed
CI run (Actions → artifact) or copy from the deployed site
(`wasm/geant4.mjs`, `wasm/geant4.wasm`, `wasm/geant4.data`).

## Validation

Run the *Validate physics* workflow (Actions tab → run manually). It builds native
Geant4, points it at the slimmed data pack, and runs 7 materials × 7 energies
(59.5 keV–2.6 MeV). If any pruned data file were actually needed, Geant4 would
abort — a completed run proves the pack is sufficient. Reference spectra are
uploaded as artifacts for cross-checking browser results.

## Physics notes & limitations

- Electromagnetic physics only: photoelectric effect, Compton (with Doppler
  broadening, Livermore/low-energy models), Rayleigh, pair production; e± transport
  with bremsstrahlung, ionization, annihilation, Goudsmit–Saunderson MSC.
- Monoenergetic point (isotropic, cone-biased with exact solid-angle bookkeeping)
  or parallel-beam sources. No radioactive decay schemes or coincidence summing (yet).
- Resolution broadening uses generic FWHM(E) models per material
  (`web/src/broaden.ts`) — adjust coefficients to match a specific instrument.
- The aluminum housing is a simple can; no reflector, PMT, or dead layers.

## Roadmap

- Multithreading via `coi-serviceworker` (N× speedup)
- Multi-line sources (Eu-152, Co-60 sum peaks) and simple decay schemes
- Shielding/absorber layer between source and detector
- URL-shareable configurations
