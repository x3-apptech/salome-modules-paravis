# Copyright (C) 2014-2016  CEA/DEN, EDF R&D
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

PROJECT(ArrayRenamer)

cmake_minimum_required(VERSION 2.8)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

SET (outifaces)	
SET (outsrcs)

QT4_WRAP_CPP(moc_srcs pqArraysEditorPropertyWidget.h pqEditComponents.h)

  ADD_PARAVIEW_PROPERTY_GROUP_WIDGET(outifaces outsrcs
    TYPE "custom_arrays_editor"
    CLASS_NAME pqArraysEditorPropertyWidget)

ADD_PARAVIEW_PLUGIN(ArrayRenamer "1.0"
  SERVER_MANAGER_XML ArrayRenamer.xml
  SERVER_MANAGER_SOURCES vtkArrayRenamerFilter.cxx
  GUI_INTERFACES ${outifaces}
  GUI_RESOURCES pqArrayRenamer.qrc
  SOURCES ${outsrcs} ${moc_srcs} pqArraysEditorPropertyWidget.cxx
  )

INSTALL(TARGETS ArrayRenamer
  DESTINATION lib/paraview
)
