# Common CMake macros
# ===================
set(TMP_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
unset(CMAKE_MODULE_PATH)
set(CONFIGURATION_ROOT_DIR $ENV{CONFIGURATION_ROOT_DIR} CACHE PATH "Path to the Salome CMake configuration files")
if(EXISTS ${CONFIGURATION_ROOT_DIR})
  list(APPEND CMAKE_MODULE_PATH "${CONFIGURATION_ROOT_DIR}/cmake")
  include(SalomeMacros)
else()
  message(FATAL_ERROR "We absolutely need the Salome CMake configuration files, please define CONFIGURATION_ROOT_DIR !")
endif()

set(MEDCOUPLING_ROOT_DIR $ENV{MEDCOUPLING_ROOT_DIR} CACHE PATH "Path to the MEDCoupling tool")
if(EXISTS ${MEDCOUPLING_ROOT_DIR})
  list(APPEND CMAKE_MODULE_PATH "${MEDCOUPLING_ROOT_DIR}/cmake_files")
endif()
list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
list(APPEND CMAKE_MODULE_PATH ${TMP_CMAKE_MODULE_PATH})

include(SalomeSetupPlatform)
set(BUILD_SHARED_LIBS TRUE)

find_package(SalomePythonInterp REQUIRED)
find_package(SalomePythonLibs REQUIRED)
find_package(SalomeHDF5 REQUIRED)
find_package(SalomeMEDCoupling REQUIRED)
find_package(SalomeMEDFile REQUIRED)

if(MEDCOUPLING_USE_64BIT_IDS)
  add_definitions("-DMEDCOUPLING_USE_64BIT_IDS")
endif(MEDCOUPLING_USE_64BIT_IDS)

SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_BINS}
                                                 ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON})
SALOME_ACCUMULATE_ENVIRONMENT(LD_LIBRARY_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_LIBS})
SALOME_ACCUMULATE_ENVIRONMENT(PV_PLUGIN_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/lib/paraview)

add_subdirectory(MEDLoaderForPV)
add_subdirectory(ParaViewPlugin)

option(BUILD_TESTING "Build Plugin Testing" OFF)
if (BUILD_TESTING)
  add_subdirectory(Test)
endif()
