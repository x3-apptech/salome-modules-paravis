include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/ParaMEDMEM2VTK )

paraview_add_plugin(ParaMEDCorbaPlugin
  VERSION "1.0"
  MODULES ParaMEDCorbaModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ParaMEDCorbaModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

add_subdirectory(ParaMEDMEM2VTK)

install(TARGETS ParaMEDCorbaPlugin
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
