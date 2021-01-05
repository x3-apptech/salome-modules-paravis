# Copyright (C) 2010-2020  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  
  ###########################
  # Tests for standalone mode
  ###########################

  if (PARAVIEW_ENABLE_PYTHON)
    file(GLOB pythonTests *.py)
    foreach(testFullPath ${pythonTests})
      get_filename_component(testFile ${testFullPath} NAME)
      paraview_add_test_python(NO_RT DIRECT_DATA ${testFile})
      get_filename_component(testName ${testFile} NAME_WE)
      set_tests_properties(Python-${testName} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib;PYTHONPATH=${MEDCOUPLING_ROOT_DIR}/lib/python3.6/site-packages/;PV_PLUGIN_PATH=${CMAKE_BINARY_DIR}/lib/paraview-5.8/plugins/MEDWriter;PARAVIEW_BIN_DIR=${ParaView_DIR}/bin/")
    endforeach()
  endif()

  if(PARAVIEW_USE_QT)
    file(GLOB xmlTests *.xml)
    foreach(testFullPath ${xmlTests})
      get_filename_component(testFile ${testFullPath} NAME)
      get_filename_component(testName ${testFile} NAME_WE)
      set (${testName}_USES_DIRECT_DATA ON)
      paraview_add_client_tests(
        LOAD_PLUGIN MEDWriter
        PLUGIN_PATH $<TARGET_FILE_DIR:MEDWriter>
        TEST_SCRIPTS ${testFile})
        set_tests_properties(pv.${testName} PROPERTIES ENVIRONMENT "LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib")
    endforeach()
  endif()

else()

  ########################
  # Tests for PARAVIS mode
  ########################

  include(tests.set)

  salome_generate_tests_environment(tests_env)

  IF(SALOME_PARAVIS_NO_VISU_TESTS)
    SET(_other_options "-D")
  ENDIF()

  foreach(tfile ${TEST_NUMBERS})
    add_test(Python-TestMEDWriter${tfile}
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/TestMEDWriter${tfile}.py
             ${_other_options})
    set_tests_properties(Python-TestMEDWriter${tfile} PROPERTIES ENVIRONMENT "${tests_env}")
  endforeach()

  # Application tests
  set(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test/MEDWriter)
  foreach(tfile ${TEST_NUMBERS})
    install(FILES TestMEDWriter${tfile}.py
            DESTINATION ${TEST_INSTALL_DIRECTORY})
  endforeach()

  install(FILES tests.set
          DESTINATION ${TEST_INSTALL_DIRECTORY})
  install(FILES CTestTestfileInstall.cmake
          DESTINATION ${TEST_INSTALL_DIRECTORY}
          RENAME CTestTestfile.cmake)

endif()
