# Copyright (C) 2015 CEA/DEN, EDF R&D
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

# Author : Roman NIKOLAEV

PROJECT(JSONReader)

cmake_minimum_required(VERSION 2.8)

# Find ParaView
FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

# Standard CMake option for building libraries shared or static by default.
OPTION(BUILD_SHARED_LIBS "Build with shared libraries" ${VTK_BUILD_SHARED_LIBS})

# Add subdirectories
# ADD_SUBDIRECTORY(JSONParser)
#IF(SALOME_BUILD_TESTS)
#  ENABLE_TESTING()
#  ADD_SUBDIRECTORY(Test)
#ENDIF()

ADD_SUBDIRECTORY(ParaViewPlugin)
