#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.9 FATAL_ERROR)
project(eicdirc)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT REQUIRED)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
include_directories(SYSTEM ${Geant4_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

# add tensorflow, cppflow 
include_directories($ENV{HOME}/usr/include)
include_directories(cppflow/include)
link_directories($ENV{HOME}/usr/lib)

include(${ROOT_USE_FILE})
include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/src/*.h)

MESSAGE(STATUS ${PROJECT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 17)
#----------------------------------------------------------------------------
# Generate dictionary for root
ROOT_GENERATE_DICTIONARY(G__Prt PrtHit.h PrtEvent.h PrtRun.h PrtLutNode.h PrtTools.h LINKDEF src/PrtLinkDef.h MODULE Prt)
add_library(Prt SHARED src/PrtHit.cxx src/PrtEvent.cxx src/PrtRun.cxx src/PrtLutNode.cxx src/PrtTools.cxx G__Prt.cxx)

target_link_libraries(Prt ${ROOT_LIBRARIES} -lSpectrum -lHistPainter)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries

if(AI)
  add_definitions( -DAI )
  set(AI_LIB "-ltensorflow")
else()
  set(AI_LIB "")
endif()

add_executable(eicdirc ${sources})
target_link_libraries(eicdirc -lGLU ${AI_LIB} ${Geant4_LIBRARIES} Prt)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build Prt. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(Prt_SCRIPTS
    PrtDirc.out
    PrtDirc.mac
    gui.mac
    icons.mac
    run.png
    vis.mac
    misalign.sh
  )

foreach(_script ${Prt_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS eicdirc DESTINATION bin)

