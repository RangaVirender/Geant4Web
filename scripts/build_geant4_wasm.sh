#!/usr/bin/env bash
# Build Geant4 static libraries with Emscripten. Run inside a Linux environment
# (GitHub Actions). Produces install at $PWD/build/geant4-wasm-install.
set -euo pipefail

GEANT4_VERSION="${GEANT4_VERSION:-11.3.2}"
EMSDK_VERSION="${EMSDK_VERSION:-3.1.61}"
JOBS="${JOBS:-$(nproc)}"
ROOT="$PWD"
BUILD="$ROOT/build"
mkdir -p "$BUILD"

# --- emsdk ---
if [ ! -d "$BUILD/emsdk" ]; then
  git clone --depth 1 https://github.com/emscripten-core/emsdk.git "$BUILD/emsdk"
fi
"$BUILD/emsdk/emsdk" install "$EMSDK_VERSION"
"$BUILD/emsdk/emsdk" activate "$EMSDK_VERSION"
source "$BUILD/emsdk/emsdk_env.sh"

# --- Geant4 source ---
SRC="$BUILD/geant4-v$GEANT4_VERSION"
if [ ! -d "$SRC" ]; then
  curl -L "https://gitlab.cern.ch/geant4/geant4/-/archive/v$GEANT4_VERSION/geant4-v$GEANT4_VERSION.tar.gz" \
    -o "$BUILD/geant4.tar.gz"
  tar -xzf "$BUILD/geant4.tar.gz" -C "$BUILD"
fi

# --- configure & build (static, no vis/UI, no data install) ---
INSTALL="$BUILD/geant4-wasm-install"
mkdir -p "$BUILD/geant4-wasm-build"
cd "$BUILD/geant4-wasm-build"
emcmake cmake "$SRC" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$INSTALL" \
  -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF \
  -DGEANT4_INSTALL_DATA=OFF \
  -DGEANT4_USE_GDML=OFF \
  -DGEANT4_USE_SYSTEM_EXPAT=OFF \
  -DGEANT4_USE_SYSTEM_ZLIB=OFF \
  -DGEANT4_BUILD_MULTITHREADED=OFF \
  -DGEANT4_USE_OPENGL_X11=OFF -DGEANT4_USE_QT=OFF \
  -DCMAKE_CXX_FLAGS="-fexceptions -O2" \
  -DCMAKE_C_FLAGS="-O2 -DXML_POOR_ENTROPY" \
  -DCMAKE_EXE_LINKER_FLAGS="-fexceptions"
emmake make -j"$JOBS"
emmake make install
echo "Geant4 WASM install at $INSTALL"
