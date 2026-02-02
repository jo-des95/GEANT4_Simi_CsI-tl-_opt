#include "G4MTRunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "detector_construction.hh"
#include "action_initialization.hh"
#include "PhysicsList.hh"

int main(int argc, char* argv[])
{
    // Setup Run Manager
    auto runManager = new G4MTRunManager;
    runManager->SetNumberOfThreads(1);

    // Initializations
    runManager->SetUserInitialization(new DetectorConstruction);
    runManager->SetUserInitialization(new PhysicsList);
    runManager->SetUserInitialization(new ActionInitialization);
    runManager->Initialize();

    // Visualization - ONLY in interactive mode
    G4VisExecutive* visMgr = nullptr;
    if (argc == 1) {
        // Interactive mode: enable visualization
        visMgr = new G4VisExecutive;
        visMgr->Initialize();
        G4UImanager::GetUIpointer()->ApplyCommand("/control/execute initvis.mac");
    } else {
        // Batch mode: skip visualization (FASTER & LIGHTER)
        // No visualization overhead, no trajectory storage
    }

    // UI
    G4UIExecutive* ui = nullptr;
    if (argc == 1) {
        ui = new G4UIExecutive(argc, argv);
    }

    // Process macro files in batch mode
    if (argc > 1) {
        G4UImanager::GetUIpointer()->ApplyCommand(G4String(argv[1]));
    }

    // Interactive session
    if (ui) {
        ui->SessionStart();
        delete ui;
    }

    // Cleanup
    if (visMgr) delete visMgr;
    delete runManager;

    return 0;
}
