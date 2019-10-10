set(classes
  vtkScaleVectorFilter
)

vtk_module_add_module(ScaleVectorModule
  FORCE_STATIC
  CLASSES ${classes}
)
