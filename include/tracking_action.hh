#ifndef TRACKING_ACTION_HH
#define TRACKING_ACTION_HH

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class EventAction;

class TrackingAction : public G4UserTrackingAction {
public:
    TrackingAction(EventAction* eventAction);
    virtual ~TrackingAction();

    virtual void PreUserTrackingAction(const G4Track* track) override;
    virtual void PostUserTrackingAction(const G4Track* track) override;

private:
    EventAction* fEventAction;
};

#endif
