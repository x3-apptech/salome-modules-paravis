# Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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
# Author: Adrien Bruneton (CEA)

PROJECT(LightPARAVIS)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10)
SET(CMAKE_BUILD_TYPE "Debug")

OPTION(LIGHTPARAVIS_WITH_GUI "Build GUI test app" ON)

# Package detection
FIND_PACKAGE(Qt4 REQUIRED)
LIST(APPEND CMAKE_PREFIX_PATH "$ENV{PARAVIEW_ROOT_DIR}")
FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})
INCLUDE(ParaViewMacros)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Main applications
IF(LIGHTPARAVIS_WITH_GUI)
    ADD_SUBDIRECTORY(gui)
ENDIF()
ADD_SUBDIRECTORY(simple)
