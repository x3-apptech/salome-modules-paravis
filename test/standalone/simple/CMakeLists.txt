# Copyright (C) 2010-2015  CEA/DEN, EDF R&D
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
# Author: Adrien Bruneton (CEA)

SET(pl_SOURCES
    simple_gil.cxx
    #PyInterp_Interp.cxx
    #SUITApp_init_python.cxx
    Container_init_python.cxx
    )
    
INCLUDE_DIRECTORIES(
    ${PARAVIEW_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    ${QT_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(paraCmdLine ${pl_SOURCES})
TARGET_LINK_LIBRARIES(paraCmdLine ${PYTHON_LIBRARIES} ${QT_LIBRARIES} pqApplicationComponents vtkRenderingFreeType)

