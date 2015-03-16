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

SET(_PYFILES_TO_INSTALL)

IF(SALOME_LIGHT_ONLY)
  LIST(APPEND _PYFILES_TO_INSTALL
    pvsimple.py
    presentations.py
  )
ELSE(SALOME_LIGHT_ONLY)
    # Use a set of Python files redirecting the API directly 
    # to the original ParaView modules.
    LIST(APPEND _PYFILES_TO_INSTALL
      pvserver.py
      pvsimple.py
      presentations.py
      paravisSM.py
    )
ENDIF(SALOME_LIGHT_ONLY)

INSTALL_AND_COMPILE_PYTHON_FILE("${_PYFILES_TO_INSTALL}" ${SALOME_INSTALL_PYTHON})
