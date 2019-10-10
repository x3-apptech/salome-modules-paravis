set(classes
  vtkEllipseBuilderFilter
)

vtk_module_add_module(EllipseBuilderModule
  FORCE_STATIC
  CLASSES ${classes}
)
