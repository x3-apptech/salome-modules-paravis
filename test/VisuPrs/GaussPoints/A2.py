# Copyright (C) 2010-2013  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

# This case corresponds to: /visu/GaussPoints/A2 case
# Create Gauss Points on the field of the MED file

import os
import sys

from paravistest import datadir, pictureext, get_picture_dir
from presentations import GaussPointsOnField, EntityType, get_time, process_prs_for_test
import paravis
import pvsimple


# Directory for saving snapshots
picturedir = get_picture_dir(sys.argv[1], "GaussPoints/A2")
if not picturedir.endswith(os.sep):
    picturedir += os.sep
    
# MED file
file_name = datadir + "Fields_group3D.med"
field_name = "scalar_field"
timestamp_nb = 1

paravis.myParavis.ImportFile(file_name)
med_reader = pvsimple.GetActiveSource()
if med_reader is None:
    raise RuntimeError("File wasn't imported!!!")

# Create Gauss Points presentation
prs = GaussPointsOnField(med_reader, EntityType.CELL, field_name, timestamp_nb)
if prs is None:
    raise RuntimeError, "Created presentation is None!!!"

# Display presentation and get snapshot
view = pvsimple.GetRenderView()
time = get_time(med_reader, timestamp_nb)

pic_name = picturedir + field_name + "_" + str(time) + "_GAUSSPOINTS." + pictureext
process_prs_for_test(prs, view, pic_name)

