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

SET(pl_HEADERS
    PLMainWindow.hxx
    PLViewTab.hxx
    PVViewer_Behaviors.h
    PVViewer_GUIElements.h
    )
    
SET(pl_OTHER_HEADERS
    PVViewer_Core.h
    )

SET(pl_SOURCES
    main.cpp
    PLMainWindow.cxx
    PLViewTab.cxx
    PVViewer_Behaviors.cxx
    PVViewer_GUIElements.cxx
    PVViewer_Core.cxx
    )

SET(pl_FORMS
    ui/light_para.ui
    ui/view_tab.ui
    )
    
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PARAVIEW_INCLUDE_DIRS})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

# Generate resources that will embedded 
SET(ui_resources
  "${CMAKE_CURRENT_BINARY_DIR}/LightPara_configuration.qrc")
  
GENERATE_QT_RESOURCE_FROM_FILES(
    "${ui_resources}"
    "/LightPara/Configuration"
    "${PROJECT_SOURCE_DIR}/src/xml/ParaViewReaders.xml")

QT4_ADD_RESOURCES(rcs_sources
    ${ui_resources}
    )
QT4_WRAP_UI(pl_FORMS_HEADERS ${pl_FORMS})
QT4_WRAP_CPP(pl_HEADERS_MOC  ${pl_HEADERS})

ADD_EXECUTABLE(paraLight
    ${pl_SOURCES} 
    ${pl_HEADERS_MOC} 
    ${pl_FORMS_HEADERS}
    ${rcs_sources})
    
TARGET_LINK_LIBRARIES(paraLight ${QT_LIBRARIES} pqApplicationComponents vtkRenderingFreeTypeOpenGL)
#INSTALL(TARGET paraLight bin)

