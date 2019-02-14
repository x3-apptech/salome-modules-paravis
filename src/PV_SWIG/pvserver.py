# Copyright (C) 2010-2019  CEA/DEN, EDF R&D
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
#  File   : paravis.py
#  Module : PARAVIS
#

"""
This module does the minimum required to retrieve the PVSERVER engine and make
it available in the global variables myParavis and myPVServerService
(TODO: who needs this ??)
"""

import PVSERVER_utils

## Initialize PARAVIS interface
myPVServerService = PVSERVER_utils.getService()
# Backward compatibility:
myParavis = myPVServerService 
