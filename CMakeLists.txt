cmake_minimum_required (VERSION 3.13)

project(SDMMCAnalyzer)

add_definitions( -DLOGIC2 )

# enable generation of compile_commands.json, helpful for IDEs to locate include files.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# custom CMake Modules are located in the cmake directory.
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(ExternalAnalyzerSDK)

set(SOURCES 
src/SDMMCAnalyzer.cpp 
src/SDMMCAnalyzerResults.cpp 
src/SDMMCAnalyzerSettings.cpp 
src/SDMMCHelpers.cpp 
src/SDMMCSimulationDataGenerator.cpp
src/SDMMCAnalyzer.h
src/SDMMCAnalyzerResults.h
src/SDMMCAnalyzerSettings.h 
src/SDMMCHelpers.h 
src/SDMMCSimulationDataGenerator.h
)

add_analyzer_plugin(${PROJECT_NAME} SOURCES ${SOURCES})
