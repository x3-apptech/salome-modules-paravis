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

PROJECT( ParaGEOMCorba )

  CMAKE_MINIMUM_REQUIRED( VERSION 2.8.7 )
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE( ${PARAVIEW_USE_FILE} )
  SET( PARA_GEOM_CORBA_SRCS
    ${ParaGEOMCorba_SOURCE_DIR}/vtkParaGEOMCorbaSource.cxx
  )
  ADD_PARAVIEW_PLUGIN( ParaGEOMCorbaPlugin "1.0"
    SERVER_MANAGER_XML ParaGEOMCorbaServerManager.xml
    SERVER_MANAGER_SOURCES ${PARA_GEOM_CORBA_SRCS} )
    
  INCLUDE_DIRECTORIES(
     ${OMNIORB_INCLUDE_DIR} 
     ${GEOM_ROOT_DIR}/include/salome
     ${KERNEL_ROOT_DIR}/include/salome )
     
  ADD_DEFINITIONS(${OMNIORB_DEFINITIONS})
    TARGET_LINK_LIBRARIES(ParaGEOMCorbaPlugin
      ${OMNIORB_LIBRARIES}
      ${KERNEL_SalomeIDLKernel}
      ${KERNEL_SalomeLifeCycleCORBA}
      ${KERNEL_SalomeNS}
      ${GEOM_GEOMClient}
      ${GEOM_SalomeIDLGEOM}
      ${GEOM_OCC2VTK}
  )
    
  INSTALL(TARGETS ParaGEOMCorbaPlugin DESTINATION lib/paraview)
  INSTALL(FILES ParaGEOMCorbaServerManager.xml DESTINATION share)
