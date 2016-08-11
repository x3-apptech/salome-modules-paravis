
SET(BUILD_SHARED_LIBS TRUE)
INCLUDE_DIRECTORIES(
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${MPI_INCLUDE_PATH}
  ${PARAVIS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../Plugins/MEDReader/IO/
)


SET(_link_LIBRARIES
  ${MEDCoupling_medcoupling}
  ${MEDCoupling_interpkernel}
  ${MPI_LIBRARIES}
  ${VTK_LIBRARIES}
  ${PARAVIS_MEDLoaderForPV}
  #${MEDLoaderForPV}
  ${MEDCoupling_medloader}
  ${MEDFILE_C_LIBRARIES}
  vtkPVCatalyst
  vtkPVPythonCatalyst
)

SET(_lib_HEADERS
  visu.hxx
)

SET(_lib_SOURCES
  visu.cxx
)

ADD_LIBRARY(visulib ${_lib_SOURCES})
TARGET_LINK_LIBRARIES(visulib ${_link_LIBRARIES} )

INSTALL(TARGETS visulib
  RUNTIME DESTINATION lib/salome
  LIBRARY DESTINATION lib/salome
  ARCHIVE DESTINATION lib/salome
  )
INSTALL(FILES ${_lib_HEADERS} DESTINATION include/salome )


