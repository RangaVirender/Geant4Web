#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "SimConfig.hh"

class G4LogicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  explicit DetectorConstruction(const SimConfig& cfg) : fCfg(cfg) {}

  G4VPhysicalVolume* Construct() override;

  void SetConfig(const SimConfig& cfg) { fCfg = cfg; }
  const G4LogicalVolume* GetScoringVolume() const { return fScoringVolume; }

  // Envelope of the sensitive volume, used for source cone biasing.
  double EnvelopeRadius() const;   // mm, transverse
  double FrontFaceZ() const;       // mm, z of detector front face (facing source at -z)

private:
  G4Material* BuildMaterial(const std::string& name) const;

  SimConfig fCfg;
  G4LogicalVolume* fScoringVolume = nullptr;
};

#endif
