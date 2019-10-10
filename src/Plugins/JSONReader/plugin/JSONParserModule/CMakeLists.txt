set(classes
  vtkJSONParser
)

vtk_module_add_module(JSONParserModule
  FORCE_STATIC
  CLASSES ${classes}
)
