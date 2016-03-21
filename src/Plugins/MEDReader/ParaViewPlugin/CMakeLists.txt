# Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../IO )

IF(PARAVIEW_BUILD_QT_GUI)

  QT4_WRAP_UI(CLIENT_UI_SRCS
    Resources/UI/MEDReaderVectorsWidget.ui
    Resources/UI/MEDReaderTimeModeWidget.ui)

  # create a plugin that implements an object panel for the reader
  SET(SM_SRCS
    vtkPVMetaDataInformation.cxx)

  SET(CLIENT_SRCS
    pqExtractGroupFieldsWidget.cxx
    pqAbstractFieldsWidget.cxx
    VectBoolSpreadSheet.cxx
    VectBoolWidget.cxx
    pqMEDReaderGraphUtils.cxx
    pqMEDReaderVectorsWidget.cxx
    pqMEDReaderTimeModeWidget.cxx
    pqMEDReaderReloadWidget.cxx
    pqMEDReaderTimesFlagsWidget.cxx
    pqMEDReaderFieldsWidget.cxx)

  QT4_WRAP_CPP(MOC_SRCS
    pqExtractGroupFieldsWidget.h
    pqAbstractFieldsWidget.h
    VectBoolSpreadSheet.h
    VectBoolWidget.h
    pqMEDReaderVectorsWidget.h
    pqMEDReaderTimeModeWidget.h
    pqMEDReaderReloadWidget.h
    pqMEDReaderTimesFlagsWidget.h
    pqMEDReaderFieldsWidget.h)

  ADD_PARAVIEW_PROPERTY_WIDGET(MEDREADER_TIMEMODE_IFACES MEDREADER_TIMEMODE_IFACE_SRCS
    TYPE "MEDReaderTimeModeWidgetType"
    CLASS_NAME pqMEDReaderTimeModeWidget)

  ADD_PARAVIEW_PROPERTY_WIDGET(MEDREADER_RELOAD_IFACES MEDREADER_RELOAD_IFACE_SRCS
    TYPE "MEDReaderReloadWidgetType"
    CLASS_NAME pqMEDReaderReloadWidget)

  ADD_PARAVIEW_PROPERTY_WIDGET(MEDREADER_VECTORS_IFACES MEDREADER_VECTORS_IFACE_SRCS
    TYPE "MEDReaderVectorsWidgetType"
    CLASS_NAME pqMEDReaderVectorsWidget)

  ADD_PARAVIEW_PROPERTY_WIDGET(MEDREADER_TIMESFLAGS_IFACES MEDREADER_TIMESFLAGS_IFACE_SRCS
    TYPE "MEDReaderTimesFlagsWidgetType"
    CLASS_NAME pqMEDReaderTimesFlagsWidget)

  ADD_PARAVIEW_PROPERTY_WIDGET(MEDREADER_FIELDS_IFACES MEDREADER_FIELDS_IFACE_SRCS
    TYPE "MEDReaderFieldsWidgetType"
    CLASS_NAME pqMEDReaderFieldsWidget)

  ADD_PARAVIEW_PROPERTY_WIDGET(EXTRACTGROUP_FIELDS_IFACES EXTRACTGROUP_FIELDS_IFACES_SRCS
    TYPE "ExtractGroupFieldsWidgetType"
    CLASS_NAME pqExtractGroupFieldsWidget)

  # create the med reader plugin itself
  ADD_PARAVIEW_PLUGIN(MEDReaderPlugin "4.0"
    SERVER_MANAGER_SOURCES ${SM_SRCS}
    SERVER_MANAGER_XML Resources/MEDReaderServer.xml
    GUI_INTERFACES
      ${MEDREADER_TIMEMODE_IFACES}
      ${MEDREADER_RELOAD_IFACES}
      ${MEDREADER_TIMESFLAGS_IFACES}
      ${MEDREADER_FIELDS_IFACES}
      ${MEDREADER_VECTORS_IFACES}
      ${EXTRACTGROUP_FIELDS_IFACES}
    GUI_RESOURCES Resources/pqMEDReader.qrc
    SOURCES
      ${CLIENT_SRCS}
      ${CLIENT_UI_SRCS}
      ${MOC_SRCS}
      ${MEDREADER_TIMEMODE_IFACE_SRCS}
      ${MEDREADER_RELOAD_IFACE_SRCS}
      ${MEDREADER_TIMESFLAGS_IFACE_SRCS}
      ${MEDREADER_FIELDS_IFACE_SRCS}
      ${MEDREADER_VECTORS_IFACE_SRCS}
      ${EXTRACTGROUP_FIELDS_IFACES_SRCS}
    CS_KITS
    # this is the same name as the name in module.cmake file IO
    # directory.
    vtkMEDReader
    )

ELSE(PARAVIEW_BUILD_QT_GUI)

  # create the med reader plugin itself
  ADD_PARAVIEW_PLUGIN(MEDReaderPlugin "4.0"
    SERVER_MANAGER_SOURCES ${SM_SRCS}
    SERVER_MANAGER_XML Resources/MEDReaderServer.xml
    CS_KITS
    # this is the same name as the name in module.cmake file IO
    # directory.
    vtkMEDReader
    )

ENDIF(PARAVIEW_BUILD_QT_GUI)

TARGET_LINK_LIBRARIES(MEDReaderPlugin vtkMEDReader vtkMEDReaderCS)
# Install
INSTALL(
  TARGETS MEDReaderPlugin
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )


