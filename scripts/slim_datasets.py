#!/usr/bin/env python3
"""Download G4EMLOW + G4ENSDFSTATE and prune them to gamma-only EM physics
for the elements present in our material list. Output: data/slim/.

Usage: python3 scripts/slim_datasets.py [--out data/slim]
"""
import argparse
import re
import shutil
import tarfile
import urllib.request
from pathlib import Path

G4EMLOW = ("https://cern.ch/geant4-data/datasets/G4EMLOW.8.6.1.tar.gz", "G4EMLOW8.6.1")
G4ENSDF = ("https://cern.ch/geant4-data/datasets/G4ENSDFSTATE.3.0.tar.gz", "G4ENSDFSTATE3.0")

# Elements in detector materials (NaI, CsI, LaBr3, BGO, HPGe, CdTe, PVT),
# housing (Al), air, and common shielding for future use (Pb, W, Fe, Cu, Sn).
ALLOWED_Z = {1, 6, 7, 8, 11, 13, 18, 26, 29, 32, 35, 48, 50, 52, 53, 55, 57, 74, 82, 83}

# G4EMLOW subdirectories used by G4EmStandardPhysics_option4 (gamma + e-/e+):
# photoelectric, Compton, Rayleigh, pair, bremsstrahlung (Seltzer-Berger),
# atomic deexcitation (fluor/auger), Goudsmit-Saunderson MSC, Livermore/Penelope
# shared tables. Anything not listed here is dropped entirely.
KEEP_DIRS = [
    "epics2017/phot", "epics2017/comp", "epics2017/rayl", "epics2017/pair",
    "comp", "doppler", "livermore", "penelope", "brem", "brem_SB",
    "photoelectric_angular", "ioni",
    "fluor", "fluor_Bearden", "fluor_XDB_EADL", "auger", "msc_GS",
    # option4 builds ion ionisation tables at init (G4IonICRU73Data) even though
    # no ions are ever tracked in a gamma run; missing files segfault Geant4.
    "ion_stopping_data",
]

# Directories whose loaders iterate over ALL Z at init (no per-element pruning):
# G4AugerData / G4FluoData load every element up to Z=100.
PRUNE_EXEMPT = {"fluor", "fluor_Bearden", "fluor_XDB_EADL", "auger",
                "ion_stopping_data",
                # G4DopplerProfile loads profiles for all Z at init
                "doppler",
                # msc_GS file numbering is a grid index, not Z
                "msc_GS"}

# Per-element files look like "pe-cs-53.dat", "re-ss-cs-53.dat", "br53" etc.
Z_PATTERN = re.compile(r"[-._]?(\d{1,3})(?:\.dat|\.bin|\.ascii|\.txt|)$")


def fetch(url: str, dest: Path) -> Path:
    dest.mkdir(parents=True, exist_ok=True)
    tgz = dest / url.split("/")[-1]
    if not tgz.exists():
        print(f"downloading {url}")
        urllib.request.urlretrieve(url, tgz)
    print(f"extracting {tgz.name}")
    with tarfile.open(tgz) as tf:
        tf.extractall(dest)
    return dest


def keep_file(path: Path) -> bool:
    m = Z_PATTERN.search(path.name)
    if not m:
        return True  # non per-element file (headers, grids, binding energies...)
    z = int(m.group(1))
    if z > 100:
        return True  # not an atomic number (e.g. energy grid label)
    return z in ALLOWED_Z


def slim_emlow(src: Path, dst: Path) -> None:
    kept = total = 0
    # top-level loose files are always kept
    for f in src.iterdir():
        if f.is_file():
            shutil.copy2(f, dst / f.name)
    for sub in KEEP_DIRS:
        d = src / sub
        if not d.is_dir():
            continue
        for f in d.rglob("*"):
            if not f.is_file():
                continue
            total += 1
            if sub in PRUNE_EXEMPT or keep_file(f):
                kept += 1
                rel = f.relative_to(src)
                out = dst / rel
                out.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(f, out)
    print(f"G4EMLOW: kept {kept}/{total} files in {len(KEEP_DIRS)} dirs")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="data/slim")
    ap.add_argument("--cache", default="data/cache")
    args = ap.parse_args()
    out = Path(args.out)
    cache = Path(args.cache)
    if out.exists():
        shutil.rmtree(out)

    fetch(G4EMLOW[0], cache)
    emlow_dst = out / "G4EMLOW"
    emlow_dst.mkdir(parents=True)
    slim_emlow(cache / G4EMLOW[1], emlow_dst)

    fetch(G4ENSDF[0], cache)
    shutil.copytree(cache / G4ENSDF[1], out / "G4ENSDFSTATE")

    size = sum(f.stat().st_size for f in out.rglob("*") if f.is_file())
    print(f"slimmed dataset size: {size / 1e6:.1f} MB -> {out}")


if __name__ == "__main__":
    main()
