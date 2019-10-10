include_directories(
  ${FIELDS_ROOT_DIR}/include/salome
  )

add_definitions(${OMNIORB_DEFINITIONS})

set(classes
  vtkParaMEDCorbaSource
)

vtk_module_add_module(ParaMEDCorbaModule
  FORCE_STATIC
  CLASSES ${classes}
)

target_link_libraries(ParaMEDCorbaModule PRIVATE ParaMEDMEM2VTK ${OMNIORB_LIBRARIES} ${FIELDS_SalomeIDLMED})