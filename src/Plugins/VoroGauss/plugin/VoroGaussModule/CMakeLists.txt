set(classes
  vtkVoroGauss
)

vtk_module_add_module(VoroGaussModule
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(VoroGaussModule PRIVATE ${MEDCOUPLING_INCLUDE_DIRS})

if(HDF5_IS_PARALLEL)
  target_link_libraries(VoroGaussModule PRIVATE ${MEDCoupling_paramedloader})
else()
  target_link_libraries(VoroGaussModule PRIVATE ${MEDCoupling_medloader})
endif()
