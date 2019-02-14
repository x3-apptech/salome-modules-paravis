# Copyright (C) 2016-2019  EDF R&D
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

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../IO )
ADD_PARAVIEW_PLUGIN(VoroGaussPlugin "1.0"
  SERVER_MANAGER_SOURCES ${SM_SRCS} ${PROJECT_SOURCE_DIR}/IO/vtkVoroGauss.cxx
  SERVER_MANAGER_XML Resources/VoroGaussServer.xml)
TARGET_LINK_LIBRARIES(VoroGaussPlugin ${MEDCoupling_medcoupling})
INSTALL(TARGETS VoroGaussPlugin RUNTIME DESTINATION lib/paraview LIBRARY DESTINATION lib/paraview ARCHIVE DESTINATION lib/paraview)
