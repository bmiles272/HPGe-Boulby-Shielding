#ifndef RADIOIMPURITIES_HH
#define RADIOIMPURITIES_HH

#include "G4Material.hh"
#include "G4NistManager.hh"

namespace RadioImpurities {

  G4Material* CreateUltraPureCopper();   
  G4Material* CreateImpureCopper();      
  G4Material* CreateLowBackgroundLead(); 
  G4Material* CreateImpureLead();

}

#endif
