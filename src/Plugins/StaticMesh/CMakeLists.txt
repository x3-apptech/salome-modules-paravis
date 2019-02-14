# Copyright (C) 2018-2019  EDF R&D
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

if (NOT ParaView_BINARY_DIR)
  cmake_minimum_required(VERSION 3.3)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif()

include_directories(
  ${VTK_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(Module_SRCS
  vtkStaticDataSetSurfaceFilter.cxx
  vtkStaticEnSight6BinaryReader.cxx
  vtkStaticEnSight6Reader.cxx
  vtkStaticEnSightGoldBinaryReader.cxx
  vtkStaticEnSightGoldReader.cxx
  vtkStaticMeshObjectFactory.cxx
  vtkStaticPlaneCutter.cxx
)
if (PARAVIEW_USE_MPI)
  list(APPEND Module_SRCS vtkStaticPUnstructuredGridGhostCellsGenerator.cxx)
endif()

add_paraview_plugin(
  StaticMesh "0.1"
  SERVER_MANAGER_XML
    StaticMeshSM.xml
  SERVER_MANAGER_SOURCES
    vtkTemporalUGWavelet.cxx
  SOURCES
    ${Module_SRCS}
  )

if (BUILD_TESTING)
  add_subdirectory(Testing)
endif()
INSTALL(TARGETS StaticMesh RUNTIME DESTINATION lib/paraview LIBRARY DESTINATION lib/paraview ARCHIVE DESTINATION lib/paraview)
