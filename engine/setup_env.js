// Injected via --pre-js: point Geant4 at the dataset locations inside MEMFS.
// The worker fetches the slimmed datasets into /data before calling init().
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  ENV.G4LEDATA = '/data/G4EMLOW';
  ENV.G4ENSDFSTATEDATA = '/data/G4ENSDFSTATE';
  // Unused by our physics list but some Geant4 builds probe them at startup;
  // point them somewhere harmless.
  ENV.G4PARTICLEXSDATA = '/data/G4PARTICLEXS';
  ENV.G4LEVELGAMMADATA = '/data/PhotonEvaporation';
  ENV.G4RADIOACTIVEDATA = '/data/RadioactiveDecay';
});
