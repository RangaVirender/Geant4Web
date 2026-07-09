// Native CLI for validation: web-geant4 <config.json> <nEvents> <out_prefix>
// Writes <out_prefix>_spectrum.csv, <out_prefix>_summary.json, <out_prefix>_tracks.json
#include "SimEngine.hh"

#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, char** argv) {
  std::string configJson = "{}";
  int nEvents = 100000;
  std::string prefix = "out";
  if (argc > 1) {
    std::ifstream f(argv[1]);
    std::stringstream ss;
    ss << f.rdbuf();
    configJson = ss.str();
  }
  if (argc > 2) nEvents = std::stoi(argv[2]);
  if (argc > 3) prefix = argv[3];

  SimEngine engine;
  engine.Configure(configJson);
  engine.Run(nEvents, 10000, [nEvents](long done) {
    std::cout << "\rprogress: " << done << "/" << nEvents << std::flush;
  });
  std::cout << "\n";

  {
    std::ofstream out(prefix + "_spectrum.csv");
    out << "energy_kev,counts\n";
    const auto& h = engine.Spectrum();
    for (size_t i = 0; i < h.size(); ++i)
      out << (i + 0.5) * engine.BinWidth() << "," << h[i] << "\n";
  }
  {
    std::ofstream out(prefix + "_summary.json");
    out << engine.SummaryJson() << "\n";
  }
  {
    std::ofstream out(prefix + "_tracks.json");
    out << engine.TracksJson() << "\n";
  }
  std::cout << "summary: " << engine.SummaryJson() << "\n";
  return 0;
}
