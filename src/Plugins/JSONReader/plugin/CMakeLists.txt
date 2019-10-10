
paraview_add_plugin(JSONReader
  VERSION "1.0"
  MODULES JSONReaderModule
  MODULE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/JSONParserModule/vtk.module"
    "${CMAKE_CURRENT_SOURCE_DIR}/JSONReaderModule/vtk.module"
  SERVER_MANAGER_XML sources.xml
  )

install(TARGETS JSONReader
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
