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

#salome_accumulate_environment(PARAVIEW_DATA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/Data)
salome_generate_tests_environment(tests_env)
list(APPEND tests_env "PARAVIEW_DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/Data")

if (PARAVIEW_ENABLE_PYTHON)
  file(GLOB pythonTests *.py)
  list(FILTER pythonTests EXCLUDE REGEX ".*MEDReaderHelper.py$")
  foreach(testFullPath ${pythonTests})
    get_filename_component(testFile ${testFullPath} NAME)
    paraview_add_test_python(NO_RT DIRECT_DATA ${testFile})
    get_filename_component(testName ${testFile} NAME_WE)
#   set_tests_properties(Python-${testName} PROPERTIES ENVIRONMENT "PARAVIEW_DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/Data;LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib;PYTHONPATH=${MEDCOUPLING_ROOT_DIR}/lib/python3.6/site-packages/;PV_PLUGIN_PATH=${CMAKE_BINARY_DIR}/lib/paraview-5.7/plugins/MEDReader;PARAVIEW_BIN_DIR=${ParaView_DIR}/bin/")
    set_tests_properties(Python-${testName} PROPERTIES ENVIRONMENT "${tests_env}")
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
#   set_tests_properties(pv.${testName} PROPERTIES ENVIRONMENT "PARAVIEW_DATA_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/Data;LD_LIBRARY_PATH=${QT5_ROOT_DIR}/lib/:${MEDCOUPLING_ROOT_DIR}/lib/:${MEDFILE_ROOT_DIR}/lib/:${HDF5_ROOT_DIR}/lib:${PYTHON_ROOT_DIR}/lib")
    set_tests_properties(pv.${testName} PROPERTIES ENVIRONMENT "${tests_env}")
  endforeach()
endif()
