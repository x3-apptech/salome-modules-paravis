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

add_library(MEDLoaderForPV SHARED MEDFileFieldRepresentationTree.cxx  MEDTimeReq.cxx  MEDUtilities.cxx  vtkGenerateVectors.cxx)
target_include_directories(MEDLoaderForPV PRIVATE . ${MEDCOUPLING_INCLUDE_DIRS})

target_link_libraries(MEDLoaderForPV VTK::CommonCore VTK::CommonDataModel VTK::IOXML ${MEDFILE_C_LIBRARIES})

IF(HDF5_IS_PARALLEL)
  target_link_libraries(MEDLoaderForPV ${MEDCoupling_paramedloader})
ELSE(HDF5_IS_PARALLEL)
  TARGET_LINK_LIBRARIES(MEDLoaderForPV ${MEDCoupling_medloader})
ENDIF(HDF5_IS_PARALLEL)

install(TARGETS MEDLoaderForPV 
        DESTINATION lib/salome)#${CMAKE_INSTALL_LIBDIR}
