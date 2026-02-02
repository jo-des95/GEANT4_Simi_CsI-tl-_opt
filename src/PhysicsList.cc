#include "PhysicsList.hh"

#include "globals.hh"
#include "G4ios.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"

#include "G4BosonConstructor.hh"
#include "G4LeptonConstructor.hh"
#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"
#include "G4IonConstructor.hh"

#include "G4LossTableManager.hh"
#include "G4EmConfigurator.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"

// ===== OPTICAL PHYSICS =====
#include "G4OpticalPhysics.hh"

#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4PAIModel.hh"
#include "G4PAIPhotModel.hh"

PhysicsList::PhysicsList() : G4VModularPhysicsList() {
    defaultCutValue = 1.0*mm;
    fConfig = G4LossTableManager::Instance()->EmConfigurator();
    SetVerboseLevel(1);
    
    ConstructParticle();
    
    // Electromagnetic physics
    RegisterPhysics(new G4EmStandardPhysics_option4());
    RegisterPhysics(new G4EmExtraPhysics());
    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4HadronElasticPhysics());
    RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());
    RegisterPhysics(new G4StoppingPhysics());
    RegisterPhysics(new G4IonPhysics());
    RegisterPhysics(new G4NeutronTrackingCut());
    
    // ===== OPTICAL PHYSICS (Simple version - compatible with older Geant4) =====
    RegisterPhysics(new G4OpticalPhysics());
    
    G4cout << "✅ Optical Physics ENABLED" << G4endl;
    
    // Add PAI model
    AddPAIModel("pai");
}

PhysicsList::~PhysicsList() {}

void PhysicsList::ConstructParticle() {
    // Build particle types
    G4BosonConstructor pBoson; pBoson.ConstructParticle();
    G4LeptonConstructor pLepton; pLepton.ConstructParticle();
    G4MesonConstructor pMeson; pMeson.ConstructParticle();
    G4BaryonConstructor pBaryon; pBaryon.ConstructParticle();
    G4IonConstructor pIon; pIon.ConstructParticle();
    G4ShortLivedConstructor pShort; pShort.ConstructParticle();
    
    // ===== OPTICAL PHOTON =====
    G4OpticalPhoton::OpticalPhotonDefinition();
}

void PhysicsList::SetCuts() {
    if (verboseLevel > 0) {
        G4cout << "PhysicsList::SetCuts:";
        G4cout << "CutLength : " << defaultCutValue/mm << " mm" << G4endl;
    }
    
    SetCutValue(defaultCutValue, "gamma");
    SetCutValue(defaultCutValue, "e-");
    SetCutValue(defaultCutValue, "e+");
    SetCutValue(defaultCutValue, "proton");
    
    // No cut for optical photons
    SetCutValue(0.0, "opticalphoton");
    
    if (verboseLevel > 0) DumpCutValuesTable();
}

void PhysicsList::AddPAIModel(const G4String& modname) {
    auto* theParticleIterator = G4ParticleTable::GetParticleTable()->GetIterator();
    theParticleIterator->reset();
    
    while ((*theParticleIterator)()) {
        G4ParticleDefinition* particle = theParticleIterator->value();
        G4String partname = particle->GetParticleName();
        if(partname == "e-" || partname == "e+") {
            NewPAIModel(particle, modname, "eIoni");
        } else if(partname == "mu-" || partname == "mu+") {
            NewPAIModel(particle, modname, "muIoni");
        } else if(partname == "proton" || partname == "pi+" || partname == "pi-") {
            NewPAIModel(particle, modname, "hIoni");
        }
    }
}

void PhysicsList::NewPAIModel(const G4ParticleDefinition* part,
                             const G4String& modname,
                             const G4String& procname) {
    G4String partname = part->GetParticleName();
    if(modname == "pai") {
        G4PAIModel* pai = new G4PAIModel(part,"PAIModel");
        fConfig->SetExtraEmModel(partname, procname, pai, "Crystal", 0.0, 100.*GeV);
    } else if(modname == "pai_photon") {
        G4PAIPhotModel* pai = new G4PAIPhotModel(part,"PAIPhotModel");
        fConfig->SetExtraEmModel(partname, procname, pai, "Crystal", 0.0, 100.*TeV);
    }
}
