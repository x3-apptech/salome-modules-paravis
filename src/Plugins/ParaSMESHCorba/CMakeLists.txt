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

 PROJECT(ParaSMESHCorba)
 CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )

 FIND_PACKAGE(ParaView REQUIRED)
 INCLUDE(${PARAVIEW_USE_FILE})

 SET(PARA_SMESH_CORBA_SRC
    ${ParaSMESHCorba_SOURCE_DIR}/vtkParaSMESHCorbaSource.cxx 
    )

 ADD_PARAVIEW_PLUGIN( ParaSMESHCorbaPlugin "1.0"
      SERVER_MANAGER_XML ParaSMESHCorbaServerManager.xml
      SERVER_MANAGER_SOURCES ${PARA_SMESH_CORBA_SRC} )

 INCLUDE_DIRECTORIES(
     ${OMNIORB_INCLUDE_DIR}
     ${SMESH_ROOT_DIR}/include/salome 
     ${GEOM_ROOT_DIR}/include/salome
     ${KERNEL_ROOT_DIR}/include/salome )
     
 ADD_DEFINITIONS(${OMNIORB_DEFINITIONS})
 TARGET_LINK_LIBRARIES(ParaSMESHCorbaPlugin
      ${OMNIORB_LIBRARIES}
      ${KERNEL_SalomeIDLKernel}
      ${SMESH_SalomeIDLSMESH}
      ${SMESH_SMESHClient}
      ${GEOM_SalomeIDLGEOM}
  )

 INSTALL(TARGETS ParaSMESHCorbaPlugin DESTINATION lib/paraview)
 INSTALL(FILES ParaSMESHCorbaServerManager.xml DESTINATION share)
