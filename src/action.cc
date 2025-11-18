#include "detectorShielding.hh"
#include "generator.hh"
#include "action.hh"
#include "G4RunManager.hh"

MyActionInitialization::MyActionInitialization()
{}

MyActionInitialization::~MyActionInitialization()
{}

void MyActionInitialization::Build() const {
    auto det = static_cast<const detectorShielding*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction()
    );

    MyPrimaryGenerator* generator = new MyPrimaryGenerator(det);
    SetUserAction(new MyPrimaryGenerator(det));
}