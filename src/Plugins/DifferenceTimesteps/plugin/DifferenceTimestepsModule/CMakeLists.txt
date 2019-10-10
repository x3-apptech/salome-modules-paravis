set(classes
  vtkDifferenceTimestepsFilter
)

vtk_module_add_module(DifferenceTimestepsModule
  FORCE_STATIC
  CLASSES ${classes}
)
