# Copyright (C) 2010-2017  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

# (cd /home/H87074/salome/DEV/modules/build/MEDReader ; export CURRENT_SOFTWARE_SRC_DIR=/home/H87074/salome/DEV/modules/src/PARAVIS ; export CURRENT_SOFTWARE_BUILD_DIR=/home/H87074/salome/DEV/modules/build/PARAVIS_master ; export CURRENT_SOFTWARE_INSTALL_DIR=/home/H87074/salome/DEV/modules/install/PARAVIS_master ; export PYTHON_VERSION="2.7" ; . /home/H87074/salome/DEV/modules/build/PARAVIS_master/.yamm/env_build.sh >/dev/null 2>&1 ; . /home/H87074/salome/DEV/salome_modules.sh >/dev/null 2>&1 ; . /home/H87074/salome/DEV/salome_prerequisites.sh >/dev/null 2>&1 ; export PARAVIEW_INSTALL_DIR="/home/H87074/salome/prerequisites/install/Paraview-v512_EDF-cm362-dox1831-gl2ps139-gr2380-hd1814-xml2290-py2710-qt561" ;  cmake -DCMAKE_INSTALL_PREFIX=/home/H87074/salome/DEV/modules/install/PARAVIS_master  -DPYTHON_ROOT_DIR=${PYTHONHOME}  -DSALOME_PARAVIS_MINIMAL_CORBA=ON   -DCMAKE_BUILD_TYPE=Debug -DSALOME_PARAVIS_MINIMAL_CORBA=ON -DMEDCOUPLING_ROOT_DIR=/home/H87074/salome/DEV/tools/install/Medcoupling-master /home/H87074/salome/DEV/modules/src/PARAVIS/src/Plugins/MEDReader)

IF(NOT KW_TESTING_PLATFORM)
PROJECT(MEDReader C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
INCLUDE(CMakeDependentOption)
ENDIF(NOT KW_TESTING_PLATFORM)

IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(MED_READER_MAJOR_VERSION 0)
SET(MED_READER_MINOR_VERSION 0)
SET(MED_READER_BUILD_VERSION 0)
SET(MED_READER_VERSION "${MED_READER_MAJOR_VERSION}.${MED_READER_MINOR_VERSION}.${MED_READER_BUILD_VERSION}")

#
# Load the ParaView/VTK packages
#
SET(MEDReader_BUILD_ParaView_Plugin ON CACHE BOOL "Build the ParaView plugin")

  
# Ensure a proper linker behavior:
CMAKE_POLICY(SET CMP0003 NEW)
CMAKE_POLICY(SET CMP0045 OLD)
CMAKE_POLICY(SET CMP0023 OLD)
CMAKE_POLICY(SET CMP0022 OLD)
IF(WIN32)
  CMAKE_POLICY(SET CMP0020 OLD) # disable automatic linking to qtmain.lib 
ENDIF(WIN32)

# Common CMake macros
# ===================
SET(TMP_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
unset(CMAKE_MODULE_PATH)
SET(CONFIGURATION_ROOT_DIR $ENV{CONFIGURATION_ROOT_DIR} CACHE PATH "Path to the Salome CMake configuration files")
IF(EXISTS ${CONFIGURATION_ROOT_DIR})
  LIST(APPEND CMAKE_MODULE_PATH "${CONFIGURATION_ROOT_DIR}/cmake")
  INCLUDE(SalomeMacros)
ELSE()
  MESSAGE(FATAL_ERROR "We absolutely need the Salome CMake configuration files, please define CONFIGURATION_ROOT_DIR !")
ENDIF()

SET(MEDCOUPLING_ROOT_DIR $ENV{MEDCOUPLING_ROOT_DIR} CACHE PATH "Path to the MEDCoupling tool")
IF(EXISTS ${MEDCOUPLING_ROOT_DIR})
  LIST(APPEND CMAKE_MODULE_PATH "${MEDCOUPLING_ROOT_DIR}/cmake_files")   
ENDIF()
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
LIST(APPEND CMAKE_MODULE_PATH ${TMP_CMAKE_MODULE_PATH})



# Platform setup
# ==============
INCLUDE(SalomeSetupPlatform)
SET(BUILD_SHARED_LIBS TRUE)

FIND_PACKAGE(SalomePythonInterp REQUIRED)
FIND_PACKAGE(SalomePythonLibs REQUIRED)
FIND_PACKAGE(SalomeHDF5 REQUIRED)
FIND_PACKAGE(SalomeMEDCoupling REQUIRED)
FIND_PACKAGE(SalomeMEDFile REQUIRED)

SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_BINS}
                                                 ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON})
SALOME_ACCUMULATE_ENVIRONMENT(LD_LIBRARY_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_LIBS})
SALOME_ACCUMULATE_ENVIRONMENT(PV_PLUGIN_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/lib/paraview)

IF(MEDReader_BUILD_ParaView_Plugin)
    FIND_PACKAGE(ParaView REQUIRED)
    IF(NOT ParaView_FOUND)
        MESSAGE(FATAL_ERROR "Please locate ParaView." )
    ENDIF(NOT ParaView_FOUND)
    INCLUDE(${PARAVIEW_USE_FILE})
    # Load information about VTK/ParaView modules loaded/available.
    PV_SETUP_MODULE_ENVIRONMENT("vtkMEDReader")
ELSE(MEDReader_BUILD_ParaView_Plugin)
    FIND_PACKAGE(VTK REQUIRED)
    if(NOT VTK_FOUND)
        MESSAGE(FATAL_ERROR "Please locate VTK." )
    ENDIF(NOT VTK_FOUND)
    INCLUDE(${VTK_USE_FILE})
ENDIF(MEDReader_BUILD_ParaView_Plugin)
 
FIND_PACKAGE(SalomeQt5 REQUIRED)
INCLUDE(UseQtExt)

# Accumulate environment variables for PARAVIS module
SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_BINS}
                                                 ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON})
SALOME_ACCUMULATE_ENVIRONMENT(LD_LIBRARY_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_LIBS})
SALOME_ACCUMULATE_ENVIRONMENT(PV_PLUGIN_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/lib/paraview)

#
# process all vtk-modules defined in this plugin. This generates the necessary
# wrappings for all these modules as well.
#
SET(VTK_INSTALL_RUNTIME_DIR lib/salome)
SET(VTK_INSTALL_LIBRARY_DIR lib/salome)
SET(VTK_INSTALL_ARCHIVE_DIR lib/salome)
INCLUDE_DIRECTORIES(${MEDFILE_INCLUDE_DIRS} ${MEDCOUPLING_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS})
PV_PROCESS_MODULES()

#
# Build the parallel vtkPMEDReader
#
SET(MEDReader_BUILD_PARALLEL ${VTK_USE_PARALLEL} CACHE BOOL "Build the Med Reader with parallel support.")

#
# Build the ParaView Plugin
#
IF(MEDReader_BUILD_ParaView_Plugin)
  ADD_SUBDIRECTORY(ParaViewPlugin)
ENDIF(MEDReader_BUILD_ParaView_Plugin)

SET(SALOME_BUILD_TESTS ON CACHE BOOL "Build Salome Test")
IF(SALOME_BUILD_TESTS)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(Test)
ENDIF()
