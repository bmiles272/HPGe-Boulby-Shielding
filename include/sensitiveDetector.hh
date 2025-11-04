#ifndef SENSITIVEDETECTOR_HH
#define SENSITIVEDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include <fstream>
#include <map>

struct GammaTrackData {
  G4double totalLength = 0.0;
  G4String finalVolume = "Unknown";
};

class SensitiveDetector : public G4VSensitiveDetector
{
public:
  SensitiveDetector(const G4String& name, const G4String& outputFile);
  ~SensitiveDetector() override;

  void Initialize(G4HCofThisEvent*) override;
  G4bool ProcessHits(G4Step* step, G4TouchableHistory*) override;
  void EndOfEvent(G4HCofThisEvent*) override;

private:
  std::ofstream fOutFile;
  std::map<G4int, GammaTrackData> fTrackData;  // trackID → data, many per event
};

#endif
