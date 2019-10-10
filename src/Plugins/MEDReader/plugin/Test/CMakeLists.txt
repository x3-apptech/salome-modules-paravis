if (PARAVIEW_ENABLE_PYTHON)
  file(GLOB pythonTests *.py)
  list(FILTER pythonTests EXCLUDE REGEX ".*MEDReaderHelper.py$")
  foreach(testFullPath ${pythonTests})
    get_filename_component(testFile ${testFullPath} NAME)
    paraview_add_test_python(NO_RT DIRECT_DATA ${testFile})
    get_filename_component(testName ${testFile} NAME_WE)
    set_tests_properties(Python-${testName} PROPERTIES ENVIRONMENT "PARAVIEW_DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/Data;LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib;PYTHONPATH=${MEDCOUPLING_ROOT_DIR}/lib/python3.6/site-packages/;PV_PLUGIN_PATH=${CMAKE_BINARY_DIR}/lib/paraview-5.7/plugins/MEDReader;PARAVIEW_BIN_DIR=${ParaView_DIR}/bin/")
  endforeach()
endif()

if (PARAVIEW_USE_QT)
  file(GLOB xmlTests *.xml)
  foreach(testFullPath ${xmlTests})
    get_filename_component(testFile ${testFullPath} NAME)
    get_filename_component(testName ${testFile} NAME_WE)
    set (${testName}_USES_DIRECT_DATA ON)
    paraview_add_client_tests(
      LOAD_PLUGIN MEDReader
      PLUGIN_PATH $<TARGET_FILE_DIR:MEDReader>
      BASELINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Data/Baseline
      TEST_SCRIPTS ${testFile})
    set_tests_properties(pv.${testName} PROPERTIES ENVIRONMENT "PARAVIEW_DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/Data;LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib")
  endforeach()
endif()
