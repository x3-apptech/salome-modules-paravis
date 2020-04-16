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

set(classes
  vtkDevelopedSurface
  VTKToMEDMem
)

vtk_module_add_module(DevelopedSurfaceModule
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(DevelopedSurfaceModule PRIVATE ${MEDCOUPLING_INCLUDE_DIRS})

if(HDF5_IS_PARALLEL)
  target_link_libraries(DevelopedSurfaceModule PRIVATE ${MEDCoupling_paramedloader})
else()
  target_link_libraries(DevelopedSurfaceModule PRIVATE ${MEDCoupling_medloader})
endif()
