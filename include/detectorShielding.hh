#ifndef detectorShielding_h
#define detectorShielding_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4GenericMessenger.hh"
#include "globals.hh"

class G4VPhysicalVolume;

class detectorShielding : public G4VUserDetectorConstruction
{
public:
  detectorShielding();
  ~detectorShielding() override;

  G4VPhysicalVolume* Construct() override;

  void SetInnerCu1Thickness(G4double val);
  void SetInnerCu2Thickness(G4double val);
  void SetOuterPb1Thickness(G4double val);
  void SetOuterPb2Thickness(G4double val);

private:
  G4VPhysicalVolume* DefineVolumes();

  G4double fHPGeHalfX;
  G4double fHPGeHalfY;
  G4double fHPGeHalfZ;

  G4double fCavityHalfX;
  G4double fCavityHalfY;
  G4double fCavityHalfZ;

  G4double fInnerCu1Thickness;
  G4double fInnerCu2Thickness;
  G4double fOuterPb1Thickness;
  G4double fOuterPb2Thickness;

  G4GenericMessenger* fMessenger;
};

#endif