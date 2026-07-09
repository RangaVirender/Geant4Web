#!/usr/bin/env bash
# Native (Linux) build for validation. Builds Geant4 natively (cached in CI),
# then the CLI app. Usage: scripts/build_native.sh
set -euo pipefail

GEANT4_VERSION="${GEANT4_VERSION:-11.3.2}"
ROOT="$PWD"
BUILD="$ROOT/build-native"
INSTALL="$BUILD/geant4-install"
mkdir -p "$BUILD"

SRC="$ROOT/build/geant4-v$GEANT4_VERSION"
if [ ! -d "$SRC" ]; then
  mkdir -p "$ROOT/build"
  curl -L "https://gitlab.cern.ch/geant4/geant4/-/archive/v$GEANT4_VERSION/geant4-v$GEANT4_VERSION.tar.gz" \
    -o "$ROOT/build/geant4.tar.gz"
  tar -xzf "$ROOT/build/geant4.tar.gz" -C "$ROOT/build"
fi

if [ ! -f "$INSTALL/lib/cmake/Geant4/Geant4Config.cmake" ]; then
  mkdir -p "$BUILD/geant4-build"
  cd "$BUILD/geant4-build"
  cmake "$SRC" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$INSTALL" \
    -DGEANT4_INSTALL_DATA=OFF -DGEANT4_USE_GDML=OFF \
    -DGEANT4_BUILD_MULTITHREADED=OFF -DGEANT4_USE_QT=OFF
  make -j"$(nproc)"
  make install
fi

mkdir -p "$ROOT/engine/third_party"
[ -f "$ROOT/engine/third_party/json.hpp" ] || curl -L \
  https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
  -o "$ROOT/engine/third_party/json.hpp"

mkdir -p "$BUILD/app"
cd "$BUILD/app"
cmake "$ROOT/engine" -DCMAKE_BUILD_TYPE=Release -DGeant4_DIR="$INSTALL/lib/cmake/Geant4"
make -j"$(nproc)"
echo "native binary: $BUILD/app/web-geant4"
