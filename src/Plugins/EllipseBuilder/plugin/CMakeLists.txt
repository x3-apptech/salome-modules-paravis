set(qrc_files)
if(PARAVIEW_USE_QT)
  list(APPEND qrc_files pqEllipseBuilder.qrc)
endif()

paraview_add_plugin(EllipseBuilderFilter
  VERSION "1.0"
  UI_RESOURCES ${qrc_files}
  MODULES EllipseBuilderModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/EllipseBuilderModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS EllipseBuilderFilter
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
