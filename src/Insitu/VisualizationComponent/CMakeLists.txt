
# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/idl
  ${CMAKE_CURRENT_SOURCE_DIR}/../VisualizationLibrary
)

# --- definitions ---
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
)

# find additional libraries
#FIND_LIBRARY( _userlib_visulib visulib PATH /home/I35256/salome/install/V7_main_git/modules/install/PARAVIS_master/lib/salome)
#FIND_LIBRARY( _userlib_MEDLoaderForPV MEDLoaderForPV PATH /home/I35256/salome/install/V7_main_git/modules/install/PARAVIS_master/lib/salome)


# libraries to link to
SET(_link_LIBRARIES
  ${OMNIORB_LIBRARIES}
  ${KERNEL_SalomeIDLKernel}
  ${KERNEL_OpUtil}
  ${KERNEL_SalomeContainer}
  SalomeIDLPARAVIS
  ${KERNEL_SalomeMPIContainer}
  #${_userlib_visulib}
  #${_userlib_MEDLoaderForPV}
  ${PARAVIS_MEDLoaderForPV}
  ${PARAVIS_visulib}
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
#SET_TARGET_PROPERTIES( VisualizationComponentEngine PROPERTIES INSTALL_RPATH /home/I35256/salome/install/V7_main_git/modules/install/PARAVIS_master/lib/salome)

INSTALL(TARGETS VisualizationComponentEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${PARAVIS_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
