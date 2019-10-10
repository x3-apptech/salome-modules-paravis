set(qrc_files)
if(PARAVIEW_USE_QT)
  list(APPEND qrc_files pqDifferenceTimesteps.qrc)
endif()

paraview_add_plugin(DifferenceTimesteps
  VERSION "1.0"
  UI_RESOURCES ${qrc_files}
  MODULES DifferenceTimestepsModule
  MODULE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DifferenceTimestepsModule/vtk.module"
  SERVER_MANAGER_XML filters.xml
)

install(TARGETS DifferenceTimesteps
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
