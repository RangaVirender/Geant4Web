#include "PrimaryGenerator.hh"

#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGenerator::PrimaryGenerator(const SimConfig& cfg, double envelopeRadius, double frontFaceZ)
    : fCfg(cfg), fEnvelopeRadius(envelopeRadius) {
  fGun = new G4ParticleGun(1);
  fGun->SetParticleDefinition(G4Gamma::Definition());
  fGun->SetParticleEnergy(fCfg.energy * keV);

  if (fCfg.sourceType == "point") {
    fSourceZ = frontFaceZ - fCfg.sourceDistance;  // mm
    // Cone that covers the whole detector envelope (from source to back corner),
    // with a 5% angular margin.
    const double dzBack = std::abs(fSourceZ) + /*full det length*/ 0.0;
    (void)dzBack;
    const double dz = fCfg.sourceDistance;  // to front face
    double theta = std::atan2(fEnvelopeRadius, dz);
    theta = std::min(theta * 1.05, M_PI / 2);
    fCosThetaMin = std::cos(theta);
    fSolidAngleFraction = 0.5 * (1.0 - fCosThetaMin);
  } else {  // beam
    fSourceZ = frontFaceZ - 10.0;  // 10 mm upstream of front face
    fSolidAngleFraction = 1.0;
  }
}

PrimaryGenerator::~PrimaryGenerator() { delete fGun; }

void PrimaryGenerator::GeneratePrimaries(G4Event* event) {
  if (fCfg.sourceType == "point") {
    fGun->SetParticlePosition(G4ThreeVector(0, 0, fSourceZ * mm));
    const double cosT = fCosThetaMin + (1.0 - fCosThetaMin) * G4UniformRand();
    const double sinT = std::sqrt(std::max(0.0, 1.0 - cosT * cosT));
    const double phi = CLHEP::twopi * G4UniformRand();
    fGun->SetParticleMomentumDirection(
        G4ThreeVector(sinT * std::cos(phi), sinT * std::sin(phi), cosT));
  } else {
    // Uniform disc of the envelope radius, parallel to +z
    const double r = fEnvelopeRadius * std::sqrt(G4UniformRand());
    const double phi = CLHEP::twopi * G4UniformRand();
    fGun->SetParticlePosition(
        G4ThreeVector(r * std::cos(phi) * mm, r * std::sin(phi) * mm, fSourceZ * mm));
    fGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
  }
  fGun->GeneratePrimaryVertex(event);
}
