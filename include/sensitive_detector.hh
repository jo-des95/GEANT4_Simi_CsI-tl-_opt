#ifndef SENSITIVE_DETECTOR_HH
#define SENSITIVE_DETECTOR_HH

#include "G4VSensitiveDetector.hh"

class EventAction;

class CrystalSD : public G4VSensitiveDetector 
{
public:
    CrystalSD(const G4String& name);
    ~CrystalSD() override;
    G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    void SetEventAction(EventAction* ea) { fEventAction = ea; }

private:
    EventAction* fEventAction;
};

#endif
