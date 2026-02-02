#include "run_action.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include <sstream>

RunAction::RunAction() : G4UserRunAction() {}

RunAction::~RunAction() {
    auto analysis = G4AnalysisManager::Instance();
    if (analysis) {
        analysis->Write();
        analysis->CloseFile(false);
    }
}

void RunAction::BeginOfRunAction(const G4Run* run) {
    auto analysis = G4AnalysisManager::Instance();
    
    std::stringstream fileName;
    fileName << "muon_csi_simulation_run" << run->GetRunID() << "_t0.root";
    analysis->OpenFile(fileName.str());
    analysis->Reset();

    // Hits ntuple (Ntuple 0) - ΕΛΑΧΙΣΤΟ
    analysis->CreateNtuple("Hits", "Hit Data");
    analysis->CreateNtupleIColumn("EventID");
    analysis->CreateNtupleIColumn("TrackID");
    analysis->CreateNtupleDColumn("Energy");
    analysis->FinishNtuple();

    // PMT ntuple (Ntuple 1) - ΕΛΑΧΙΣΤΟ
    analysis->CreateNtuple("PMTHits", "PMT Hit Data");
    analysis->CreateNtupleIColumn("EventID");
    analysis->CreateNtupleIColumn("PhotonID");
    analysis->CreateNtupleDColumn("Energy");
    analysis->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run*) {
    auto analysis = G4AnalysisManager::Instance();
    if (analysis) analysis->Write();
}
