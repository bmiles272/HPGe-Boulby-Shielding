#include "detectorShielding.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "radioImpurities.hh"
#include "G4SDManager.hh"
#include "sensitiveDetector.hh"
#include "G4SubtractionSolid.hh"
#include "G4PhysicalVolumeStore.hh"

detectorShielding::detectorShielding()
 : G4VUserDetectorConstruction(),
   fHPGeHalfX(115*mm),
   fHPGeHalfY(225*mm),
   fHPGeHalfZ(115*mm),
   fInnerCu1Thickness(5*mm),
   fInnerCu2Thickness(20*mm),
   fOuterPb1Thickness(50*mm),
   fOuterPb2Thickness(150*mm),
   fCavityHalfX(50*mm),
   fCavityHalfY(10*mm),
   fCavityHalfZ(50*mm),
   fMessenger(nullptr)
{
  // optional: create simple messenger for runtime control (safe if header included)
  fMessenger = new G4GenericMessenger(this, "/geometry/", "Geometry control");
  // You can add commands here if desired, e.g. to change thicknesses before Construct is called.
}

detectorShielding::~detectorShielding()
{
  delete fMessenger;
}

G4VPhysicalVolume* detectorShielding::Construct()
{
  return DefineVolumes();
}

void detectorShielding::SetInnerCu1Thickness(G4double val) { fInnerCu1Thickness = val; }
void detectorShielding::SetInnerCu2Thickness(G4double val) { fInnerCu2Thickness = val; }
void detectorShielding::SetOuterPb1Thickness(G4double val) { fOuterPb1Thickness = val; }
void detectorShielding::SetOuterPb2Thickness(G4double val) { fOuterPb2Thickness = val; }

G4VPhysicalVolume* detectorShielding::DefineVolumes()
{
  G4NistManager* nist = G4NistManager::Instance();

  // Materials
  G4Material* Cu1Material = RadioImpurities::CreateUltraPureCopper();
  G4Material* Cu2Material = RadioImpurities::CreateImpureCopper();
  G4Material* Pb1Material = RadioImpurities::CreateLowBackgroundLead();
  G4Material* Pb2Material = RadioImpurities::CreateImpureLead();

  G4Material* Ge = nist->FindOrBuildMaterial("G4_Ge");
  G4Material* Air = nist->FindOrBuildMaterial("G4_AIR");

  // World
  G4double worldSize = 300 * cm;
  auto solidWorld = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
  auto logicWorld = new G4LogicalVolume(solidWorld, Air, "World");
  auto physWorld = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0, true);

  // ---------------- Core detector ----------------
  // HPGe crystal (innermost)
  auto solidHPGe = new G4Box("HPGe", fHPGeHalfX, fHPGeHalfY, fHPGeHalfZ);
  auto logicHPGe = new G4LogicalVolume(solidHPGe, Ge, "HPGe");
  new G4PVPlacement(nullptr, {}, logicHPGe, "HPGe", logicWorld, false, 0, true);

  // ---------------- Shielding layers ----------------
  G4double gap = 50*mm;

  // compute cumulative half-sizes (measured from center)
  G4double hpge_hx = fHPGeHalfX;

  // First calculate the maximum extent of HPGe + cavity in any direction
  G4double maxInnerExtentX = fHPGeHalfX + fCavityHalfX;
  G4double maxInnerExtentY = fHPGeHalfY + fCavityHalfY;
  G4double maxInnerExtentZ = fHPGeHalfZ + fCavityHalfZ;
  
  // Use the largest dimension to ensure cubic shells fully contain detector+cavity
  G4double maxInnerExtent = std::max({maxInnerExtentX, maxInnerExtentY, maxInnerExtentZ});
  
  // Now define cubic shells using this maximum extent
  G4double cu1_inner_half = maxInnerExtent;
  G4double cu1_outer_half = cu1_inner_half + fInnerCu1Thickness;

  G4double cu2_inner_half = cu1_outer_half;
  G4double cu2_outer_half = cu2_inner_half + fInnerCu2Thickness;

  G4double pb1_inner_half = cu2_outer_half;
  G4double pb1_outer_half = pb1_inner_half + fOuterPb1Thickness;

  G4double pb2_inner_half = pb1_outer_half;
  G4double pb2_outer_half = pb2_inner_half + fOuterPb2Thickness;

  G4ThreeVector origin = {0, 0, 0};

  // Inner copper shell (Cu1) using the computed halves
  auto solidCu1_outer = new G4Box("Cu1_outer",
      cu1_outer_half, cu1_outer_half, cu1_outer_half);
  auto solidCu1_inner = new G4Box("Cu1_inner",
      cu1_inner_half, cu1_inner_half, cu1_inner_half);
  auto solidCu1 = new G4SubtractionSolid("Cu1", solidCu1_outer, solidCu1_inner);
  auto logicCu1 = new G4LogicalVolume(solidCu1, Cu1Material, "Cu1");
  new G4PVPlacement(nullptr, origin, logicCu1, "Cu1", logicWorld, false, 0, true);

  // Outer copper shell (Cu2)
  auto solidCu2_outer = new G4Box("Cu2_outer",
      cu2_outer_half, cu2_outer_half, cu2_outer_half);
  auto solidCu2_inner = new G4Box("Cu2_inner",
      cu2_inner_half, cu2_inner_half, cu2_inner_half);
  auto solidCu2 = new G4SubtractionSolid("Cu2", solidCu2_outer, solidCu2_inner);
  auto logicCu2 = new G4LogicalVolume(solidCu2, Cu2Material, "Cu2");
  new G4PVPlacement(nullptr, origin, logicCu2, "Cu2", logicWorld, false, 0, true);

  // Inner lead shell (Pb1)
  auto solidPb1_outer = new G4Box("Pb1_outer",
      pb1_outer_half, pb1_outer_half, pb1_outer_half);
  auto solidPb1_inner = new G4Box("Pb1_inner",
      pb1_inner_half, pb1_inner_half, pb1_inner_half);
  auto solidPb1 = new G4SubtractionSolid("Pb1", solidPb1_outer, solidPb1_inner);
  auto logicPb1 = new G4LogicalVolume(solidPb1, Pb1Material, "Pb1");
  new G4PVPlacement(nullptr, origin, logicPb1, "Pb1", logicWorld, false, 0, true);

  // Outer lead shell (Pb2)
  auto solidPb2_outer = new G4Box("Pb2_outer",
      pb2_outer_half, pb2_outer_half, pb2_outer_half);
  auto solidPb2_inner = new G4Box("Pb2_inner",
      pb2_inner_half, pb2_inner_half, pb2_inner_half);
  auto solidPb2 = new G4SubtractionSolid("Pb2", solidPb2_outer, solidPb2_inner);
  auto logicPb2 = new G4LogicalVolume(solidPb2, Pb2Material, "Pb2");
  new G4PVPlacement(nullptr, origin, logicPb2, "Pb2", logicWorld, false, 0, true);

  // ---------------- Visualization ----------------
  logicWorld->SetVisAttributes(new G4VisAttributes(G4Colour(0.9, 0.9, 0.9, 0.05)));

  auto hpgeVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0));  // solid red
  hpgeVis->SetForceSolid(true);
  logicHPGe->SetVisAttributes(hpgeVis);

  auto cu1Vis = new G4VisAttributes(G4Colour(0.8, 0.4, 0.1, 0.2));
  cu1Vis->SetForceSolid(true);
  logicCu1->SetVisAttributes(cu1Vis);

  auto cu2Vis = new G4VisAttributes(G4Colour(0.9, 0.5, 0.1, 0.15));
  cu2Vis->SetForceSolid(true);
  logicCu2->SetVisAttributes(cu2Vis);

  auto pb1Vis = new G4VisAttributes(G4Colour(0.4, 0.4, 0.4, 0.15));
  pb1Vis->SetForceSolid(true);
  logicPb1->SetVisAttributes(pb1Vis);

  auto pb2Vis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 0.1));
  pb2Vis->SetForceSolid(true);
  logicPb2->SetVisAttributes(pb2Vis);

  // ---------------- Sensitive detectors ----------------
  auto sdManager = G4SDManager::GetSDMpointer();
  auto shieldSD = new SensitiveDetector("ShieldSD",
      "/home/bmiles/miniconda3/envs/geant4_env/share/Geant4/bramGeant4/hPGeShield/gammas/gamma295kev.txt");
  sdManager->AddNewDetector(shieldSD);

  logicHPGe->SetSensitiveDetector(shieldSD);
  logicCu1->SetSensitiveDetector(shieldSD);
  logicCu2->SetSensitiveDetector(shieldSD);
  logicPb1->SetSensitiveDetector(shieldSD);
  logicPb2->SetSensitiveDetector(shieldSD);

  // Add volume listing
  G4PhysicalVolumeStore* store = G4PhysicalVolumeStore::GetInstance();
  G4cout << "=== Physical Volumes in Geometry ===" << G4endl;
  for (auto pv : *store) {
      G4cout << pv->GetName() << G4endl;
  }
  G4cout << "====================================" << G4endl;

  return physWorld;
}

