set(classes
  vtkELNOFilter
  vtkELNOMeshFilter
  vtkELNOSurfaceFilter
  vtkExtractGroup
  vtkExtractCellType
  vtkMEDQuadraturePointsGenerator
  vtkMEDReader
  vtkPVMetaDataInformation
)

vtk_module_add_module(MEDReaderIO
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(MEDReaderIO PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/../MEDLoaderForPV"
  ${MEDCOUPLING_INCLUDE_DIRS})

target_link_libraries(MEDReaderIO PUBLIC MEDLoaderForPV)
