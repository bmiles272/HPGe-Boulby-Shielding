#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleSource = new G4GeneralParticleSource();
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleSource;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    // G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    // G4String particleName = "gamma";
    // G4ParticleDefinition* particle = particleTable->FindParticle(particleName);
    // fParticleGun->SetParticleDefinition(particle);
    // fParticleGun->SetParticleEnergy(295.2 * keV); //gamma peak from pb-214
    // fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));
    // fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 70*cm));

    fParticleSource->GeneratePrimaryVertex(anEvent);
}