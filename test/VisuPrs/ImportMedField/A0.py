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

# This case corresponds to: /visu/ImportMedField/A0 case
# Import MED file; create presentations for the given fields.

from paravistest import datadir, Import_Med_Field

med_file = datadir + "ResOK_0000.med"
field_names1 = ["temperature", "vitesse", "pression"]
prs_list1 = [ [1,2,3,4,8], [1,2,3,4,5,6,8,9], [0,1,2,3,4,8] ]

Import_Med_Field(med_file, field_names1, 1, prs_list1)

# Stream Lines presentation on "vitesse" field is created
# by ParaView as an empty presentation: no any cells or points.
# TODO: check why presentation is empty.
field_names2 = ["vitesse"]
prs_list2 = [ [7] ]

Import_Med_Field(med_file, field_names2, 0, prs_list2)
