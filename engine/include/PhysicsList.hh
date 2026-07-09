#ifndef PHYSICSLIST_HH
#define PHYSICSLIST_HH

#include "G4EmStandardPhysics_option4.hh"
#include "G4VModularPhysicsList.hh"

// Gamma/e-/e+ electromagnetic physics only (Livermore/Penelope-grade models).
// No hadronics, no radioactive decay -> only G4EMLOW + G4ENSDFSTATE data needed.
class PhysicsList : public G4VModularPhysicsList {
public:
  PhysicsList() {
    SetVerboseLevel(0);
    RegisterPhysics(new G4EmStandardPhysics_option4(0));
  }
};

#endif
