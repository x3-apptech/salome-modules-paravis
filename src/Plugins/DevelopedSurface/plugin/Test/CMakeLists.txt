# Copyright (C) 2017-2019  EDF R&D
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

SET(TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary")

IF(NOT EXISTS ${TEMP_DIR})
  FILE(MAKE_DIRECTORY ${TEMP_DIR})
ENDIF(NOT EXISTS ${TEMP_DIR})

SET(DEV_SURFACE_TESTS test_dev_surface2 test_dev_surface3)

IF(NOT SALOME_PARAVIS_NO_VISU_TESTS)
  FOREACH(tfile ${DEV_SURFACE_TESTS})
    ADD_TEST(${tfile} ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${tfile}.py )
    SET_TESTS_PROPERTIES(${tfile} PROPERTIES LABELS "PVS_ADD_ONS")
  ENDFOREACH(tfile ${DEV_SURFACE_TESTS})
ENDIF()
