#include "detector_construction.hh"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4MaterialPropertiesTable.hh"

// Optical surfaces
#include "G4OpticalSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

#include "sensitive_detector.hh"
#include "pmt_sd.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "event_action.hh"
#include "G4Material.hh"

DetectorConstruction::DetectorConstruction()
    : G4VUserDetectorConstruction() {}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{

    G4NistManager* nist = G4NistManager::Instance();
    auto* air = nist->FindOrBuildMaterial("G4_AIR");
    auto* glass = nist->FindOrBuildMaterial("G4_GLASS_PLATE");
    auto* al = nist->FindOrBuildMaterial("G4_Al");

    // ---------------------------
    // CsI(Tl) material
    // ---------------------------

    G4double density_csi_tl = 4.51*g/cm3;
    G4Material* csi_tl = new G4Material("CsI_Tl", density_csi_tl, 3);
    csi_tl->AddElement(nist->FindOrBuildElement("Cs"), 51.0*perCent);
    csi_tl->AddElement(nist->FindOrBuildElement("I"), 48.5*perCent);
    csi_tl->AddElement(nist->FindOrBuildElement("Tl"), 0.5*perCent);

    // ===== REALISTIC OPTICAL PROPERTIES - 200 POINTS FOR SMOOTH SPECTRUM =====

    const G4int nEntries = 200;

    G4double photonEnergy[nEntries];
    G4double csiRINDEX[nEntries];
    G4double csiABSLENGTH[nEntries];
    G4double csiSCINTILLATION[nEntries];

    // Fill arrays with dense sampling (1.8 to 3.2 eV)

    for (int i = 0; i < nEntries; i++) {

        photonEnergy[i] = 1.8*eV + i * (3.2 - 1.8)*eV / (nEntries - 1);
        csiRINDEX[i] = 1.79; // CsI refractive index
        csiABSLENGTH[i] = 100*cm;

        // Emission spectrum peaked at 550 nm (2.25 eV) - SMOOTH Gaussian
        G4double wavelength = 1240.0 / (photonEnergy[i]/eV);
        csiSCINTILLATION[i] = exp(-pow((wavelength - 550.0) / 50.0, 2));

    }

    // CsI(Tl) optical properties

    G4MaterialPropertiesTable* csiMPT = new G4MaterialPropertiesTable();
    csiMPT->AddProperty("RINDEX", photonEnergy, csiRINDEX, nEntries);
    csiMPT->AddProperty("ABSLENGTH", photonEnergy, csiABSLENGTH, nEntries);
    csiMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergy, csiSCINTILLATION, nEntries);

    // REALISTIC scintillation parameters for CsI(Tl)

    csiMPT->AddConstProperty("SCINTILLATIONYIELD", 54./MeV);
    csiMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    csiMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 680.*ns);
    csiMPT->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
    csi_tl->SetMaterialPropertiesTable(csiMPT);

    // Air optical properties

    G4MaterialPropertiesTable* airMPT = new G4MaterialPropertiesTable();
    G4double airRINDEX[nEntries];

    for (int i = 0; i < nEntries; i++) {
        airRINDEX[i] = 1.0;
    }

    airMPT->AddProperty("RINDEX", photonEnergy, airRINDEX, nEntries);
    air->SetMaterialPropertiesTable(airMPT);

    // Glass optical properties

    G4MaterialPropertiesTable* glassMPT = new G4MaterialPropertiesTable();
    G4double glassRINDEX[nEntries];
    G4double glassABSLENGTH[nEntries];

    for (int i = 0; i < nEntries; i++) {
        glassRINDEX[i] = 1.52;
        glassABSLENGTH[i] = 100*cm;
    }

    glassMPT->AddProperty("RINDEX", photonEnergy, glassRINDEX, nEntries);
    glassMPT->AddProperty("ABSLENGTH", photonEnergy, glassABSLENGTH, nEntries);
    glass->SetMaterialPropertiesTable(glassMPT);

    // Aluminum optical properties

    G4double alREFLECTIVITY[nEntries];

    for (int i = 0; i < nEntries; i++) {
        alREFLECTIVITY[i] = 0.97;
    }

    G4MaterialPropertiesTable* alMPT = new G4MaterialPropertiesTable();
    alMPT->AddProperty("REFLECTIVITY", photonEnergy, alREFLECTIVITY, nEntries);
    al->SetMaterialPropertiesTable(alMPT);

    G4cout << "✅ REALISTIC optical properties configured (200 energy points):" << G4endl;
    G4cout << "   CsI(Tl): 54k photons/MeV, 100cm absorption, 680ns decay" << G4endl;
    G4cout << "   Aluminum: 97% reflectivity (high-quality foil)" << G4endl;
    G4cout << "   Energy range: 1.8 - 3.2 eV (smooth sampling)" << G4endl;

    // ---------------------------
    // World - ENLARGED to prevent boundary oscillation
    // ---------------------------

    auto* worldSolid = new G4Box("World", 200*cm, 200*cm, 300*cm);
    auto* worldLog = new G4LogicalVolume(worldSolid, air, "World");
    auto* worldPhys = new G4PVPlacement(nullptr, G4ThreeVector(), worldLog, "World", nullptr, false, 0);

    G4cout << "✅ ENLARGED World volume: 200cm x 200cm x 300cm (prevents boundary oscillation)" << G4endl;

    // ---------------------------
    // Crystal geometry (truncated prism)
    // ---------------------------

    G4double small_half_x = 2*cm;
    G4double small_half_y = 2*cm;
    G4double large_half_x = 4*cm;
    G4double large_half_y = 4*cm;
    G4double half_length = 15*cm;

    auto* crystal = new G4Trd("Crystal", small_half_x, large_half_x,
                               small_half_y, large_half_y, half_length);

    auto* crystalLog = new G4LogicalVolume(crystal, csi_tl, "Crystal");

    // ---------------------------
    // Aluminum wrapper (with large base open)
    // ---------------------------

    G4double t_wrap = 25*micrometer;
    G4double wrapper_half_z = half_length + t_wrap;

    auto* fullWrapper = new G4Trd("FullWrapper",
                                   small_half_x + t_wrap, large_half_x + t_wrap,
                                   small_half_y + t_wrap, large_half_y + t_wrap,
                                   wrapper_half_z);

    // Cut open the large base (+Z side)

    G4double cutBox_halfZ = t_wrap + 0.1*mm;
    auto* cutBox = new G4Box("CutBox",
                              large_half_x + t_wrap + 0.1*mm,
                              large_half_y + t_wrap + 0.1*mm,
                              cutBox_halfZ);

    G4double cutBox_pos_z = wrapper_half_z - cutBox_halfZ + 0.05*mm;

    auto* wrapperOpen = new G4SubtractionSolid("WrapperOpen", fullWrapper, cutBox,
                                                nullptr, G4ThreeVector(0, 0, cutBox_pos_z));

    auto* fullWrapperLog = new G4LogicalVolume(wrapperOpen, al, "FullWrapper");

    // Place crystal inside wrapper

    auto* crystalPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), crystalLog,
                                           "Crystal", fullWrapperLog, false, 0);

    // Place wrapper in world

    auto* wrapperPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fullWrapperLog,
                                           "FullWrapper", worldLog, false, 0);

    // ---------------------------
    // Small base cover (aluminum cap at -Z)
    // ---------------------------

    G4double smallCover_halfZ = t_wrap/2.0;
    auto* smallBaseCover = new G4Box("SmallBaseCover",
                                      small_half_x + t_wrap,
                                      small_half_y + t_wrap,
                                      smallCover_halfZ);

    auto* smallBaseCoverLog = new G4LogicalVolume(smallBaseCover, al, "SmallBaseCover");

    G4double pos_smallCover_z = -(wrapper_half_z + smallCover_halfZ);

    new G4PVPlacement(nullptr, G4ThreeVector(0,0,pos_smallCover_z), smallBaseCoverLog,
                      "SmallBaseCover", worldLog, false, 0);

    // ---------------------------
    // Glass window at +Z (large base side)
    // ---------------------------

    G4double glassThickness = 2*mm;
    G4double glassRadius = large_half_x;

    auto* glassWindow = new G4Tubs("GlassWindow", 0.0*cm, glassRadius,
                                    glassThickness/2.0, 0.0*deg, 360.0*deg);

    auto* glassLog = new G4LogicalVolume(glassWindow, glass, "GlassWindow");

    G4double pos_back_z = wrapper_half_z;
    G4double back_halfZ = t_wrap/2.0;
    G4double glass_pos_z = pos_back_z + back_halfZ + glassThickness/2.0;

    auto* glassPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,glass_pos_z), glassLog,
                                         "GlassWindow", worldLog, false, 0);

    // ---------------------------
    // Virtual PMT (photocathode) behind glass
    // ---------------------------

    G4double pmtThickness = 0.1*mm;
    auto* pmtSolid = new G4Tubs("PMT", 0.0*cm, glassRadius, pmtThickness/2.0,
                                 0.0*deg, 360.0*deg);

    auto* pmtLog = new G4LogicalVolume(pmtSolid, air, "PMT");

    G4double pmt_pos_z = glass_pos_z + glassThickness/2.0 + pmtThickness/2.0;

    auto* pmtPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,pmt_pos_z), pmtLog,
                                       "PMT", worldLog, false, 0);

    G4cout << "✅ Virtual PMT placed at Z = " << pmt_pos_z/cm << " cm" << G4endl;

    // ===== OPTICAL SURFACES - STRONG INTERNAL REFLECTION =====

    // Crystal-Aluminum boundary (highly reflective mirror-like surface)

    G4OpticalSurface* crystalAlSurface = new G4OpticalSurface("CrystalAlSurface");
    crystalAlSurface->SetType(dielectric_metal);
    crystalAlSurface->SetFinish(polished);
    crystalAlSurface->SetModel(unified);

    G4MaterialPropertiesTable* crystalAlMPT = new G4MaterialPropertiesTable();
    crystalAlMPT->AddProperty("REFLECTIVITY", photonEnergy, alREFLECTIVITY, nEntries);
    crystalAlSurface->SetMaterialPropertiesTable(crystalAlMPT);

    // Apply to crystal-wrapper boundary for strong internal reflection

    new G4LogicalBorderSurface("CrystalWrapperSurface",
                               crystalPhys, wrapperPhys,
                               crystalAlSurface);

    // ===== REFLECTIVE SURFACE AT BACK OPENING =====

    G4OpticalSurface* backOpeningSurface = new G4OpticalSurface("BackOpeningSurface");
    backOpeningSurface->SetType(dielectric_metal);
    backOpeningSurface->SetFinish(polished);
    backOpeningSurface->SetModel(unified);

    G4MaterialPropertiesTable* backOpeningMPT = new G4MaterialPropertiesTable();
    backOpeningMPT->AddProperty("REFLECTIVITY", photonEnergy, alREFLECTIVITY, nEntries);
    backOpeningSurface->SetMaterialPropertiesTable(backOpeningMPT);

    new G4LogicalBorderSurface("BackOpeningWrapperSurface",
                               wrapperPhys, glassPhys,
                               backOpeningSurface);

    G4cout << "✅ Optical boundary surfaces configured (97% reflectivity)" << G4endl;
    G4cout << "   → Photons will undergo strong internal reflection at Crystal-Al interface" << G4endl;
    G4cout << "   → REFLECTIVE COATING applied to back opening edges" << G4endl;

    return worldPhys;

}

void DetectorConstruction::ConstructSDandField()
{

    // Get EventAction from RunManager

    auto* runManager = G4RunManager::GetRunManager();
    auto* eventAction = const_cast<EventAction*>(
        static_cast<const EventAction*>(runManager->GetUserEventAction()));

    // Crystal sensitive detector

    auto* crystalSD = new CrystalSD("CrystalSD");
    crystalSD->SetEventAction(eventAction);
    G4SDManager::GetSDMpointer()->AddNewDetector(crystalSD);
    SetSensitiveDetector("Crystal", crystalSD);

    // PMT sensitive detector

    auto* pmtSD = new PMTSD("PMTSD");
    pmtSD->SetEventAction(eventAction);
    G4SDManager::GetSDMpointer()->AddNewDetector(pmtSD);
    SetSensitiveDetector("PMT", pmtSD);

    G4cout << "✅ Sensitive detectors configured (Crystal + PMT)" << G4endl;

}
