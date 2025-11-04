#include "sensitiveDetector.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <iomanip>

SensitiveDetector::SensitiveDetector(const G4String& name, const G4String& outputFile)
  : G4VSensitiveDetector(name)
{
  fOutFile.open(outputFile, std::ios::out | std::ios::app);
  if (fOutFile.is_open()) {
    fOutFile << "# EventID\tTrackID\tTotalTrackLength_mm\tFinalVolume" << std::endl;
  }
}

SensitiveDetector::~SensitiveDetector()
{
  if (fOutFile.is_open()) fOutFile.close();
}

void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
  // Reset track data for this event
  fTrackData.clear();
}

G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  G4Track* track = step->GetTrack();

  // ✅ Only process primary gammas (parentID == 0)
  if (track->GetDefinition()->GetParticleName() != "gamma") return false;
  if (track->GetParentID() != 0) return false;  // skip secondaries

  G4int trackID = track->GetTrackID();

  // Accumulate total path length
  fTrackData[trackID].totalLength += step->GetStepLength();

  // If the gamma stops, record where
  if (track->GetTrackStatus() == fStopAndKill)
  {
    auto postVol = step->GetPostStepPoint()->GetPhysicalVolume();
    fTrackData[trackID].finalVolume = postVol ? postVol->GetName() : "OutsideWorldVolume";
  }

  return true;
}

void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
  // Each event can have many primary gammas
  G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

  for (auto& [trackID, data] : fTrackData)
  {
    if (fOutFile.is_open()) {
      fOutFile << eventID << "\t"
               << trackID << "\t"
               << std::fixed << std::setprecision(3)
               << data.totalLength/mm << "\t"
               << data.finalVolume << std::endl;
    }
  }

  fOutFile.flush();
}
