set(private_classes
  vtkStaticDataSetSurfaceFilter
  vtkStaticEnSight6BinaryReader
  vtkStaticEnSight6Reader
  vtkStaticEnSightGoldBinaryReader
  vtkStaticEnSightGoldReader
  vtkStaticMeshObjectFactory
  vtkStaticPlaneCutter
)

set(classes
  vtkTemporalUGWavelet
)

if (PARAVIEW_USE_MPI)
  list(APPEND private_classes vtkStaticPUnstructuredGridGhostCellsGenerator)
endif()

vtk_module_add_module(StaticMeshModule
  FORCE_STATIC
  CLASSES ${classes}
  PRIVATE_CLASSES ${private_classes}
)
