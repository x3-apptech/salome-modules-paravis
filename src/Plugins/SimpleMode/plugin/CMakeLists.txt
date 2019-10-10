
paraview_add_plugin(SimpleModePlugin
  VERSION "1.0"
  MODULES SimpleModeModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/SimpleModeModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS SimpleModePlugin
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
