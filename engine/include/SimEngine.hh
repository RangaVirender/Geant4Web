#ifndef SIMENGINE_HH
#define SIMENGINE_HH

#include "Actions.hh"
#include "SimConfig.hh"

#include <functional>
#include <string>
#include <vector>

class G4RunManager;
class DetectorConstruction;
class PrimaryGenerator;

// Owns the Geant4 run manager. Create once; reconfigure between runs.
class SimEngine {
public:
  SimEngine();
  ~SimEngine();

  // jsonConfig: flat JSON object matching SimConfig fields (all optional).
  void Configure(const std::string& jsonConfig);

  // Runs nEvents in chunks; progress(doneEvents) is called after each chunk.
  void Run(int nEvents, int chunkSize, const std::function<void(long)>& progress);

  const std::vector<double>& Spectrum() const { return fData.hist; }
  double BinWidth() const { return fData.binWidth; }
  std::string SummaryJson() const;
  std::string TracksJson() const;
  const SimConfig& Config() const { return fCfg; }

private:
  void RebuildActions();

  SimConfig fCfg;
  RunData fData;
  G4RunManager* fRunManager = nullptr;
  DetectorConstruction* fDetector = nullptr;   // owned by run manager
  bool fInitialized = false;
};

#endif
