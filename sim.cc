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
  auto detector = new detectorShielding();
  runManager->SetUserInitialization(detector);

  auto generator = new MyPrimaryGenerator(detector);
  runManager->SetUserAction(generator);


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
  
    UImanager->ApplyCommand("/control/macroPath /home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/macros");

    UImanager->ApplyCommand("/control/execute sourceGPS.mac");


    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
  
  return 0;
}