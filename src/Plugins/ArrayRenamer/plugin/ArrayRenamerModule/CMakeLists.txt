set(classes
  vtkArrayRenamerFilter
)

vtk_module_add_module(ArrayRenamerModule
  FORCE_STATIC
  CLASSES ${classes}
)
