# Copyright (C) 2016-2019  CEA/DEN, EDF R&D
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
# Author : Anthony Geay (EDF R&D)

PROJECT(MEDWriter)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
  #CMAKE_POLICY(SET CMP0022 OLD)
  #CMAKE_POLICY(SET CMP0023 OLD)
ENDIF(COMMAND cmake_policy)

SET(MED_WRITER_VERSION "0.0.0")

FIND_PACKAGE(ParaView REQUIRED)
IF(NOT ParaView_FOUND)
  MESSAGE(FATAL_ERROR "Please locate ParaView." )
ENDIF(NOT ParaView_FOUND)
INCLUDE(${PARAVIEW_USE_FILE})

OPTION(BUILD_SHARED_LIBS "Build with shared libraries." ${VTK_BUILD_SHARED_LIBS})

SET(VTK_INSTALL_RUNTIME_DIR lib/salome)
SET(VTK_INSTALL_LIBRARY_DIR lib/salome)
SET(VTK_INSTALL_ARCHIVE_DIR lib/salome)
SET(MEDWRITER_INSTALL_CMAKE_LOCAL adm_local/cmake_files CACHE PATH "Install path: local SALOME CMake files")
SET(MEDWRITER_INSTALL_HEADERS include/salome CACHE PATH "Install path: local include files")


SET(MEDCOUPLING_ROOT_DIR $ENV{MEDCOUPLING_ROOT_DIR} CACHE PATH "MEDCOUPLING_ROOT_DIR")
LIST(APPEND CMAKE_MODULE_PATH "${MEDCOUPLING_ROOT_DIR}/cmake_files")
INCLUDE(SalomeMacros)
FIND_PACKAGE(SalomeHDF5 REQUIRED)
FIND_PACKAGE(SalomeMEDCoupling REQUIRED)
FIND_PACKAGE(SalomeMEDFile REQUIRED)

PV_PROCESS_MODULES()

INCLUDE_DIRECTORIES(
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS})

ADD_SUBDIRECTORY(IO)
ADD_SUBDIRECTORY(ParaViewPlugin)

INCLUDE(CMakePackageConfigHelpers)

SALOME_CONFIGURE_PREPARE(MEDCOUPLING MEDFILE HDF5)
CONFIGURE_PACKAGE_CONFIG_FILE(${PROJECT_NAME}Config.cmake.in 
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION "${MEDWRITER_INSTALL_CMAKE_LOCAL}"
  PATH_VARS CMAKE_INSTALL_PREFIX MEDWRITER_INSTALL_HEADERS)

INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" DESTINATION "${MEDWRITER_INSTALL_CMAKE_LOCAL}")
INSTALL(EXPORT ${PROJECT_NAME}TargetGroup DESTINATION "${MEDWRITER_INSTALL_CMAKE_LOCAL}" FILE ${PROJECT_NAME}Targets.cmake)
