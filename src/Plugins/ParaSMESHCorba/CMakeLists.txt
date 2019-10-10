# Copyright (C) 2010-2019  CEA/DEN, EDF R&D
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
cmake_minimum_required(VERSION 3.8)
project(ParaSMESHCorba)

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

find_package(SalomeSMESH)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/plugin/ParaSMESHCorbaModule )

set("_paraview_plugin_default_${CMAKE_PROJECT_NAME}" ON)
paraview_plugin_scan(
  ENABLE_BY_DEFAULT YES
  PLUGIN_FILES      "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin"
  PROVIDES_PLUGINS  plugins
  REQUIRES_MODULES  required_modules)

foreach(module IN LISTS required_modules)
  if(NOT TARGET "${module}")
    message("Missing required module: ${module}")
    return()
  endif()
endforeach()

set(BUILD_SHARED_LIBS ON)
paraview_plugin_build(
  RUNTIME_DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY_DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY_SUBDIRECTORY "${PARAVIEW_PLUGIN_SUBDIR}"
  PLUGINS ${plugins}
  AUTOLOAD ${plugins})

# PROJECT(ParaSMESHCorba)
# CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
#
# FIND_PACKAGE(ParaView REQUIRED)
# INCLUDE(${PARAVIEW_USE_FILE})
#
# SET(PARA_SMESH_CORBA_SRC
#    ${ParaSMESHCorba_SOURCE_DIR}/vtkParaSMESHCorbaSource.cxx 
#    )

# ADD_PARAVIEW_PLUGIN( ParaSMESHCorbaPlugin "1.0"
#      SERVER_MANAGER_XML ParaSMESHCorbaServerManager.xml
#      SERVER_MANAGER_SOURCES ${PARA_SMESH_CORBA_SRC} )

# INCLUDE_DIRECTORIES(
#     ${OMNIORB_INCLUDE_DIR}
#     ${SMESH_ROOT_DIR}/include/salome 
#     ${GEOM_ROOT_DIR}/include/salome
#     ${KERNEL_ROOT_DIR}/include/salome )
     
# ADD_DEFINITIONS(${OMNIORB_DEFINITIONS})
# TARGET_LINK_LIBRARIES(ParaSMESHCorbaPlugin
#      ${OMNIORB_LIBRARIES}
#      ${KERNEL_SalomeIDLKernel}
#      ${SMESH_SalomeIDLSMESH}
#      ${SMESH_SMESHClient}
#      ${GEOM_SalomeIDLGEOM}
#  )

# INSTALL(TARGETS ParaSMESHCorbaPlugin DESTINATION lib/paraview)
# INSTALL(FILES ParaSMESHCorbaServerManager.xml DESTINATION share)
