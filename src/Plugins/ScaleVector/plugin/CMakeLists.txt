
paraview_add_plugin(ScaleVector
  VERSION "1.0"
  MODULES ScaleVectorModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ScaleVectorModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS ScaleVector
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
