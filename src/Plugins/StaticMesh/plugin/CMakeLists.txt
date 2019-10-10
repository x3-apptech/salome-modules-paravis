
paraview_add_plugin(StaticMesh
  VERSION "1.0"
  MODULES StaticMeshModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/StaticMeshModule/vtk.module"
  SERVER_MANAGER_XML sources.xml
)

install(TARGETS StaticMesh
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
