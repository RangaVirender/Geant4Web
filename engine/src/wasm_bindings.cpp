// Emscripten embind API exposed to JavaScript (runs inside a Web Worker).
#include "SimEngine.hh"

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <memory>

using emscripten::val;

namespace {
std::unique_ptr<SimEngine> gEngine;

void init() {
  if (!gEngine) gEngine = std::make_unique<SimEngine>();
}

void configure(const std::string& jsonConfig) {
  init();
  gEngine->Configure(jsonConfig);
}

void run(int nEvents, int chunkSize, val progressCb) {
  init();
  gEngine->Run(nEvents, chunkSize, [&progressCb](long done) {
    progressCb(static_cast<double>(done));
  });
}

val getSpectrum() {
  const auto& h = gEngine->Spectrum();
  return val(emscripten::typed_memory_view(h.size(), h.data()));
}

double getBinWidth() { return gEngine->BinWidth(); }
std::string getSummary() { return gEngine->SummaryJson(); }
std::string getTracks() { return gEngine->TracksJson(); }
}  // namespace

EMSCRIPTEN_BINDINGS(geant4_module) {
  emscripten::function("init", &init);
  emscripten::function("configure", &configure);
  emscripten::function("run", &run);
  emscripten::function("getSpectrum", &getSpectrum);
  emscripten::function("getBinWidth", &getBinWidth);
  emscripten::function("getSummary", &getSummary);
  emscripten::function("getTracks", &getTracks);
}
