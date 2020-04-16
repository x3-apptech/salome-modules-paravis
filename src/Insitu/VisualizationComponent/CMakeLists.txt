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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${FIELDS_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../VisualizationLibrary
)

# --- definitions ---
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${FIELDS_DEFINITIONS}
  ${MEDCOUPLING_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${OMNIORB_LIBRARIES}
  ${KERNEL_SalomeIDLKernel}
  ${KERNEL_OpUtil}
  ${KERNEL_SalomeContainer}
  SalomeIDLPARAVIS
  ${KERNEL_SalomeMPIContainer}
  MEDLoaderForPV
  visulib
  ${FIELDS_SalomeIDLMED}
  ${FIELDS_medcouplingcorba}
  ${FIELDS_medcouplingclient}
  ${FIELDS_paramedmemcompo}
  ${FIELDS_paramedcouplingcorba}  
)

# --- headers ---

# header files / no moc processing

SET(PARAVIS_HEADERS
  VisualizationComponent.hxx
)

# --- sources ---

# sources / static
SET(PARAVIS_SOURCES
  VisualizationComponent.cxx
  
)

# --- rules ---

ADD_LIBRARY(VisualizationComponentEngine ${PARAVIS_SOURCES})
TARGET_LINK_LIBRARIES(VisualizationComponentEngine ${_link_LIBRARIES} )

INSTALL(TARGETS VisualizationComponentEngine
        DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${PARAVIS_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
