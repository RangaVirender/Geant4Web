#!/usr/bin/env python3
"""Run the native app over a material x energy matrix using the slimmed
datasets. A run that completes proves the slimmed data pack is sufficient;
outputs are kept as reference spectra for comparing against browser runs.
"""
import argparse
import json
import subprocess
from pathlib import Path

MATERIALS = ["NaI", "CsI", "LaBr3", "BGO", "HPGe", "CdTe", "PVT"]
ENERGIES = [59.5, 122.0, 356.0, 662.0, 1173.2, 1332.5, 2614.5]  # keV


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--bin", required=True)
    ap.add_argument("--events", type=int, default=200000)
    ap.add_argument("--out", default="validation/results")
    args = ap.parse_args()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    for mat in MATERIALS:
        for e in ENERGIES:
            cfg = {"material": mat, "energy": e, "sourceType": "point",
                   "sourceDistance": 100.0}
            tag = f"{mat}_{e:.0f}keV"
            cfg_file = out / f"{tag}.json"
            cfg_file.write_text(json.dumps(cfg))
            print(f"=== {tag} ===", flush=True)
            subprocess.run(
                [args.bin, str(cfg_file), str(args.events), str(out / tag)],
                check=True)
            summary = json.loads((out / f"{tag}_summary.json").read_text())
            print(f"  peak eff (abs): {summary['absolutePeakEff']:.4g}  "
                  f"P/T: {summary['peakToTotal']:.3f}")


if __name__ == "__main__":
    main()
