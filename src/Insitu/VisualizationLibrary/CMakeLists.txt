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

SET(BUILD_SHARED_LIBS TRUE)

ADD_DEFINITIONS(${MEDCOUPLING_DEFINITIONS})

INCLUDE_DIRECTORIES(
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${MPI_INCLUDE_PATH}
  ${PARAVIS_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/MEDReader/plugin/MEDLoaderForPV
)

SET(_link_LIBRARIES
  ${MEDCoupling_medcoupling}
  ${MEDCoupling_interpkernel}
  ${MPI_LIBRARIES}
  ${PARAVIS_MEDLoaderForPV}
  ${MEDCoupling_medloader}
  ${MEDFILE_C_LIBRARIES}
  ParaView::PythonCatalyst
)

SET(_lib_HEADERS
  visu.hxx
)

SET(_lib_SOURCES
  visu.cxx
)

ADD_LIBRARY(visulib ${_lib_SOURCES})
TARGET_LINK_LIBRARIES(visulib ${_link_LIBRARIES} )

INSTALL(TARGETS visulib
        EXPORT ${PROJECT_NAME}TargetGroup
        DESTINATION ${SALOME_INSTALL_LIBS})
INSTALL(FILES ${_lib_HEADERS} DESTINATION include/salome )


