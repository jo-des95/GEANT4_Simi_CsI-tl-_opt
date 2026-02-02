#pragma once

#include "G4VSensitiveDetector.hh"

class EventAction;

class PMTSD : public G4VSensitiveDetector {
public:
    PMTSD(const G4String& name);
    ~PMTSD() override;

    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    
    void SetEventAction(EventAction* ea) { fEventAction = ea; }

private:
    EventAction* fEventAction;
};
