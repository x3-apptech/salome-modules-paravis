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

SALOME_GENERATE_TESTS_ENVIRONMENT(tests_env)

SET(TEST_NUMBERS 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17)

FOREACH(tfile ${TEST_NUMBERS})
  ADD_TEST(testMEDReader${tfile} ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/testMEDReader${tfile}.py)
  SET_TESTS_PROPERTIES(testMEDReader${tfile} PROPERTIES ENVIRONMENT "${tests_env}")
ENDFOREACH()

# Application tests
SET(TEST_INSTALL_DIRECTORY ${SALOME_INSTALL_SCRIPT_SCRIPTS}/test/MEDReader)

FOREACH(tfile ${TEST_NUMBERS})
  INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/testMEDReader${tfile}.py
          DESTINATION ${TEST_INSTALL_DIRECTORY})
ENDFOREACH()

INSTALL(FILES CTestTestfileInstall.cmake
        DESTINATION ${TEST_INSTALL_DIRECTORY}
        RENAME CTestTestfile.cmake)
