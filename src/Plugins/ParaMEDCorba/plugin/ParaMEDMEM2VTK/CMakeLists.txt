# Copyright (C) 2010-2021  CEA/DEN, EDF R&D
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

include_directories(
  ${OMNIORB_INCLUDE_DIR}
  ${FIELDS_ROOT_DIR}/include/salome
  ${KERNEL_ROOT_DIR}/include/salome
  ${CMAKE_CURRENT_SOURCE_DIR}/../ParaMEDCorbaModule
  )

add_definitions(${OMNIORB_DEFINITIONS})

add_library(ParaMEDMEM2VTK SHARED VTKMEDCouplingMeshClient.cxx VTKMEDCouplingUMeshClient.cxx VTKMEDCouplingCMeshClient.cxx VTKMEDCouplingFieldClient.cxx VTKParaMEDFieldClient.cxx VTKMEDCouplingMultiFieldsClient.cxx VTKMEDCouplingCurveLinearMeshClient.cxx)

target_link_libraries(ParaMEDMEM2VTK
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::CommonExecutionModel
  VTK::FiltersGeneral
  ParaView::VTKExtensionsMisc
  ${OMNIORB_LIBRARIES}
  ${FIELDS_SalomeIDLMED}
  ${KERNEL_SalomeIDLKernel}
  )

#Build library with a static runtime and ignore MSVCRT.lib on WINDOWS
if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set_target_properties(ParaMEDMEM2VTK PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRTD")
    set_target_properties(ParaMEDMEM2VTK PROPERTIES COMPILE_FLAGS "/MTd")
  else()
    set_target_properties(ParaMEDMEM2VTK PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
    set_target_properties(ParaMEDMEM2VTK PROPERTIES COMPILE_FLAGS "/MT")
  endif()
endif(WIN32)

install(TARGETS ParaMEDMEM2VTK EXPORT DESTINATION ${SALOME_INSTALL_LIBS})
