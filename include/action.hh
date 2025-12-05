#ifndef ACTION_HH
#define ACTION_HH

#include "G4VUserActionInitialization.hh"
#include "generator.hh"

class MyActionInitialization : public G4VUserActionInitialization
{
public:
    MyActionInitialization(detectorShielding* det);
    virtual ~MyActionInitialization();

    virtual void Build() const override;
    
private:
    detectorShielding* fDet;
};
#endif