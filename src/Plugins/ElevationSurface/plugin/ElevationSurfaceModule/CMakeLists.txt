set(classes
  vtkElevationSurfaceFilter
)

vtk_module_add_module(ElevationSurfaceModule
  FORCE_STATIC
  CLASSES ${classes}
)
