# Copyright (C) 2015-2016  CEA/DEN, EDF R&D
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

SET(TEST_DIRECTORIES
  2D_viewer
  3D_viewer
  ScalarMap
  DeformedShape
  ScalarMap_On_DeformedShape
  CutPlanes
  CutLines
  Vectors
  Plot3D
  IsoSurfaces
  MeshPresentation
  Animation
  GaussPoints
  StreamLines
  SWIG_scripts
  Tables
  ImportMedField
  united
  bugs
  imps
  dump_study
)

FOREACH(test_dir ${TEST_DIRECTORIES})
  SUBDIRS(${test_dir})
ENDFOREACH()
