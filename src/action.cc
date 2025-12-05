#include "detectorShielding.hh"
#include "generator.hh"
#include "action.hh"
#include "G4RunManager.hh"

MyActionInitialization::MyActionInitialization(detectorShielding* det)
    : fDet(det)
{}

MyActionInitialization::~MyActionInitialization()
{}

void MyActionInitialization::Build() const {
    SetUserAction(new MyPrimaryGenerator(fDet));
}