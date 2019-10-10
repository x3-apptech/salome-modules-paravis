# Copyright (C) 2010-2019  CEA/DEN, EDF R&D
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

INCLUDE(UseQtExt)

# --- options ---

INCLUDE_DIRECTORIES(
    ${KERNEL_INCLUDE_DIRS}
    ${GUI_INCLUDE_DIRS}  
    ${PROJECT_BINARY_DIR} 
    ${Boost_INCLUDE_DIRS}
    ${QT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

# additional preprocessor / compiler flags
ADD_DEFINITIONS(${KERNEL_DEFINITIONS})

# libraries to link to

SET(_link_LIBRARIES 
  ${QT_LIBRARIES}
  ${PYTHON_LIBS}
  ${KERNEL_LDFLAGS}
  ${GUI_LDFLAGS} ${GUI_LightApp} ${GUI_CAM} ${GUI_qtx} ${GUI_suit}
                 ${GUI_SalomeObject}
  ParaView::pqApplicationComponents		 
  ${GUI_PVViewer}
)

# --- headers ---

SET(_ui_files
    resources/pqCustomSettingsWidget.ui
    )

QT_WRAP_UIC(_ui_FORMS_HEADERS ${_ui_files})

# header files / to be processed by moc
SET(_moc_HEADERS
  PVGUI_Module.h
  PVGUI_DataModel.h
  PVGUI_ParaViewSettingsPane.h
)

# header files / to install
SET(PVGUI_HEADERS ${_moc_HEADERS} ${_ui_FORMS_HEADERS})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  resources/PARAVIS_images.ts
  resources/PARAVIS_msg_en.ts
  resources/PARAVIS_msg_fr.ts
  resources/PARAVIS_msg_ja.ts
)

# resource files / static
SET(_other_RESOURCES
  # sampleservers.pvsc ???
)

IF(SALOME_BUILD_DOC)
  # --- ParaView documentation is built as a resource ---
  # Check ParaView documentation file existence
  # if it exists add it to the resources (compile into binary form)
  # otherwise print warning message
  SET(PV_DOC_FILE ${ParaView_PREFIX_PATH}/share/doc/ParaView/paraview.qch)
  IF (EXISTS ${PV_DOC_FILE})
    SET(RC_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/PVGUI.qrc.in)
    SET(RC_GENERATED ${CMAKE_CURRENT_BINARY_DIR}/PVGUI.qrc)
    MESSAGE(STATUS "Creation of ${RC_GENERATED}")
    CONFIGURE_FILE(${RC_SOURCE} ${RC_GENERATED})
    SET(_rcc_RESOURCES ${RC_GENERATED})
    ADD_DEFINITIONS(-DHAS_PV_DOC)
  ELSE ()
    MESSAGE(WARNING "Help file for ParaView has not been found! ParaView documentation will not be available.")
  ENDIF ()
ENDIF()

# --- sources ---

# sources / rcc wrappings
QT_ADD_RESOURCES(_rcc_SOURCES ${_rcc_RESOURCES})

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  PVGUI_Module.cxx
  PVGUI_Module_actions.cxx
  PVGUI_Module_widgets.cxx
  PVGUI_ParaViewSettingsPane.cxx
  PVGUI_DataModel.cxx
  )

# sources / to compile
SET(PVGUI_SOURCES 
  ${_other_SOURCES} 
  ${_moc_SOURCES} 
  ${_rcc_SOURCES} 
  ${_VTK_wrap_SOURCES}
  ${_ui_FORMS_HEADERS}
  )

# --- rules ---
ADD_LIBRARY(PARAVIS ${PVGUI_SOURCES})
TARGET_LINK_LIBRARIES(PARAVIS ${_link_LIBRARIES})
INSTALL(TARGETS PARAVIS EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

# --- header and resources installation ---
INSTALL(FILES ${PVGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_PARAVIS_INSTALL_RES_DATA}")
INSTALL(FILES ${_other_RESOURCES} DESTINATION ${SALOME_PARAVIS_INSTALL_RES_DATA})
