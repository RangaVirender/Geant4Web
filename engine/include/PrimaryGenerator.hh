#ifndef PRIMARYGENERATOR_HH
#define PRIMARYGENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "SimConfig.hh"

class G4ParticleGun;

// Point source on the detector axis at -z (isotropic, biased into the cone
// subtending the detector envelope; the solid-angle fraction is recorded so
// absolute efficiencies can be recovered), or a parallel disc beam along +z.
class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {
public:
  PrimaryGenerator(const SimConfig& cfg, double envelopeRadius, double frontFaceZ);
  ~PrimaryGenerator() override;

  void GeneratePrimaries(G4Event* event) override;

  // Fraction of 4pi into which the point source is emitted (1.0 for beam).
  double SolidAngleFraction() const { return fSolidAngleFraction; }

private:
  SimConfig fCfg;
  G4ParticleGun* fGun;
  double fSourceZ = 0;        // mm
  double fCosThetaMin = -1;   // cone half-angle cosine (emission toward +z)
  double fEnvelopeRadius = 0; // mm
  double fSolidAngleFraction = 1.0;
};

#endif
