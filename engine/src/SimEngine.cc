#include "SimEngine.hh"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "PrimaryGenerator.hh"

#include "G4RunManager.hh"

#include "json.hpp"

#include <sstream>

using nlohmann::json;

SimEngine::SimEngine() {
  fRunManager = new G4RunManager();
  fDetector = new DetectorConstruction(fCfg);
  fRunManager->SetUserInitialization(fDetector);
  fRunManager->SetUserInitialization(new PhysicsList());
}

SimEngine::~SimEngine() { delete fRunManager; }

void SimEngine::Configure(const std::string& jsonConfig) {
  json j = json::parse(jsonConfig);
  auto get = [&j](const char* key, auto& field) {
    if (j.contains(key)) field = j[key].get<std::decay_t<decltype(field)>>();
  };
  get("material", fCfg.material);
  get("shape", fCfg.shape);
  get("diameter", fCfg.diameter);
  get("height", fCfg.height);
  get("boxX", fCfg.boxX);
  get("boxY", fCfg.boxY);
  get("wellDiameter", fCfg.wellDiameter);
  get("wellDepth", fCfg.wellDepth);
  get("housingThickness", fCfg.housingThickness);
  get("sourceType", fCfg.sourceType);
  get("sourceDistance", fCfg.sourceDistance);
  get("energy", fCfg.energy);
  get("nBins", fCfg.nBins);
  get("eMax", fCfg.eMax);
  get("trackSampleRate", fCfg.trackSampleRate);
  get("maxTrackedEvents", fCfg.maxTrackedEvents);

  fDetector->SetConfig(fCfg);
  if (fInitialized) {
    fRunManager->ReinitializeGeometry();
  }
  fData.Configure(fCfg);
}

void SimEngine::RebuildActions() {
  auto* generator =
      new PrimaryGenerator(fCfg, fDetector->EnvelopeRadius(), fDetector->FrontFaceZ());
  fData.solidAngleFraction = generator->SolidAngleFraction();
  auto* eventAction = new EventAction(&fData);
  fRunManager->SetUserAction(generator);
  fRunManager->SetUserAction(eventAction);
  fRunManager->SetUserAction(new SteppingAction(&fData, eventAction, fDetector));
}

void SimEngine::Run(int nEvents, int chunkSize,
                    const std::function<void(long)>& progress) {
  if (!fInitialized) {
    fRunManager->Initialize();
    fInitialized = true;
  }
  // (Re)create user actions after the kernel is initialized; the run manager
  // deletes the previously registered ones.
  RebuildActions();
  if (chunkSize <= 0) chunkSize = 1000;
  long done = 0;
  while (done < nEvents) {
    const int n = static_cast<int>(std::min<long>(chunkSize, nEvents - done));
    fRunManager->BeamOn(n);
    done += n;
    if (progress) progress(done);
  }
}

std::string SimEngine::SummaryJson() const {
  const double omega = fData.solidAngleFraction;
  const long n = fData.nEvents > 0 ? fData.nEvents : 1;
  json j;
  j["nEvents"] = fData.nEvents;
  j["nDeposits"] = fData.nDeposits;
  j["nFullPeak"] = fData.nFullPeak;
  j["solidAngleFraction"] = omega;
  j["binWidth"] = fData.binWidth;
  j["energy"] = fCfg.energy;
  // Intrinsic-ish: relative to gammas emitted into the biased cone (~hitting detector)
  j["intrinsicTotalEff"] = double(fData.nDeposits) / n;
  j["intrinsicPeakEff"] = double(fData.nFullPeak) / n;
  // Absolute: relative to 4pi emission (beam: same as intrinsic)
  j["absoluteTotalEff"] = double(fData.nDeposits) / n * omega;
  j["absolutePeakEff"] = double(fData.nFullPeak) / n * omega;
  j["peakToTotal"] =
      fData.nDeposits > 0 ? double(fData.nFullPeak) / fData.nDeposits : 0.0;
  return j.dump();
}

std::string SimEngine::TracksJson() const {
  json arr = json::array();
  for (const auto& t : fData.tracks) {
    arr.push_back({{"ev", t.eventId},
                   {"tid", t.trackId},
                   {"pdg", t.pdg},
                   {"p0", {t.x0, t.y0, t.z0}},
                   {"p1", {t.x1, t.y1, t.z1}},
                   {"proc", t.process}});
  }
  json j;
  j["tracks"] = arr;
  // Geometry info for the viewer
  j["geometry"] = {{"shape", fCfg.shape},
                   {"diameter", fCfg.diameter},
                   {"height", fCfg.height},
                   {"boxX", fCfg.boxX},
                   {"boxY", fCfg.boxY},
                   {"wellDiameter", fCfg.wellDiameter},
                   {"wellDepth", fCfg.wellDepth},
                   {"housingThickness", fCfg.housingThickness},
                   {"sourceZ", fDetector->FrontFaceZ() - fCfg.sourceDistance}};
  return j.dump();
}
