# Copyright (C) 2016-2021  CEA/DEN, EDF R&D
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

IF(SALOME_PARAVIS_BUILD_INSITU)
  INCLUDE(UseOmniORB)  # Provided by KERNEL

  INCLUDE_DIRECTORIES(
    ${OMNIORB_INCLUDE_DIR}
    ${KERNEL_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}/idl
    ${FIELDS_INCLUDE_DIRS}
    ${MEDCOUPLING_INCLUDE_DIRS}
  )

  SET(SalomeIDLPARAVIS_IDLSOURCES
    PARAVIS.idl
    
  )

  SET(_idl_include_dirs
    ${KERNEL_ROOT_DIR}/idl/salome
    ${FIELDS_ROOT_DIR}/idl/salome
    
  )

  SET(_idl_link_flags
    ${KERNEL_SalomeIDLKernel}
    ${FIELDS_SalomeIDLMED}
    
  )

  OMNIORB_ADD_MODULE(SalomeIDLPARAVIS "${SalomeIDLPARAVIS_IDLSOURCES}" "${_idl_include_dirs}" "${_idl_link_flags}")
  INSTALL(TARGETS SalomeIDLPARAVIS EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})
ENDIF(SALOME_PARAVIS_BUILD_INSITU)
