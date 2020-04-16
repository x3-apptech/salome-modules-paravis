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

set(qrc_files)
if(PARAVIEW_USE_QT)
  list(APPEND qrc_files pqEllipseBuilder.qrc)
endif()

paraview_add_plugin(EllipseBuilderFilter
  VERSION "1.0"
  UI_RESOURCES ${qrc_files}
  MODULES EllipseBuilderModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/EllipseBuilderModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS EllipseBuilderFilter
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
