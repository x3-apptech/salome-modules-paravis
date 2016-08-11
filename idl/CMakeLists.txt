
INCLUDE(UseOmniORB)  # Provided by KERNEL

INCLUDE_DIRECTORIES(
  ${OMNIORB_INCLUDE_DIR}
  ${KERNEL_INCLUDE_DIRS}
  ${PROJECT_BINARY_DIR}/idl
)

SET(SalomeIDLPARAVIS_IDLSOURCES
  PARAVIS.idl
  
)

SET(_idl_include_dirs
  ${KERNEL_ROOT_DIR}/idl/salome
  ${MED_ROOT_DIR}/idl/salome
  
)

SET(_idl_link_flags
  ${KERNEL_SalomeIDLKernel}
  ${MED_SalomeIDLMED}
  
)

OMNIORB_ADD_MODULE(SalomeIDLPARAVIS "${SalomeIDLPARAVIS_IDLSOURCES}" "${_idl_include_dirs}" "${_idl_link_flags}")
INSTALL(TARGETS SalomeIDLPARAVIS EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})
