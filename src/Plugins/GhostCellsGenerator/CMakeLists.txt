CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(GhostCellsGenerator)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

IF(PARAVIEW_USE_MPI)
# Necessary while this class is not in VTK
  SET(SOURCES vtkPUnstructuredGridGhostCellsGenerator.cxx)
ENDIF(PARAVIEW_USE_MPI)

ADD_PARAVIEW_PLUGIN(GhostCellsGenerator "1.0"
  SERVER_MANAGER_SOURCES vtkPVUnstructuredGridGhostCellsGenerator.cxx
  SERVER_MANAGER_XML GhostCellsGenerator.xml
  SOURCES ${SOURCES}
  REQUIRED_ON_SERVER)

INSTALL( TARGETS GhostCellsGenerator 
  RUNTIME DESTINATION lib/paraview
  LIBRARY DESTINATION lib/paraview
  ARCHIVE DESTINATION lib/paraview
  )
