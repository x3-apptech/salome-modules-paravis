# Copyright (C) 2010-2015  CEA/DEN, EDF R&D
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

SET(BASE_TESTS A0 B1 C3 D0 E0)
SET(ALL_TESTS  A0 A1 A2 A3 A4 A5 A6 A7 A9 B1 C3 C4 C5 C6 C7 C8 C9
               D0 D1 D3 D5 D6 D7 E0)

# For make test
IF(SALOME_PARAVIS_ALL_TEST)
  SET(TEST_NAMES ${ALL_TESTS})
ELSE()
  SET(TEST_NAMES ${BASE_TESTS})
ENDIF()

FOREACH(tfile ${TEST_NAMES})
  SET(TEST_NAME BUGS_${tfile})
  ADD_TEST(${TEST_NAME} ${PYTHON_EXECUTABLE} ${SALOME_TEST_DRIVER} ${TIMEOUT}
    ${CMAKE_CURRENT_SOURCE_DIR}/${tfile}.py)
  SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES FAIL_REGULAR_EXPRESSION "FAILED" TIMEOUT ${TIMEOUT})
  SET_TESTS_PROPERTIES(${TEST_NAME} PROPERTIES ENVIRONMENT "${tests_env}")
ENDFOREACH()

# Application tests
SET(APPLICATION_TESTS ${ALL_TESTS})
SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test/VisuPrs/bugs)

FOREACH(tfile ${APPLICATION_TESTS})
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${tfile}.py
          DESTINATION ${TEST_INSTALL_DIRECTORY})
ENDFOREACH()

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
