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

# This case corresponds to: /visu/3D_viewer/A5 case
# Create 3D Viewer and test set view properties for Deformed Shape presentation
# Author:       POLYANSKIKH VERA
from paravistest import *
from presentations import *
from pvsimple import *
import sys
import paravis
import time

# Directory for saving snapshots
picturedir = get_picture_dir(sys.argv[1], "3D_viewer/A5")

# Add path separator to the end of picture path if necessery
if not picturedir.endswith(os.sep):
    picturedir += os.sep

#import file
myParavis = paravis.myParavis

# Get view
my_view = GetRenderView()
reset_view(my_view)
Render(my_view)

file_name = datadir + "cube_hexa8_quad4.med"
print " --------------------------------- "
print "file ", file_name
print " --------------------------------- "

myParavis.ImportFile(file_name)
proxy = GetActiveSource()
if proxy is None:
    raise RuntimeError("Error: can't import file.")
else:
    print "OK"

represents = [RepresentationType.POINTS, RepresentationType.WIREFRAME,\
RepresentationType.SURFACE, RepresentationType.VOLUME]
shrinks = [0, 1]
shadings = [0, 1]
opacities = [1.0, 0.5, 0.0]
linewidths = [1.0, 3.0, 10.0]
compare_prec = 0.00001
shrink_filter = None
shrinked_ds = None

field_name = 'fieldcelldouble'

print "\nCreating deformed shape.......",
ds = DeformedShapeOnField(proxy, EntityType.CELL,
field_name, 1, scale_factor=0.5, is_colored=True)
if ds is None:
    raise RuntimeError("Error!!! Presentation wasn't created...")

display_only(ds, my_view)
reset_view(my_view)
Render(my_view)

print "\nChange Presentation Parameters..."


for reprCode in represents:
    repr = RepresentationType.get_name(reprCode)
    call_and_check(ds, "Representation", repr, 1)
    for shr in shrinks:
        if shr > 0 and reprCode != RepresentationType.POINTS:
            if shrinked_ds is None:
                ds.Visibility = 1
                shrink_filter = Shrink(ds.Input)
                shrink_filter.ShrinkFactor = 0.8
                shrink_filter.UpdatePipeline()
                shrinked_ds = GetRepresentation(shrink_filter)
                shrinked_ds.ColorAttributeType = ds.ColorAttributeType
                shrinked_ds.ColorArrayName = ds.ColorArrayName
                shrinked_ds.LookupTable = ds.LookupTable
            ds.Visibility = 0
            shrinked_ds.Representation = ds.Representation
            shape_to_show = shrinked_ds
        else:
            if shrinked_ds is not None:
                shrinked_ds.Visibility = 0
            shape_to_show = ds
        shape_to_show.Visibility = 1
        Render(my_view)

        for sha in shadings:
            setShaded(my_view, sha)
            call_and_check(shape_to_show, "Shading", sha, 1)
            Render(my_view)

            for opa in opacities:
                call_and_check(shape_to_show, "Opacity", opa, 1, compare_prec)

                for lwi in linewidths:
                    call_and_check(shape_to_show, "LineWidth", lwi, 1,
                    compare_prec)

                    time.sleep(1)
                    # save picture in file
                    # Construct image file name
                    pic_name = picturedir + "params_" + repr.replace(' ', '_') + "_" + str(shr) + "_" +  \
                        str(sha) + "_" + str(opa) + "_" + str(lwi) + "." + pictureext
                    # Show and record the presentation
                    WriteImage(pic_name, view=my_view, Magnification=1)
                    pass
                pass
            pass
        pass
    pass