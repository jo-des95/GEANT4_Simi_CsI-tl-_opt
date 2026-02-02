#ifndef EVENT_ACTION_HH
#define EVENT_ACTION_HH

#include "G4UserEventAction.hh"
#include "G4ThreeVector.hh"
#include "G4Event.hh"
#include "G4Types.hh"

#include <vector>
#include <fstream>

class EventAction : public G4UserEventAction {
public:
    EventAction();
    ~EventAction() override;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;

    void AddEdep(G4double edep, G4double x, G4double y, G4double z);
    void AddTrackLength(G4double length);
    void AddPhotonCount();

    void SetInitialMuonEnergy(G4double energy) { fInitialMuonEnergy = energy; }
    void SetFinalMuonEnergy(G4double energy)   { fFinalMuonEnergy   = energy; }

    // Access to debug output stream
    std::ofstream& GetDebugStream() { return fDebugStream; }

private:
    G4double           fTotalEdep;
    G4double           fTrackLength;
    G4int              fPhotonCount;
    G4double           fInitialMuonEnergy;
    G4double           fFinalMuonEnergy;
    std::vector<G4ThreeVector> fEdepPositions;

    // Debug file for optical photon tracking
    std::ofstream      fDebugStream;
};

#endif
