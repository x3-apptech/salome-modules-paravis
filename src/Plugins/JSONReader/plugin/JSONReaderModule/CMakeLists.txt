set(classes
  vtkJSONReader
)

vtk_module_add_module(JSONReaderModule
  FORCE_STATIC
  CLASSES ${classes}
)
