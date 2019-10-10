set(classes
  VTKToMEDMem
  vtkMEDWriter
)

vtk_module_add_module(MEDWriterIO
  FORCE_STATIC
  CLASSES ${classes}
)

target_include_directories(MEDWriterIO PRIVATE ${MEDCOUPLING_INCLUDE_DIRS})

if(HDF5_IS_PARALLEL)
  target_link_libraries(MEDWriterIO PRIVATE ${MEDCoupling_paramedloader})
else()
  target_link_libraries(MEDWriterIO PRIVATE ${MEDCoupling_medloader})
endif()

