# Copyright (C) 2010-2020  CEA/DEN, EDF R&D
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

find_package(SalomeHDF5 REQUIRED)
find_package(SalomeMEDCoupling REQUIRED)

if(MEDCOUPLING_USE_64BIT_IDS)
  add_definitions("-DMEDCOUPLING_USE_64BIT_IDS")
endif(MEDCOUPLING_USE_64BIT_IDS)

SALOME_ACCUMULATE_ENVIRONMENT(PYTHONPATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_BINS}
                                                 ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_PYTHON})
SALOME_ACCUMULATE_ENVIRONMENT(LD_LIBRARY_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/${SALOME_INSTALL_LIBS})
SALOME_ACCUMULATE_ENVIRONMENT(PV_PLUGIN_PATH NOCHECK ${CMAKE_INSTALL_PREFIX}/lib/paraview)

paraview_add_plugin(VoroGaussPlugin
  VERSION "1.0"
  MODULES VoroGaussModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/VoroGaussModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS VoroGaussPlugin
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
