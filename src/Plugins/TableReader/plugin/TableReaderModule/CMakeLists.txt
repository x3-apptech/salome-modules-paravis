set(classes
  vtkVisuTableReader
)

vtk_module_add_module(TableReaderModule
  FORCE_STATIC
  CLASSES ${classes}
)
