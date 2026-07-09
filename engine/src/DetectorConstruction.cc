#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Tubs.hh"

#include <algorithm>

G4Material* DetectorConstruction::BuildMaterial(const std::string& name) const {
  auto* nist = G4NistManager::Instance();
  if (name == "NaI")  return nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");
  if (name == "CsI")  return nist->FindOrBuildMaterial("G4_CESIUM_IODIDE");
  if (name == "BGO")  return nist->FindOrBuildMaterial("G4_BGO");
  if (name == "HPGe") return nist->FindOrBuildMaterial("G4_Ge");
  if (name == "CdTe") return nist->FindOrBuildMaterial("G4_CADMIUM_TELLURIDE");
  if (name == "PVT")  return nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
  if (name == "LaBr3") {
    static G4Material* labr = nullptr;
    if (!labr) {
      labr = new G4Material("LaBr3", 5.06 * g / cm3, 2);
      labr->AddElement(nist->FindOrBuildElement("La"), 1);
      labr->AddElement(nist->FindOrBuildElement("Br"), 3);
    }
    return labr;
  }
  return nist->FindOrBuildMaterial("G4_SODIUM_IODIDE");
}

double DetectorConstruction::EnvelopeRadius() const {
  double r;
  if (fCfg.shape == "box")
    r = 0.5 * std::hypot(fCfg.boxX, fCfg.boxY);
  else
    r = 0.5 * fCfg.diameter;
  return r + fCfg.housingThickness;
}

double DetectorConstruction::FrontFaceZ() const {
  // Detector is centered at origin; front face (toward source at -z).
  return -0.5 * fCfg.height - fCfg.housingThickness;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  auto* nist = G4NistManager::Instance();
  auto* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
  auto* air = nist->FindOrBuildMaterial("G4_AIR");
  auto* alu = nist->FindOrBuildMaterial("G4_Al");
  auto* detMat = BuildMaterial(fCfg.material);

  const double h = fCfg.height * mm;
  const double t = fCfg.housingThickness * mm;

  // World: big enough for source distance + detector
  const double worldR = std::max(fCfg.sourceDistance + fCfg.height, 2.0 * fCfg.height) * mm + 20 * cm;
  auto* worldS = new G4Box("World", worldR, worldR, worldR);
  auto* worldL = new G4LogicalVolume(worldS, air, "World");
  auto* worldP = new G4PVPlacement(nullptr, {}, worldL, "World", nullptr, false, 0, true);

  G4VSolid* crystalS = nullptr;
  G4VSolid* housingS = nullptr;

  if (fCfg.shape == "box") {
    crystalS = new G4Box("Crystal", 0.5 * fCfg.boxX * mm, 0.5 * fCfg.boxY * mm, 0.5 * h);
    if (t > 0)
      housingS = new G4Box("HousingOuter", 0.5 * fCfg.boxX * mm + t, 0.5 * fCfg.boxY * mm + t, 0.5 * h + t);
  } else {
    const double r = 0.5 * fCfg.diameter * mm;
    G4VSolid* solid = new G4Tubs("CrystalFull", 0, r, 0.5 * h, 0, 360 * deg);
    if (fCfg.shape == "well") {
      const double wr = 0.5 * fCfg.wellDiameter * mm;
      const double wd = fCfg.wellDepth * mm;
      auto* bore = new G4Tubs("Bore", 0, wr, 0.5 * wd + 0.5 * mm, 0, 360 * deg);
      // bore enters from front face (-z)
      solid = new G4SubtractionSolid("Crystal", solid, bore, nullptr,
                                     G4ThreeVector(0, 0, -0.5 * h + 0.5 * wd - 0.5 * mm));
    }
    crystalS = solid;
    if (t > 0)
      housingS = new G4Tubs("HousingOuter", 0, r + t, 0.5 * h + t, 0, 360 * deg);
  }

  auto* crystalL = new G4LogicalVolume(crystalS, detMat, "Crystal");
  fScoringVolume = crystalL;

  if (housingS) {
    auto* housingL = new G4LogicalVolume(housingS, alu, "Housing");
    new G4PVPlacement(nullptr, {}, housingL, "Housing", worldL, false, 0, true);
    // Fill housing interior with vacuum-equivalent gap? Keep simple: crystal sits inside housing solid.
    new G4PVPlacement(nullptr, {}, crystalL, "Crystal", housingL, false, 0, true);
    (void)vacuum;
  } else {
    new G4PVPlacement(nullptr, {}, crystalL, "Crystal", worldL, false, 0, true);
  }

  return worldP;
}
