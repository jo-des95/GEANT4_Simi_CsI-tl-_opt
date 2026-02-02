#include "pmt_sd.hh"
#include "event_action.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

PMTSD::PMTSD(const G4String& name)
    : G4VSensitiveDetector(name), fEventAction(nullptr) {}

PMTSD::~PMTSD() {}

G4bool PMTSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    
    auto track = step->GetTrack();
    
    // Only detect optical photons
    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
        return false;
    }
    
    // Get event ID and photon energy
    auto evt = G4RunManager::GetRunManager()->GetCurrentEvent();
    G4int eid = evt ? evt->GetEventID() : 0;
    G4double photonEnergy = track->GetKineticEnergy();
    
    // Fill ntuple with photon hit data
    auto analysis = G4AnalysisManager::Instance();
    analysis->FillNtupleIColumn(1, 0, eid);
    analysis->FillNtupleIColumn(1, 1, track->GetTrackID());
    analysis->FillNtupleDColumn(1, 2, photonEnergy / eV);
    analysis->AddNtupleRow(1);
    
    // Update event statistics
    if (fEventAction) {
        fEventAction->AddPhotonCount();
    }
    
    // ===== KILL THE PHOTON =====
    // After registering the hit, stop tracking this photon
    track->SetTrackStatus(fStopAndKill);
    
    G4cout << "🔴 Optical photon detected and killed at PMT: E = " 
           << photonEnergy / eV << " eV" << G4endl;
    
    return true;
}
