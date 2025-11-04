#include "radioImpurities.hh"
#include "G4SystemOfUnits.hh"
#include "G4Isotope.hh"
#include "G4Element.hh"
#include "G4Material.hh"

namespace RadioImpurities {

  static G4Isotope* MakeIsotope(const G4String& name, G4int Z, G4int A, G4double mass) {
    return new G4Isotope(name, Z, A, mass*g/mole);
  }

  // puer copper
  G4Material* CreateUltraPureCopper() {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* Cu = nist->FindOrBuildMaterial("G4_Cu");

    G4Material* ultraCu = new G4Material("UltraPureCopper", 8.96*g/cm3, 1);
    ultraCu->AddMaterial(Cu, 100.*perCent);
    return ultraCu;
  }

  // less pure copper
  G4Material* CreateImpureCopper() {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* Cu = nist->FindOrBuildMaterial("G4_Cu");

    // impurities U-238 Th-232 Co-60
    G4Isotope* U238 = MakeIsotope("U238", 92, 238, 238.051);
    G4Isotope* Th232 = MakeIsotope("Th232", 90, 232, 232.038);
    G4Isotope* Co60 = MakeIsotope("Co60", 27, 60, 59.9338);

    G4Element* elU = new G4Element("Uranium", "U", 1);
    elU->AddIsotope(U238, 100.*perCent);

    G4Element* elTh = new G4Element("Thorium", "Th", 1);
    elTh->AddIsotope(Th232, 100.*perCent);

    G4Element* elCo60 = new G4Element("Cobalt60", "Co60", 1);
    elCo60->AddIsotope(Co60, 100.*perCent);

    G4Material* impureCu = new G4Material("ImpureCopper", 8.96*g/cm3, 4);
    impureCu->AddMaterial(Cu, 99.999999*perCent); 
    impureCu->AddElement(elU,   1e-8*perCent);    // ~0.1 Bq/kg
    impureCu->AddElement(elTh,  1e-8*perCent);    // ~0.1 Bq/kg
    impureCu->AddElement(elCo60,1e-7*perCent);    // ~1 Bq/kg

    return impureCu;
  }

  // pure-ish lead
  G4Material* CreateLowBackgroundLead() {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* Pb = nist->FindOrBuildMaterial("G4_Pb");

    G4Isotope* Pb210 = MakeIsotope("Pb210", 82, 210, 209.984);
    G4Element* elPb210 = new G4Element("Lead210", "Pb210", 1);
    elPb210->AddIsotope(Pb210, 100.*perCent);

    G4Material* lowPb = new G4Material("LowBackgroundLead", 11.34*g/cm3, 2);
    lowPb->AddMaterial(Pb, 99.999999*perCent);
    lowPb->AddElement(elPb210, 1e-8*perCent); // ~10 Bq/kg

    return lowPb;
  }

  // ipure lead
  G4Material* CreateImpureLead() {
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* Pb = nist->FindOrBuildMaterial("G4_Pb");

    G4Isotope* Pb210 = MakeIsotope("Pb210", 82, 210, 209.984);
    G4Element* elPb210 = new G4Element("Lead210", "Pb210", 1);
    elPb210->AddIsotope(Pb210, 100.*perCent);

    G4Material* impurePb = new G4Material("ImpureLead", 11.34*g/cm3, 2);
    impurePb->AddMaterial(Pb, 99.999*perCent);
    impurePb->AddElement(elPb210, 0.001*perCent); // ~1000 Bq/kg

    return impurePb;
  }

}
