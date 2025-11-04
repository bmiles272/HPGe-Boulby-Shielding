#include <iostream>
#include <filesystem>

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

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

  // run manager
  G4RunManager* runManager = new G4RunManager();

  runManager->SetUserInitialization(new detectorShielding());

  // physics list
  G4PhysListFactory factory;
  G4VModularPhysicsList* physList = factory.GetReferencePhysList("FTFP_BERT");
  physList->RegisterPhysics(new G4EmLivermorePhysics());
  physList->RegisterPhysics(new G4DecayPhysics());
  physList->RegisterPhysics(new G4RadioactiveDecayPhysics());
  runManager->SetUserInitialization(physList);
  runManager->SetUserInitialization(new MyActionInitialization());

  runManager->Initialize();

  G4VisManager* visManager = new G4VisExecutive();

  visManager->Initialize();

  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  if (!ui) {
    // batch mode
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  } else {
    // interactive mode
    
    UImanager->ApplyCommand("/vis/open OGL 600x600");
    UImanager->ApplyCommand("/vis/drawVolume");
    UImanager->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 30 30");
    UImanager->ApplyCommand("/vis/viewer/zoom 1.5");
    UImanager->ApplyCommand("/vis/viewer/set/style surface");
    UImanager->ApplyCommand("/vis/viewer/set/background white");
    
    //UImanager->ApplyCommand("/vis/scene/add/axes 0 0 0 30 cm");

    //UImanager->ApplyCommand("/tracking/storeTrajectory 1");
    //UImanager->ApplyCommand("/vis/scene/add/trajectories rich");
    //UImanager->ApplyCommand("/vis/scene/add/hits");
    //UImanager->ApplyCommand("/vis/scene/endOfEventAction accumulate");
    //UImanager->ApplyCommand("/vis/modeling/trajectories/create/drawByParticleID");
    //UImanager->ApplyCommand("/vis/viewer/refresh");
    //UImanager->ApplyCommand("/vis/viewer/set/autoRefresh true");
    UImanager->ApplyCommand("/control/macroPath /home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/macros");

    UImanager->ApplyCommand("/control/execute sourceGPS.mac");


    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
  
  return 0;
}