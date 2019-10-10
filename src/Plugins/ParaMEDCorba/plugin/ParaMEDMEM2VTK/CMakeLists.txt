include_directories(
  ${OMNIORB_INCLUDE_DIR}
  ${FIELDS_ROOT_DIR}/include/salome
  ${KERNEL_ROOT_DIR}/include/salome
  ${CMAKE_CURRENT_SOURCE_DIR}/../ParaMEDCorbaModule
  )

add_definitions(${OMNIORB_DEFINITIONS})

add_library(ParaMEDMEM2VTK SHARED VTKMEDCouplingMeshClient.cxx VTKMEDCouplingUMeshClient.cxx VTKMEDCouplingCMeshClient.cxx VTKMEDCouplingFieldClient.cxx VTKParaMEDFieldClient.cxx VTKMEDCouplingMultiFieldsClient.cxx VTKMEDCouplingCurveLinearMeshClient.cxx)

target_link_libraries(ParaMEDMEM2VTK
  VTK::CommonCore
  VTK::CommonDataModel
  VTK::CommonExecutionModel
  VTK::FiltersGeneral
  ParaView::VTKExtensionsMisc
  ${OMNIORB_LIBRARIES}
  ${FIELDS_SalomeIDLMED}
  ${KERNEL_SalomeIDLKernel}
  )

#Build library with a static runtime and ignore MSVCRT.lib on WINDOWS
if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    set_target_properties(ParaMEDMEM2VTK PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRTD")
    set_target_properties(ParaMEDMEM2VTK PROPERTIES COMPILE_FLAGS "/MTd")
  else()
    set_target_properties(ParaMEDMEM2VTK PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
    set_target_properties(ParaMEDMEM2VTK PROPERTIES COMPILE_FLAGS "/MT")
  endif()
endif(WIN32)

install(TARGETS ParaMEDMEM2VTK EXPORT DESTINATION ${SALOME_INSTALL_LIBS})
