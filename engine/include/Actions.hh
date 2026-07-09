#ifndef ACTIONS_HH
#define ACTIONS_HH

#include "G4UserEventAction.hh"
#include "G4UserSteppingAction.hh"
#include "SimConfig.hh"

#include <string>
#include <vector>

class DetectorConstruction;

// One recorded step segment for visualization.
struct TrackStep {
  int eventId;
  int trackId;
  int pdg;                // 22 gamma, 11 e-, -11 e+
  float x0, y0, z0;       // mm
  float x1, y1, z1;       // mm
  std::string process;    // process at post-step point
};

// Shared accumulation state for a run.
struct RunData {
  std::vector<double> hist;     // counts per bin (energy deposited in crystal)
  double binWidth = 1.0;        // keV
  long nEvents = 0;
  long nDeposits = 0;           // events with any Edep > threshold
  long nFullPeak = 0;           // events with Edep within FEP window
  double fepLow = 0, fepHigh = 0;  // keV window for full-energy peak counting
  double solidAngleFraction = 1.0;
  std::vector<TrackStep> tracks;
  int trackSampleRate = 0;
  int maxTrackedEvents = 50;
  int trackedEvents = 0;
  bool trackCurrentEvent = false;

  void Configure(const SimConfig& cfg);
  void Fill(double edepKeV);
};

class EventAction : public G4UserEventAction {
public:
  explicit EventAction(RunData* data) : fData(data) {}
  void BeginOfEventAction(const G4Event* event) override;
  void EndOfEventAction(const G4Event* event) override;
  void AddEdep(double e) { fEdep += e; }

private:
  RunData* fData;
  double fEdep = 0;  // keV
};

class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction(RunData* data, EventAction* eventAction, const DetectorConstruction* det)
      : fData(data), fEventAction(eventAction), fDetector(det) {}
  void UserSteppingAction(const G4Step* step) override;

private:
  RunData* fData;
  EventAction* fEventAction;
  const DetectorConstruction* fDetector;
};

#endif
