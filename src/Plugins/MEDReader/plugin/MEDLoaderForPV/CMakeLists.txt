add_library(MEDLoaderForPV SHARED MEDFileFieldRepresentationTree.cxx  MEDTimeReq.cxx  MEDUtilities.cxx  vtkGenerateVectors.cxx)
target_include_directories(MEDLoaderForPV PRIVATE . ${MEDCOUPLING_INCLUDE_DIRS})

target_link_libraries(MEDLoaderForPV VTK::CommonCore VTK::CommonDataModel VTK::IOXML ${MEDFILE_C_LIBRARIES})

IF(HDF5_IS_PARALLEL)
  target_link_libraries(MEDLoaderForPV ${MEDCoupling_paramedloader})
ELSE(HDF5_IS_PARALLEL)
  TARGET_LINK_LIBRARIES(MEDLoaderForPV ${MEDCoupling_medloader})
ENDIF(HDF5_IS_PARALLEL)

install(TARGETS MEDLoaderForPV 
        DESTINATION lib/salome)#${CMAKE_INSTALL_LIBDIR}
