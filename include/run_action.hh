#ifndef RUN_ACTION_HH
#define RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "G4SystemOfUnits.hh"

class RunAction : public G4UserRunAction {
public:
    RunAction();
    ~RunAction();
    
    virtual void BeginOfRunAction(const G4Run* run);
    virtual void EndOfRunAction(const G4Run* run);
    
    // ✅ Safe flush after each event
    void FlushPerEvent();
    
private:
    int eventCounter;
};

#endif
