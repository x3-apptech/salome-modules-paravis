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

INCLUDE(UseOmniORB)  # Provided by KERNEL

INCLUDE_DIRECTORIES(
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

ADD_DEFINITIONS(${OMNIORB_DEFINITIONS})

SET(SalomeIDLPARAVIS_IDLSOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/PVSERVER_Gen.idl
  )
SET(_idl_include_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KERNEL_ROOT_DIR}/idl/salome
  )  
SET(_idl_link_flags
  ${KERNEL_SalomeIDLKernel}
)

# Demand Python generation:
SET(OMNIORB_PYTHON_BACKEND 1)
OMNIORB_ADD_MODULE(SalomeIDLPVServer "${SalomeIDLPARAVIS_IDLSOURCES}" "${_idl_include_dirs}" "${_idl_link_flags}")
INSTALL(TARGETS SalomeIDLPVServer EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})
