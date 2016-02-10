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

# This case corresponds to: /visu/ScalarMap/G2 case
# Create Scalar Map for field of the the given MED file for 10 timestamps%

import sys
import os
from paravistest import datadir, pictureext, get_picture_dir, get_png_picture_resolution
from pvsimple import GetActiveSource, GetRenderView, Render, OpenDataFile
from presentations import ScalarMapOnField, hide_all, EntityType, PrsTypeEnum,reset_view,process_prs_for_test

picturedir = get_picture_dir("ScalarMap/G2")

theFileName = datadir +  "Bug829_resu_mode.med"
print " --------------------------------- "
print "file ", theFileName
print " --------------------------------- "

"""Build presentations of the given types for all fields of the given file."""
#print "Import %s..." % theFileName.split('/')[-1],
result = OpenDataFile(theFileName)
proxy = GetActiveSource()
if proxy is None:
        raise RuntimeError, "Error: can't import file."
else: print "OK"
# Get view
aView = GetRenderView()


import time

aFieldEntity = EntityType.NODE
aFieldName = "MODES___DEPL____________________"
#create list to store picture files sizes
sizesw=[]
sizesh=[]
#create Scalar Map presentations for 10 timestamps
for i in range(1,11):
    hide_all(aView, True)
    aPrs = ScalarMapOnField(proxy, aFieldEntity,aFieldName , i)
    if aPrs is None:
        raise RuntimeError, "Presentation is None!!!"
    #display only current scalar map
    aPrs.Visibility=1
    reset_view(aView)
    Render(aView)

    # Add path separator to the end of picture path if necessery
    if not picturedir.endswith(os.sep):
            picturedir += os.sep
    prs_type = PrsTypeEnum.SCALARMAP

    # Get name of presentation type
    prs_name = PrsTypeEnum.get_name(prs_type)
    f_prs_type = prs_name.replace(' ', '').upper()
    # Construct image file name
    pic_name = picturedir + aFieldName + "_" + str(i) + "_" + f_prs_type + "." + pictureext

    # Show and record the presentation
    process_prs_for_test(aPrs, aView, pic_name)
    (w,h) = get_png_picture_resolution(pic_name)
    sizesw.append(w)
    sizesh.append(h)

# check sizes of pictures: width
if abs(max(sizesw)-min(sizesw)) > 0:
    print "<b>ERROR!!! Pictures have different width !!!</b>";
    for i in range(1,11):
        picture_name = "time_stamp_"+str(i)+"."+pictureext
        print "Picture: "+picture_name+"; width : "+str(sizesw[i-1])
    raise RuntimeError

# check sizes of pictures: height
if abs(max(sizesh)-min(sizesh)) > 0:
    print "<b>WARNING!!! Pictures have different height !!!</b>";
    for i in range(1,11):
        picture_name = "time_stamp_"+str(i)+"."+pictureext
        print "Picture: "+picture_name+"; height : "+str(sizesh[i-1])
    raise RuntimeError

