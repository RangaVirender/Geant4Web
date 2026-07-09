#!/usr/bin/env bash
# Run the native validation binary against the slimmed data (WSL).
# Usage: bash run_sim.sh <config.json> <nEvents> <out_prefix>
set -uo pipefail
export LD_LIBRARY_PATH=/mnt/d/ubuntu22/geant4-v11.2.1-MTinstall/lib:${LD_LIBRARY_PATH:-}
export G4LEDATA=$HOME/web_geant4/data/slim/G4EMLOW
export G4ENSDFSTATEDATA=$HOME/web_geant4/data/slim/G4ENSDFSTATE
cd "$HOME/web_geant4"
./build-native/app/web-geant4 "$@" > /tmp/sim_stdout.log 2>&1
code=$?
echo "EXITCODE=$code"
grep -E 'G4Exception|Missing|FatalError' /tmp/sim_stdout.log | head -5
tail -2 /tmp/sim_stdout.log
