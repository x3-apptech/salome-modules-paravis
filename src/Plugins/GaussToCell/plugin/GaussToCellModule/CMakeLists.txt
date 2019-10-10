set(classes
  vtkGaussToCell
)

vtk_module_add_module(GaussToCellModule
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(GaussToCellModule PRIVATE ${MEDCOUPLING_INCLUDE_DIRS})

if(HDF5_IS_PARALLEL)
  target_link_libraries(GaussToCellModule PRIVATE ${MEDCoupling_paramedloader})
else()
  target_link_libraries(GaussToCellModule PRIVATE ${MEDCoupling_medloader})
endif()

