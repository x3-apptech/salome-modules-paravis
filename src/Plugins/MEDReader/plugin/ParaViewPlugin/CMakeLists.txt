set(interfaces)
set(ui_files)
set(sources)

if(PARAVIEW_USE_QT)
  set(ui_files
    MEDReaderVectorsWidget.ui
    MEDReaderTimeModeWidget.ui)

  set(sources
    pqAbstractFieldsWidget.cxx
    VectBoolSpreadSheet.cxx
    VectBoolWidget.cxx
    pqExtractGroupFieldsWidget.cxx
    pqMEDReaderGraphUtils.cxx
    pqMEDReaderVectorsWidget.cxx
    pqMEDReaderTimeModeWidget.cxx
    pqMEDReaderReloadWidget.cxx
    pqMEDReaderTimesFlagsWidget.cxx
    pqMEDReaderFieldsWidget.cxx)

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "MEDReaderTimeModeWidgetType"
    CLASS_NAME pqMEDReaderTimeModeWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "MEDReaderReloadWidgetType"
    CLASS_NAME pqMEDReaderReloadWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "MEDReaderVectorsWidgetType"
    CLASS_NAME pqMEDReaderVectorsWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "MEDReaderTimesFlagsWidgetType"
    CLASS_NAME pqMEDReaderTimesFlagsWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "MEDReaderFieldsWidgetType"
    CLASS_NAME pqMEDReaderFieldsWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

  paraview_plugin_add_property_widget(
    KIND WIDGET
    TYPE "ExtractGroupFieldsWidgetType"
    CLASS_NAME pqExtractGroupFieldsWidget
    INTERFACES property_interfaces
    SOURCES property_sources)
  list(APPEND interfaces
    ${property_interfaces})
  list(APPEND sources
    ${property_sources})

endif(PARAVIEW_USE_QT)

paraview_add_plugin(MEDReader
  VERSION "5.0"
  UI_INTERFACES ${interfaces}
  UI_FILES ${ui_files}
  SOURCES ${sources}
  UI_RESOURCES Resources/pqMEDReader.qrc
  SERVER_MANAGER_XML Resources/MEDReaderServer.xml
  MODULES MEDReaderIO
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../MEDReaderIO/vtk.module"
  )

install(TARGETS MEDReader
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
