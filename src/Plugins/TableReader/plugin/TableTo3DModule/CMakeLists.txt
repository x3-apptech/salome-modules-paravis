set(classes
  vtkTableTo3D
)

vtk_module_add_module(TableTo3DModule
  FORCE_STATIC
  CLASSES ${classes}
)
