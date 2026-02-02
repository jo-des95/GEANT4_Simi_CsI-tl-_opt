#include "event_action.hh"

#include "G4Event.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction()
    : G4UserEventAction(),
      fTotalEdep(0.),
      fTrackLength(0.),
      fPhotonCount(0),
      fInitialMuonEnergy(0.),
      fFinalMuonEnergy(0.)
{
    // Open debug file once per run
    fDebugStream.open("optical_debug.log");
}

EventAction::~EventAction()
{
    if (fDebugStream.is_open()) {
        fDebugStream.close();
    }
}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fTotalEdep        = 0.;
    fTrackLength      = 0.;
    fPhotonCount      = 0;
    fInitialMuonEnergy = 0.;
    fFinalMuonEnergy   = 0.;
    fEdepPositions.clear();
}

void EventAction::EndOfEventAction(const G4Event*)
{
    if (fTotalEdep > 0.) {
        G4cout << "Event: Edep = "    << fTotalEdep / MeV << " MeV, "
               << "TrackLength = "    << fTrackLength / mm << " mm, "
               << "Photons = "        << fPhotonCount
               << " InitialE = "      << fInitialMuonEnergy / GeV << " GeV"
               << G4endl;
    }
}

void EventAction::AddEdep(G4double edep, G4double x, G4double y, G4double z)
{
    fTotalEdep += edep;
    fEdepPositions.push_back(G4ThreeVector(x, y, z));
}

void EventAction::AddTrackLength(G4double length)
{
    fTrackLength += length;
}

void EventAction::AddPhotonCount()
{
    fPhotonCount++;
}
