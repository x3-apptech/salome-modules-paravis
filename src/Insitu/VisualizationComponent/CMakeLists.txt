
# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${MED_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../VisualizationLibrary
)

# --- definitions ---
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${MED_DEFINITIONS}
  ${MEDCOUPLING_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${OMNIORB_LIBRARIES}
  ${KERNEL_SalomeIDLKernel}
  ${KERNEL_OpUtil}
  ${KERNEL_SalomeContainer}
  SalomeIDLPARAVIS
  ${KERNEL_SalomeMPIContainer}
  MEDLoaderForPV
  visulib
  ${MED_SalomeIDLMED}
  ${MED_med}
  ${MED_medcouplingcorba}
  ${MED_medcouplingclient}

  ${MED_paramedmemcompo}
  ${MED_paramedcouplingcorba}  
)

# --- headers ---

# header files / no moc processing

SET(PARAVIS_HEADERS
  VisualizationComponent.hxx
)

# --- sources ---

# sources / static
SET(PARAVIS_SOURCES
  VisualizationComponent.cxx
  
)

# --- rules ---

ADD_LIBRARY(VisualizationComponentEngine ${PARAVIS_SOURCES})
TARGET_LINK_LIBRARIES(VisualizationComponentEngine ${_link_LIBRARIES} )

INSTALL(TARGETS VisualizationComponentEngine
        DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${PARAVIS_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
