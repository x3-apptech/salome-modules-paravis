
paraview_add_plugin(ElevationSurface
  VERSION "1.0"
  MODULES ElevationSurfaceModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ElevationSurfaceModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
  )

install(TARGETS ElevationSurface
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
