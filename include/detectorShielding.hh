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

  // geometry setters
    void SetInnerCu1Thickness(G4double thickness);
    void SetInnerCu2Thickness(G4double thickness);
    void SetOuterPb1Thickness(G4double thickness);
    void SetOuterPb2Thickness(G4double thickness);
    void SetCavityHalfX(G4double halfX);
    void SetCavityHalfY(G4double halfY);
    void SetCavityHalfZ(G4double halfZ);

  // ------------------------------
  // ------------------------------
  void SetSimulationTime(G4double time);
  void ComputeDecaysForLayer(const G4String& input);
  void AddDecays(G4double n);
  G4double GetTotalDecays() const;
  void AutoBeamOn();
  // ------------------------------

private:
  G4VPhysicalVolume* DefineVolumes();

  // internal helper
  G4double GetLayerMass(const G4String& layerName);

  // geometry numbers
  G4double fHPGeHalfX, fHPGeHalfY, fHPGeHalfZ;
  G4double fCavityHalfX, fCavityHalfY, fCavityHalfZ;
  G4double fInnerCu1Thickness, fInnerCu2Thickness;
  G4double fOuterPb1Thickness, fOuterPb2Thickness;

  // simulation time storage
  G4double fSimTime = 0;

  G4double fTotalDecays = 0;

  G4GenericMessenger* fMessenger;

  std::map<std::string, int> layerMap;

  G4bool fGeometryDirty;
};

#endif
