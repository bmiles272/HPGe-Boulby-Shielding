#include <iostream>
#include <filesystem>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4AnalysisManager.hh"

// physics list
#include "G4EmLivermorePhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4PhysListFactory.hh"

// detector shielding file
#include "detectorShielding.hh"
#include "action.hh"

int main(int argc, char** argv)
{
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Run manager
    auto runManager = new G4RunManager();

    // Create detector FIRST
    auto detector = new detectorShielding();
    runManager->SetUserInitialization(detector);

    // Physics list
    G4PhysListFactory factory;
    G4VModularPhysicsList* physList = factory.GetReferencePhysList("FTFP_BERT");
    //physList->RegisterPhysics(new G4EmLivermorePhysics());
    //physList->RegisterPhysics(new G4DecayPhysics());
    physList->RegisterPhysics(new G4RadioactiveDecayPhysics());
    runManager->SetUserInitialization(physList);

    runManager->SetUserInitialization(new MyActionInitialization(detector));

    auto analysisManager = G4AnalysisManager::Instance();
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    if (ui) {
        // INTERACTIVE MODE: Set up visualization
        runManager->Initialize();
        auto visManager = new G4VisExecutive();
        visManager->Initialize();
        UImanager->ApplyCommand("/control/macroPath /home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/macros");

        // Run your radioactive decay calculation
        UImanager->ApplyCommand("/control/execute visualise.mac");

        ui->SessionStart();
        delete ui;
        delete visManager;
    } else {
        // Create output file name - will be overridden by macro if specified
        G4String outputdir = "/home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/root/";
        G4String outputFileName = "default.root";
        analysisManager->SetFileName(outputdir + outputFileName);

        // Create histograms
        analysisManager->CreateH1("HitEnergy", "Energy per Hit in HPGe", 6000, 0., 3.*MeV);
        analysisManager->CreateH1("EventEnergy", "Total Energy per Event in HPGe", 6000, 0., 3.*MeV);

        // Create ntuple for detailed hit information
        analysisManager->CreateNtuple("Hits", "Individual Hit Data");
        analysisManager->CreateNtupleDColumn("Energy");          // 0: Energy (MeV)
        analysisManager->CreateNtupleDColumn("Energy_keV");      // 1: Energy (keV)  
        analysisManager->CreateNtupleDColumn("TrackID");         // 2: Track ID
        analysisManager->CreateNtupleSColumn("Particle");        // 3: Particle type
        analysisManager->CreateNtupleDColumn("Time_ns");         // 4: Time of hit (ns)
        analysisManager->FinishNtuple();                         // Ntuple ID 0

        // Create ntuple for event summary
        analysisManager->CreateNtuple("Events", "Event Summary");
        analysisManager->CreateNtupleDColumn("EventID");         // 0: Event ID
        analysisManager->CreateNtupleDColumn("TotalEnergy");     // 1: Total energy (MeV)
        analysisManager->CreateNtupleDColumn("TotalEnergy_keV"); // 2: Total energy (keV)
        analysisManager->CreateNtupleDColumn("NHits");           // 3: Number of hits
        analysisManager->FinishNtuple();                         // Ntuple ID 1
        // =======================================================================
        G4cout << "ROOT analysis set up. Output file: " << outputFileName << G4endl;
        // =======================================================================
        runManager->Initialize();
        UImanager->ApplyCommand("/control/macroPath /home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/macros");
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        // BATCH MODE: No visualization, just execute the macro
        UImanager->ApplyCommand(command + fileName);
        G4cout << "Batch mode: Executing macro " << fileName << G4endl;
        G4cout << "Visualization disabled for high-statistics run" << G4endl;

        analysisManager->Write();
        analysisManager->CloseFile();
        //system(("ls -lh " + outputdir).c_str());
        G4cout << "ROOT output written to " << analysisManager->GetFileName() << G4endl;
    }

    delete runManager;
    return 0;
}