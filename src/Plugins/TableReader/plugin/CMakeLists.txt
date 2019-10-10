paraview_add_plugin(TableReaderPlugin
  VERSION "1.0"
  MODULES TableReaderModule TableTo3DModule
  MODULE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/TableReaderModule/vtk.module"
    "${CMAKE_CURRENT_SOURCE_DIR}/TableTo3DModule/vtk.module"
  SERVER_MANAGER_XML filters.xml sources.xml
)

install(TARGETS TableReaderPlugin
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
