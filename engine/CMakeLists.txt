cmake_minimum_required(VERSION 3.16)
project(web_geant4 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Geant4: static, no vis/UI. GEANT4_DIR must point to the (native or emscripten) install.
find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

# nlohmann/json single header, downloaded by scripts into third_party/
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

set(ENGINE_SOURCES
    src/DetectorConstruction.cc
    src/PrimaryGenerator.cc
    src/Actions.cc
    src/SimEngine.cc)

if(EMSCRIPTEN)
  add_executable(geant4app ${ENGINE_SOURCES} src/wasm_bindings.cpp)
  target_link_libraries(geant4app PRIVATE ${Geant4_LIBRARIES} embind)
  # Datasets are fetched at runtime into MEMFS by the JS side; G4*DATA env vars
  # are injected via --pre-js (scripts/setup_env.js).
  set_target_properties(geant4app PROPERTIES
    OUTPUT_NAME "geant4"
    SUFFIX ".mjs"
    LINK_FLAGS "-sMODULARIZE=1 -sEXPORT_ES6=1 -sEXPORT_NAME=createGeant4Module \
-sENVIRONMENT=worker -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=256MB \
-sSTACK_SIZE=5MB -sEXPORTED_RUNTIME_METHODS=FS,ENV -fexceptions \
--pre-js ${CMAKE_CURRENT_SOURCE_DIR}/setup_env.js")
  # Slimmed Geant4 datasets packed into a .data file loaded at module init.
  if(DEFINED G4DATA_DIR)
    get_target_property(LF geant4app LINK_FLAGS)
    set_target_properties(geant4app PROPERTIES
      LINK_FLAGS "${LF} --preload-file ${G4DATA_DIR}@/data")
  endif()
  target_compile_options(geant4app PRIVATE -fexceptions -O3)
else()
  add_executable(web-geant4 ${ENGINE_SOURCES} src/native_main.cpp)
  target_link_libraries(web-geant4 PRIVATE ${Geant4_LIBRARIES})
endif()
