#ifndef SIMCONFIG_HH
#define SIMCONFIG_HH

#include <string>

// All lengths in mm, energies in keV.
struct SimConfig {
  // Detector
  std::string material = "NaI";      // NaI, CsI, LaBr3, BGO, HPGe, CdTe, PVT
  std::string shape = "cylinder";    // cylinder, box, well
  double diameter = 76.2;            // cylinder/well outer diameter
  double height = 76.2;              // cylinder/well/box height (z)
  double boxX = 50.0, boxY = 50.0;   // box cross-section
  double wellDiameter = 16.0;        // well bore diameter
  double wellDepth = 40.0;           // well bore depth (from front face)
  // Housing (aluminum can around detector, 0 = none)
  double housingThickness = 1.0;
  // Source
  std::string sourceType = "point";  // point (isotropic, cone-biased), beam (parallel disc)
  double sourceDistance = 100.0;     // point source: distance from front face on axis
  double energy = 662.0;             // gamma energy, keV (single line)
  // Run
  int nBins = 2048;
  double eMax = 0.0;                 // histogram max, keV; 0 => 1.1 * energy
  int trackSampleRate = 0;           // record tracks every Nth event, 0 = off
  int maxTrackedEvents = 50;         // cap on events with stored tracks
};

#endif
