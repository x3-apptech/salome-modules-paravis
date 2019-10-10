set(classes
  vtkDevelopedSurface
  VTKToMEDMem
)

vtk_module_add_module(DevelopedSurfaceModule
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(DevelopedSurfaceModule PRIVATE ${MEDCOUPLING_INCLUDE_DIRS})

if(HDF5_IS_PARALLEL)
  target_link_libraries(DevelopedSurfaceModule PRIVATE ${MEDCoupling_paramedloader})
else()
  target_link_libraries(DevelopedSurfaceModule PRIVATE ${MEDCoupling_medloader})
endif()
