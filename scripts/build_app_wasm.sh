#!/usr/bin/env bash
# Build the WASM application (engine/) against the Emscripten Geant4 install
# and pack the slimmed datasets. Output: web/public/wasm/{geant4.mjs,geant4.wasm,geant4.data}
set -euo pipefail

ROOT="$PWD"
BUILD="$ROOT/build"
INSTALL="$BUILD/geant4-wasm-install"
DATA="$ROOT/data/slim"
OUT="$ROOT/web/public/wasm"

source "$BUILD/emsdk/emsdk_env.sh"

# nlohmann/json single header
mkdir -p "$ROOT/engine/third_party"
if [ ! -f "$ROOT/engine/third_party/json.hpp" ]; then
  curl -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
    -o "$ROOT/engine/third_party/json.hpp"
fi

[ -d "$DATA" ] || { echo "run scripts/slim_datasets.py first"; exit 1; }

mkdir -p "$BUILD/app-wasm"
cd "$BUILD/app-wasm"
emcmake cmake "$ROOT/engine" \
  -DCMAKE_BUILD_TYPE=Release \
  -DGeant4_DIR="$INSTALL/lib/cmake/Geant4" \
  -DPTL_DIR="$INSTALL/lib/cmake/Geant4/PTL" \
  -DG4DATA_DIR="$DATA"
emmake make -j"$(nproc)"

mkdir -p "$OUT"
cp geant4.mjs geant4.wasm geant4.data "$OUT/"
ls -lh "$OUT"
