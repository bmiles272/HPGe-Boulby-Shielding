#ifndef detectorShielding_h
#define detectorShielding_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4GenericMessenger.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

class detectorShielding : public G4VUserDetectorConstruction
{
public:
  detectorShielding();
  ~detectorShielding() override;

  G4VPhysicalVolume* Construct() override;

  virtual void ConstructSDandField() override;

  // Activity configuration
  void SetCu1Activity(G4double activityPerKg);
  void SetCu2Activity(G4double activityPerKg);
  void SetPb1Activity(G4double activityPerKg);
  void SetPb2Activity(G4double activityPerKg);
  void SetLayerActivity(const G4String& input);
  void SetSimulationTime(G4double time);
  void AutoBeamOn();

  G4double GetLayerMass(const G4String& layerName);

  // geometry setters
  void SetInnerCu1Thickness(G4double thickness);
  void SetInnerCu2Thickness(G4double thickness);
  void SetOuterPb1Thickness(G4double thickness);
  void SetOuterPb2Thickness(G4double thickness);
  void SetCavityHalfX(G4double halfX);
  void SetCavityHalfY(G4double halfY);
  void SetCavityHalfZ(G4double halfZ);

  //G4double GetTotalDecays() const;
  //G4int fTotalDecays;

  G4int GetTotalDecays() const { return fTotalDecays; }
  void SetTotalDecays(G4int decays);

private:
  G4VPhysicalVolume* DefineVolumes();

  void SetLayerActivityForName(const G4String& name, G4double activityPerKg);

  // geometry numbers
  G4double fHPGeHeight, fHPGeDiam;
  G4double fCavityHalfX, fCavityHalfY, fCavityHalfZ;
  G4double fInnerCu1Thickness, fInnerCu2Thickness;
  G4double fOuterPb1Thickness, fOuterPb2Thickness;

  // simulation time storage
  G4double fSimTime = 0;
  G4int fTotalDecays = 0;

  G4double GetLayerMass(const G4String& name) const;

  G4GenericMessenger* fMessenger;
  std::map<std::string, int> layerMap;
  G4bool fGeometryDirty;
};

#endif
