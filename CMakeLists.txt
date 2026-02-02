cmake_minimum_required(VERSION 3.16)
project(CsI_Simulation)

# Χρησιμοποιούμε Geant4 (analysis module περιλαμβάνεται αυτόματα)
find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

# Βρίσκουμε ROOT (για ROOT-based output)
find_package(ROOT REQUIRED COMPONENTS RIO Tree Hist)
include(${ROOT_USE_FILE})

# Ρυθμίσεις C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Φάκελοι με include αρχεία
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${Geant4_INCLUDE_DIRS}
  ${ROOT_INCLUDE_DIRS}
)

# Source αρχεία του project
set(SOURCES
  src/main.cc
  src/detector_construction.cc
  src/primary_generator.cc
  src/sensitive_detector.cc
  src/action_initialization.cc
  src/event_action.cc
  src/run_action.cc
  src/PhysicsList.cc
  src/stepping_action.cc
  src/tracking_action.cc
  src/pmt_sd.cc
)

# Δημιουργία εκτελέσιμου
add_executable(csi_sim ${SOURCES})

# Σύνδεση με Geant4 και ROOT
target_link_libraries(csi_sim
  ${Geant4_LIBRARIES}
  ${ROOT_LIBRARIES}
)

# Ορισμός φακέλου για το εκτελέσιμο
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


