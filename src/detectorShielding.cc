#include "detectorShielding.hh"
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "radioImpurities.hh"
#include "G4SDManager.hh"
#include "sensitiveDetector.hh"
#include "G4SubtractionSolid.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4RotationMatrix.hh"

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------
detectorShielding::detectorShielding()
 : G4VUserDetectorConstruction(),
   fHPGeHeight(45*mm),
   fHPGeDiam(46.5*mm),
   fInnerCu1Thickness(5*mm),
   fInnerCu2Thickness(20*mm),
   fOuterPb1Thickness(50*mm),
   fOuterPb2Thickness(150*mm),
   fCavityHalfX(50*mm),
   fCavityHalfY(50*mm),
   fCavityHalfZ(50*mm),
   fSimTime(0),
   fTotalDecays(0),
   fGeometryDirty(false),
   fMessenger(nullptr)
{
    G4cout << "[DEBUG] Registering setLayerActivity command" << G4endl;

    layerMap["Cu1"]  = 0;
    layerMap["Cu2"] = 1;
    layerMap["Pb1"] = 2;
    layerMap["Pb2"] = 3;

    // Create UI messenger
    fMessenger = new G4GenericMessenger(this, "/Shielding/", "Shielding controls");

    // /Shielding/setTime <seconds>
    fMessenger->DeclareMethod("setTime",
        &detectorShielding::SetSimulationTime)
        .SetGuidance("Set the simulation time in seconds.")
        .SetParameterName("time", false);

    fMessenger->DeclareMethod("setCu1Activity", &detectorShielding::SetCu1Activity)
        .SetGuidance("Set Cu1 activity in Bq/kg")
        .SetParameterName("activity", true);

    fMessenger->DeclareMethod("setCu2Activity", &detectorShielding::SetCu2Activity)
        .SetGuidance("Set Cu2 activity in Bq/kg")
        .SetParameterName("activity", true);

    fMessenger->DeclareMethod("setPb1Activity", &detectorShielding::SetPb1Activity)
        .SetGuidance("Set Pb1 activity in Bq/kg")
        .SetParameterName("activity", true);

    fMessenger->DeclareMethod("setPb2Activity", &detectorShielding::SetPb2Activity)
        .SetGuidance("Set Pb2 activity in Bq/kg")
        .SetParameterName("activity", true);
    // /Shielding/autoBeamOn
    fMessenger->DeclareMethod("autoBeamOn",
            &detectorShielding::AutoBeamOn)
        .SetGuidance("Automatically runs beamOn with the total computed decays.");
    
    fMessenger->DeclareMethod("setDecays",
            &detectorShielding::SetTotalDecays)
        .SetGuidance("Directly set total number of decays to simulate.")
        .SetParameterName("decays", false);

    fMessenger->DeclareMethodWithUnit("Cu1Thickness", "mm", 
        &detectorShielding::SetInnerCu1Thickness)
        .SetGuidance("Set inner Cu1 thickness in mm")
        .SetParameterName("thickness", false);

    fMessenger->DeclareMethodWithUnit("Cu2Thickness", "mm", 
        &detectorShielding::SetInnerCu2Thickness)
        .SetGuidance("Set inner Cu2 thickness in mm")
        .SetParameterName("thickness", false);

    fMessenger->DeclareMethodWithUnit("Pb1Thickness", "mm", 
        &detectorShielding::SetOuterPb1Thickness)
        .SetGuidance("Set outer Pb1 thickness in mm")
        .SetParameterName("thickness", false);

    fMessenger->DeclareMethodWithUnit("Pb2Thickness", "mm", 
        &detectorShielding::SetOuterPb2Thickness)
        .SetGuidance("Set outer Pb2 thickness in mm")
        .SetParameterName("thickness", false);

    // Cavity dimension setters
    fMessenger->DeclareMethodWithUnit("cavityHalfX", "mm", 
        &detectorShielding::SetCavityHalfX)
        .SetGuidance("Set cavity half X dimension in mm")
        .SetParameterName("halfX", false);

    fMessenger->DeclareMethodWithUnit("cavityHalfY", "mm", 
        &detectorShielding::SetCavityHalfY)
        .SetGuidance("Set cavity half Y dimension in mm")
        .SetParameterName("halfY", false);

    fMessenger->DeclareMethodWithUnit("cavityHalfZ", "mm", 
        &detectorShielding::SetCavityHalfZ)
        .SetGuidance("Set cavity half Z dimension in mm")
        .SetParameterName("halfZ", false);
}

detectorShielding::~detectorShielding()
{
    delete fMessenger;
}

// ------------------------------------------------------------
// Construct geometry
// ------------------------------------------------------------
G4VPhysicalVolume* detectorShielding::Construct()
{
    if (fGeometryDirty) {
        G4cout << "[Shielding] Geometry parameters changed, cleaning up old geometry..." << G4endl;
        
        G4PhysicalVolumeStore* pvStore = G4PhysicalVolumeStore::GetInstance();
        pvStore->Clean();
        
        fGeometryDirty = false;
        
        G4cout << "[Shielding] Old geometry cleaned, building new geometry..." << G4endl;
    }
    return DefineVolumes();
}

// ------------------------------------------------------------
// Build geometry
// ------------------------------------------------------------
G4VPhysicalVolume* detectorShielding::DefineVolumes()
{
    G4NistManager* nist = G4NistManager::Instance();

    // Materials (from your RadioImpurities class)
    G4Material* Cu1Material = RadioImpurities::CreateUltraPureCopper();
    G4Material* Cu2Material = RadioImpurities::CreateImpureCopper();
    G4Material* Pb1Material = RadioImpurities::CreateLowBackgroundLead();
    G4Material* Pb2Material = RadioImpurities::CreateImpureLead();

    G4Material* Ge  = nist->FindOrBuildMaterial("G4_Ge");
    G4Material* Air = nist->FindOrBuildMaterial("G4_AIR");

    // World
    G4double worldSize = 300 * cm;
    auto solidWorld = new G4Box("World", worldSize/2, worldSize/2, worldSize/2);
    auto logicWorld = new G4LogicalVolume(solidWorld, Air, "World");
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    auto physWorld  = new G4PVPlacement(nullptr, {}, logicWorld, "World", nullptr, false, 0, true);

    // HPGe
    auto solidHPGe = new G4Tubs("HPGe", 0, fHPGeDiam, fHPGeHeight, 90.0 * deg, 360.0 * deg);
    auto logicHPGe = new G4LogicalVolume(solidHPGe, Ge, "HPGe");
    auto rot = new G4RotationMatrix();
    rot->rotateX(90.0 * deg);
    new G4PVPlacement(rot, {}, logicHPGe, "HPGe", logicWorld, false, 0, true);

    // --------------------------------------------------------
    // Compute cubic shells
    // --------------------------------------------------------

    G4double maxInner = std::max(
        {fHPGeHeight + fCavityHalfX, fHPGeDiam + fCavityHalfY});

    G4double cu1_inner = maxInner;
    G4double cu1_outer = cu1_inner + fInnerCu1Thickness;

    G4double cu2_inner = cu1_outer;
    G4double cu2_outer = cu2_inner + fInnerCu2Thickness;

    G4double pb1_inner = cu2_outer;
    G4double pb1_outer = pb1_inner + fOuterPb1Thickness;

    G4double pb2_inner = pb1_outer;
    G4double pb2_outer = pb2_inner + fOuterPb2Thickness;

    auto hpgeVis = new G4VisAttributes(G4Colour(1,0,0,1));
    auto cu1Vis = new G4VisAttributes(G4Colour(0.8,0.4,0.1,0.3)); 
    auto cu2Vis = new G4VisAttributes(G4Colour(0.9,0.5,0.1,0.3));   
    auto pb1Vis = new G4VisAttributes(G4Colour(0.4,0.4,0.4,0.25)); 
    auto pb2Vis = new G4VisAttributes(G4Colour(0.2,0.2,0.2,0.2));

    hpgeVis->SetForceSolid(true);
    cu1Vis->SetForceSolid(true);
    cu2Vis->SetForceSolid(true);
    pb1Vis->SetForceSolid(true);
    pb2Vis->SetForceSolid(true);

    auto makeShell = [&](G4String name, G4double inner, G4double outer, G4Material* material, G4VisAttributes* visAttr)
    {
        auto outerBox = new G4Box(name+"_outer", outer, outer, outer);
        auto innerBox = new G4Box(name+"_inner", inner, inner, inner);
        auto shell = new G4SubtractionSolid(name, outerBox, innerBox);
        auto logic = new G4LogicalVolume(shell, material, name);

        logic->SetVisAttributes(visAttr);

        new G4PVPlacement(nullptr, {}, logic, name, logicWorld, false, 0, true);
        return logic;
    };

    logicHPGe->SetVisAttributes(hpgeVis);
    auto logicCu1 = makeShell("Cu1", cu1_inner, cu1_outer, Cu1Material, cu1Vis);
    auto logicCu2 = makeShell("Cu2", cu2_inner, cu2_outer, Cu2Material, cu2Vis);
    auto logicPb1 = makeShell("Pb1", pb1_inner, pb1_outer, Pb1Material, pb1Vis);
    auto logicPb2 = makeShell("Pb2", pb2_inner, pb2_outer, Pb2Material, pb2Vis);

    G4cout << "=== Geometry Debug Info ===" << G4endl;
    G4cout << "World size: " << worldSize/cm << " cm" << G4endl;
    G4cout << "HPGe position at origin" << G4endl;
    G4cout << "Shell dimensions:" << G4endl;
    G4cout << "  Cu1: inner=" << cu1_inner/cm << "cm, outer=" << cu1_outer/cm << "cm" << G4endl;
    G4cout << "  Cu2: inner=" << cu2_inner/cm << "cm, outer=" << cu2_outer/cm << "cm" << G4endl;
    G4cout << "  Pb1: inner=" << pb1_inner/cm << "cm, outer=" << pb1_outer/cm << "cm" << G4endl;
    G4cout << "  Pb2: inner=" << pb2_inner/cm << "cm, outer=" << pb2_outer/cm << "cm" << G4endl;

    return physWorld;
}

G4double detectorShielding::GetLayerMass(const G4String& layer)
{
    G4cout << "=== GetLayerMass Debug ===" << G4endl;
    G4cout << "Input layer: '" << layer << "'" << G4endl;

    // Determine inner & outer half-lengths
    G4double maxInner = std::max(
        {fHPGeHeight + fCavityHalfX, fHPGeDiam + fCavityHalfY});

    G4cout << "maxInner calculation:" << G4endl;
    G4cout << "  fHPGeHalfX + fCavityHalfX = " << fHPGeDiam << " + " << fCavityHalfX 
           << " = " << fHPGeDiam + fCavityHalfX << G4endl;
    G4cout << "  fHPGeHalfY + fCavityHalfY = " << fHPGeHeight << " + " << fCavityHalfY 
           << " = " << fHPGeHeight + fCavityHalfY << G4endl;
    G4cout << "  fHPGeHalfZ + fCavityHalfZ = " << fHPGeDiam << " + " << fCavityHalfZ 
           << " = " << fHPGeDiam + fCavityHalfZ << G4endl;
    G4cout << "  maxInner = " << maxInner << G4endl;

    G4double inner = 0;
    G4double outer = 0;
    G4Material* mat = nullptr;

    if (layer == "Cu1") {
        inner = maxInner;
        outer = inner + fInnerCu1Thickness;
        mat = G4Material::GetMaterial("G4_Cu");
        G4cout << "Processing Cu1 layer:" << G4endl;
        G4cout << "  inner = " << inner << G4endl;
        G4cout << "  fInnerCu1Thickness = " << fInnerCu1Thickness << G4endl;
        G4cout << "  outer = " << inner << " + " << fInnerCu1Thickness << " = " << outer << G4endl;
    }
    else if (layer == "Cu2") {
        inner = maxInner + fInnerCu1Thickness;
        outer = inner + fInnerCu2Thickness;
        mat = G4Material::GetMaterial("G4_Cu");
        G4cout << "Processing Cu2 layer:" << G4endl;
        G4cout << "  inner = " << maxInner << " + " << fInnerCu1Thickness << " = " << inner << G4endl;
        G4cout << "  fInnerCu2Thickness = " << fInnerCu2Thickness << G4endl;
        G4cout << "  outer = " << inner << " + " << fInnerCu2Thickness << " = " << outer << G4endl;
    }
    else if (layer == "Pb1") {
        inner = maxInner + fInnerCu1Thickness + fInnerCu2Thickness;
        outer = inner + fOuterPb1Thickness;
        mat = G4Material::GetMaterial("G4_Pb");
        G4cout << "Processing Pb1 layer:" << G4endl;
        G4cout << "  inner = " << maxInner << " + " << fInnerCu1Thickness << " + " 
               << fInnerCu2Thickness << " = " << inner << G4endl;
        G4cout << "  fOuterPb1Thickness = " << fOuterPb1Thickness << G4endl;
        G4cout << "  outer = " << inner << " + " << fOuterPb1Thickness << " = " << outer << G4endl;
    }
    else if (layer == "Pb2") {
        inner = maxInner + fInnerCu1Thickness + fInnerCu2Thickness + fOuterPb1Thickness;
        outer = inner + fOuterPb2Thickness;
        mat = G4Material::GetMaterial("G4_Pb");
        G4cout << "Processing Pb2 layer:" << G4endl;
        G4cout << "  inner = " << maxInner << " + " << fInnerCu1Thickness << " + " 
               << fInnerCu2Thickness << " + " << fOuterPb1Thickness << " = " << inner << G4endl;
        G4cout << "  fOuterPb2Thickness = " << fOuterPb2Thickness << G4endl;
        G4cout << "  outer = " << inner << " + " << fOuterPb2Thickness << " = " << outer << G4endl;
    }
    else {
        G4cout << "ERROR: Unknown layer name '" << layer << "'" << G4endl;
        G4Exception("GetLayerMass","BadLayer",FatalException,"Bad layer name.");
    }

    // Debug material lookup
    if (mat == nullptr) {
        G4cout << "ERROR: Material is null for layer '" << layer << "'!" << G4endl;
        G4Exception("GetLayerMass","NullMaterial",FatalException,"Material not found.");
    } else {
        G4cout << "Material: " << mat->GetName() << G4endl;
        G4cout << "Material density: " << mat->GetDensity()/(kg/m3) << " kg/m3" << G4endl;
    }

    // shell volume calculation
    G4double vol_mm3 = (std::pow(outer,3) - std::pow(inner,3)) * 8.0;
    G4double vol_m3 = vol_mm3 * 1e-9;

    G4cout << "Volume calculation:" << G4endl;
    G4cout << "  outer^3 = " << std::pow(outer,3) << G4endl;
    G4cout << "  inner^3 = " << std::pow(inner,3) << G4endl;
    G4cout << "  (outer^3 - inner^3) = " << (std::pow(outer,3) - std::pow(inner,3)) << G4endl;
    G4cout << "  vol_mm3 = " << (std::pow(outer,3) - std::pow(inner,3)) << " * 8.0 = " << vol_mm3 << " mm3" << G4endl;
    G4cout << "  vol_m3 = " << vol_mm3 << " * 1e-9 = " << vol_m3 << " m3" << G4endl;

    G4double mass = mat->GetDensity()/(kg/m3) * vol_m3;
    
    G4cout << "Mass calculation:" << G4endl;
    G4cout << "  mass = " << mat->GetDensity()/(kg/m3) << " * " << vol_m3 << " = " << mass << " kg" << G4endl;
    G4cout << "=== End GetLayerMass Debug ===" << G4endl << G4endl;

    return mass;
}

// ------------------------------------------------------------
// Simulation time
// ------------------------------------------------------------
void detectorShielding::SetSimulationTime(G4double time)
{
    fSimTime = time;
    G4cout << "[Shielding] Simulation time set to " << fSimTime << " s" << G4endl;
}



void detectorShielding::SetCu1Activity(G4double activityPerKg)
{
    SetLayerActivityForName("Cu1", activityPerKg);
}

void detectorShielding::SetCu2Activity(G4double activityPerKg)
{
    SetLayerActivityForName("Cu2", activityPerKg);
}

void detectorShielding::SetPb1Activity(G4double activityPerKg)
{
    SetLayerActivityForName("Pb1", activityPerKg);
}

void detectorShielding::SetPb2Activity(G4double activityPerKg)
{
    SetLayerActivityForName("Pb2", activityPerKg);
}

void detectorShielding::SetLayerActivityForName(const G4String& name, G4double activityPerKg)
{
    G4double mass = GetLayerMass(name);
    G4double activity = mass * activityPerKg;
    G4int decays = static_cast<G4int>(activity * fSimTime);

    G4cout << "[Shielding] Layer: " << name
           << ", Mass: " << mass << " kg"
           << ", Activity = " << activity << " Bq" 
           << ", Decays = " << decays << G4endl;

    G4cout << "[DEBUG] Before adding: fTotalDecays = " << fTotalDecays << G4endl;
    fTotalDecays += decays;
    G4cout << "[DEBUG] After adding: fTotalDecays = " << fTotalDecays << G4endl;
}

//G4int GetTotalDecays() const { return fTotalDecays; }

void detectorShielding::AutoBeamOn()
{
    G4cout << "[AutoBeamOn] fTotalDecays = " << fTotalDecays << G4endl;
    
    if (fTotalDecays <= 0) {
        G4Exception("AutoBeamOn", "NoDecays", JustWarning,
                    "Total decays = 0. Did you set simulation time and activities?");
        return;
    }

    G4cout << "[Shielding] AutoBeamOn launching: " 
           << fTotalDecays << " events" << G4endl;

    G4RunManager::GetRunManager()->BeamOn(fTotalDecays);
}

// ------------------------------------------------------------
// Thickness setters with validation
// ------------------------------------------------------------
void detectorShielding::SetInnerCu1Thickness(G4double thickness)
{
    if (thickness <= 0) {
        G4Exception("SetInnerCu1Thickness", "InvalidThickness", JustWarning,
                   "Inner Cu1 thickness must be positive. Using default 5mm.");
        fInnerCu1Thickness = 5*mm;
    } else {
        fInnerCu1Thickness = thickness;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Inner Cu1 thickness set to " << fInnerCu1Thickness/mm << " mm" << G4endl;
}

void detectorShielding::SetInnerCu2Thickness(G4double thickness)
{
    if (thickness <= 0) {
        G4Exception("SetInnerCu2Thickness", "InvalidThickness", JustWarning,
                   "Inner Cu2 thickness must be positive. Using default 20mm.");
        fInnerCu2Thickness = 20*mm;
    } else {
        fInnerCu2Thickness = thickness;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Inner Cu2 thickness set to " << fInnerCu2Thickness/mm << " mm" << G4endl;
}

void detectorShielding::SetOuterPb1Thickness(G4double thickness)
{
    if (thickness <= 0) {
        G4Exception("SetOuterPb1Thickness", "InvalidThickness", JustWarning,
                   "Outer Pb1 thickness must be positive. Using default 50mm.");
        fOuterPb1Thickness = 50*mm;
    } else {
        fOuterPb1Thickness = thickness;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Outer Pb1 thickness set to " << fOuterPb1Thickness/mm << " mm" << G4endl;
}

void detectorShielding::SetOuterPb2Thickness(G4double thickness)
{
    if (thickness <= 0) {
        G4Exception("SetOuterPb2Thickness", "InvalidThickness", JustWarning,
                   "Outer Pb2 thickness must be positive. Using default 150mm.");
        fOuterPb2Thickness = 150*mm;
    } else {
        fOuterPb2Thickness = thickness;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Outer Pb2 thickness set to " << fOuterPb2Thickness/mm << " mm" << G4endl;
}

void detectorShielding::SetCavityHalfX(G4double halfX)
{
    if (halfX <= 0) {
        G4Exception("SetCavityHalfX", "InvalidDimension", JustWarning,
                   "Cavity half X must be positive. Using default 50mm.");
        fCavityHalfX = 50*mm;
    } else {
        fCavityHalfX = halfX;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Cavity half X set to " << fCavityHalfX/mm << " mm" << G4endl;
}

void detectorShielding::SetCavityHalfY(G4double halfY)
{
    if (halfY <= 0) {
        G4Exception("SetCavityHalfY", "InvalidDimension", JustWarning,
                   "Cavity half Y must be positive. Using default 10mm.");
        fCavityHalfY = 10*mm;
    } else {
        fCavityHalfY = halfY;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Cavity half Y set to " << fCavityHalfY/mm << " mm" << G4endl;
}

void detectorShielding::SetCavityHalfZ(G4double halfZ)
{
    if (halfZ <= 0) {
        G4Exception("SetCavityHalfZ", "InvalidDimension", JustWarning,
                   "Cavity half Z must be positive. Using default 60mm.");
        fCavityHalfZ = 60*mm;
    } else {
        fCavityHalfZ = halfZ;
    }
    fGeometryDirty = true;
    G4cout << "[Shielding] Cavity half Z set to " << fCavityHalfZ/mm << " mm" << G4endl;
}

void detectorShielding::ConstructSDandField()
{
    G4SDManager* sdManager = G4SDManager::GetSDMpointer();
    
    // Create sensitive detector for HPGe
    SensitiveDetector* hpgeSD = new SensitiveDetector("HPGeSD");
    sdManager->AddNewDetector(hpgeSD);
    
    // Set the sensitive detector to the HPGe logical volume
    G4LogicalVolume* hpgeLV = G4LogicalVolumeStore::GetInstance()->GetVolume("HPGe");
    if (hpgeLV) {
        hpgeLV->SetSensitiveDetector(hpgeSD);
        G4cout << "[Shielding] Sensitive detector assigned to HPGe" << G4endl;
    } else {
        G4Exception("detectorShielding::ConstructSDandField", "SD001", 
                   FatalException, "HPGe logical volume not found!");
    }
}

void detectorShielding::SetTotalDecays(G4int decays)
{
    if (decays < 0) decays = 0;

    fTotalDecays = decays;

    G4cout << "[Shielding] Total decays manually set to "
           << fTotalDecays << G4endl;
}
