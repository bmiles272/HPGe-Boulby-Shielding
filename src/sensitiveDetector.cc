#include "sensitiveDetector.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include <iomanip>

SensitiveDetector::SensitiveDetector(const G4String& name)
  : G4VSensitiveDetector(name),
    fTotalEnergyDeposit(0.0),
    fNHits(0)
{}

SensitiveDetector::~SensitiveDetector()
{}

void SensitiveDetector::Initialize(G4HCofThisEvent*)
{
  fTotalEnergyDeposit = 0.0;
  fNHits = 0;
}

G4bool SensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    //auto particle = step->GetTrack()->GetParticleDefinition()->GetParticleName();
    //if (particle != "gamma") 
        //return false;
    
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false;

    fTotalEnergyDeposit += edep;
    fNHits++;

    auto analysisManager = G4AnalysisManager::Instance();
    
    std::string particleName = step->GetTrack()->GetParticleDefinition()->GetParticleName();
    if (particleName != "gamma") return false;

    analysisManager->FillNtupleDColumn(0, 0, edep);
    analysisManager->FillNtupleDColumn(0, 1, edep/keV); 
    analysisManager->FillNtupleDColumn(0, 2, step->GetTrack()->GetTrackID());
    analysisManager->FillNtupleSColumn(0, 3, particleName);
    analysisManager->FillNtupleDColumn(0, 4, step->GetPreStepPoint()->GetGlobalTime()/ns);
    analysisManager->AddNtupleRow(0);

    analysisManager->FillH1(0, edep);
    
    return true;
}

void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
{
    auto analysisManager = G4AnalysisManager::Instance();
    G4int eventID = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();
    
    if (fTotalEnergyDeposit > 0) {
        analysisManager->FillNtupleDColumn(1, 0, eventID);            
        analysisManager->FillNtupleDColumn(1, 1, fTotalEnergyDeposit);  
        analysisManager->FillNtupleDColumn(1, 2, fTotalEnergyDeposit/keV); 
        analysisManager->FillNtupleDColumn(1, 3, fNHits);      
        analysisManager->AddNtupleRow(1);
        
        analysisManager->FillH1(1, fTotalEnergyDeposit);
    }
  }