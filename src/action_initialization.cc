#include "action_initialization.hh"
#include "primary_generator.hh"
#include "event_action.hh"
#include "run_action.hh"
#include "stepping_action.hh"
#include "tracking_action.hh"
#include "G4RunManager.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    // Only run-level actions needed for master thread
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    // Event-level actions
    auto* eventAction = new EventAction();
    SetUserAction(eventAction);

    // Primary generator
    SetUserAction(new PrimaryGenerator());

    // Stepping or Tracking
    SetUserAction(new SteppingAction(eventAction));
    SetUserAction(new TrackingAction(eventAction));

    // Run action
    SetUserAction(new RunAction());
}
