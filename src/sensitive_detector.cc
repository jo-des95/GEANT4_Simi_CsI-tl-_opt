#include "sensitive_detector.hh"
#include "event_action.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4EventManager.hh"

CrystalSD::CrystalSD(const G4String& name) 
    : G4VSensitiveDetector(name), fEventAction(nullptr) {}

CrystalSD::~CrystalSD() {}

G4bool CrystalSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
    G4Track* track = step->GetTrack();
    G4double edep = step->GetTotalEnergyDeposit();
    
    if (edep == 0.) return false;

    auto analysis = G4AnalysisManager::Instance();
    G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    
    analysis->FillNtupleIColumn(0, 0, eventID);
    analysis->FillNtupleIColumn(0, 1, track->GetTrackID());
    analysis->FillNtupleDColumn(0, 2, edep/MeV);
    analysis->AddNtupleRow(0);

    if (fEventAction) {
        G4ThreeVector pos = step->GetPreStepPoint()->GetPosition();
        fEventAction->AddEdep(edep, pos.x(), pos.y(), pos.z());
    }

    return true;
}
