#include "Actions.hh"

#include "DetectorConstruction.hh"
#include "G4Event.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include <algorithm>

void RunData::Configure(const SimConfig& cfg) {
  const double eMax = (cfg.eMax > 0) ? cfg.eMax : 1.1 * cfg.energy;
  hist.assign(cfg.nBins, 0.0);
  binWidth = eMax / cfg.nBins;
  nEvents = nDeposits = nFullPeak = 0;
  // FEP window: +-2 keV or +-0.3% of E, whichever is larger (raw Edep, no broadening)
  const double halfWin = std::max(2.0, 0.003 * cfg.energy);
  fepLow = cfg.energy - halfWin;
  fepHigh = cfg.energy + halfWin;
  tracks.clear();
  trackSampleRate = cfg.trackSampleRate;
  maxTrackedEvents = cfg.maxTrackedEvents;
  trackedEvents = 0;
  trackCurrentEvent = false;
}

void RunData::Fill(double edepKeV) {
  nEvents++;
  if (edepKeV <= 1e-6) return;
  nDeposits++;
  if (edepKeV >= fepLow && edepKeV <= fepHigh) nFullPeak++;
  const int bin = static_cast<int>(edepKeV / binWidth);
  if (bin >= 0 && bin < static_cast<int>(hist.size())) hist[bin] += 1.0;
}

void EventAction::BeginOfEventAction(const G4Event* event) {
  fEdep = 0;
  fData->trackCurrentEvent =
      fData->trackSampleRate > 0 &&
      fData->trackedEvents < fData->maxTrackedEvents &&
      (event->GetEventID() % fData->trackSampleRate) == 0;
}

void EventAction::EndOfEventAction(const G4Event*) {
  fData->Fill(fEdep);
  if (fData->trackCurrentEvent) fData->trackedEvents++;
}

void SteppingAction::UserSteppingAction(const G4Step* step) {
  // Energy deposition in the crystal
  if (step->GetPreStepPoint()->GetTouchableHandle()->GetVolume() &&
      step->GetPreStepPoint()->GetTouchableHandle()->GetVolume()->GetLogicalVolume() ==
          fDetector->GetScoringVolume()) {
    fEventAction->AddEdep(step->GetTotalEnergyDeposit() / keV);
  }

  // Track recording for visualization
  if (fData->trackCurrentEvent && fData->tracks.size() < 20000) {
    const auto* track = step->GetTrack();
    const int pdg = track->GetDefinition()->GetPDGEncoding();
    if (pdg != 22 && pdg != 11 && pdg != -11) return;
    const auto& p0 = step->GetPreStepPoint()->GetPosition();
    const auto& p1 = step->GetPostStepPoint()->GetPosition();
    const auto* proc = step->GetPostStepPoint()->GetProcessDefinedStep();
    TrackStep ts;
    ts.eventId = fData->trackedEvents;  // sequential id among sampled events
    ts.trackId = track->GetTrackID();
    ts.pdg = pdg;
    ts.x0 = p0.x() / mm; ts.y0 = p0.y() / mm; ts.z0 = p0.z() / mm;
    ts.x1 = p1.x() / mm; ts.y1 = p1.y() / mm; ts.z1 = p1.z() / mm;
    ts.process = proc ? proc->GetProcessName() : "transport";
    fData->tracks.push_back(std::move(ts));
  }
}
