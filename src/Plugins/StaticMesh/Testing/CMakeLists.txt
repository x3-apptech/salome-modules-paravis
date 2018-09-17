include(ParaViewTestingMacros)

if (PARAVIEW_BUILD_QT_GUI)
  add_client_tests("pv"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticDataSetSurfaceFilter.xml)
  add_client_tests("pv"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticPlaneCutter.xml)
  add_client_tests("pvcs"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticDataSetSurfaceFilter.xml)
  add_client_tests("pvcs"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticPlaneCutter.xml)
  add_client_tests("pvcrs"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticDataSetSurfaceFilter.xml)
  add_client_tests("pvcrs"
    LOAD_PLUGIN "StaticMesh"
    BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
    TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticPlaneCutter.xml)

  if (PARAVIEW_USE_MPI)
    add_client_server_tests("pvcs"
      LOAD_PLUGIN "StaticMesh"
      BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
      TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticGhostCellGenerator.xml)
    add_client_render_server_tests("pvcrs"
      LOAD_PLUGIN "StaticMesh"
      BASELINE_DIR ${PARAVIEW_TEST_BASELINE_DIR}
      TEST_SCRIPTS ${CMAKE_CURRENT_SOURCE_DIR}/StaticGhostCellGenerator.xml)
  endif()
endif()
