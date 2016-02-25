#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2016  CEA/DEN, EDF R&D
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
# Author : Anthony Geay (EDF R&D)

# non regression test that emulates https://ageay@git.salome-platform.org/gitpub/samples/datafiles.git Med/ResOK_0000.med
# This test point error during commit efd9331a9455785d0f04b75 in PARAVIS

from MEDLoader import *
fname="testMEDReader20.med"
png="testMEDReader20.png"
nb=10
arrX=DataArrayDouble(nb+1) ; arrX.iota()
arrY=DataArrayDouble([0.,1.])
m=MEDCouplingCMesh() ; m.setCoords(arrX,arrY) ; m=m.buildUnstructured(); m.setName("mesh") ; m.simplexize(0)
mm=MEDFileUMesh() ; mm[0]=m
m1=m.computeSkin() ; mm[-1]=m1
#
f0=DataArrayInt(m1.getNumberOfCells()) ; f0.iota() ; mm.setFamilyFieldArr(-1,f0)
f1=DataArrayInt(m1.getNumberOfNodes()) ; f1.iota() ; mm.setFamilyFieldArr(1,f1) # <- very important the bug can be shown here
#
nbCells=m1.getNumberOfCells() ; nbNodes=m.getNumberOfNodes()
mm.write(fname,2)
for i in range(5):
    f=MEDCouplingFieldDouble(ON_CELLS) ; f.setMesh(m)
    f.setName("Field")
    arr=DataArrayInt(2*nb) ; arr.iota(i) ; arr%=nb ; arr=arr.convertToDblArr()
    f.setArray(arr) ; f.setTime(float(i),i,0)
    WriteFieldUsingAlreadyWrittenMesh(fname,f)
    #
    f=MEDCouplingFieldDouble(ON_CELLS) ; f.setMesh(m1)
    f.setName("Field")
    arr=DataArrayInt(nbCells) ; arr.iota(i) ; arr%=nbCells ; arr=arr.convertToDblArr()
    f.setArray(arr) ; f.setTime(float(i),i,0)
    WriteFieldUsingAlreadyWrittenMesh(fname,f)
    #
    f=MEDCouplingFieldDouble(ON_NODES) ; f.setMesh(m)
    f.setName("FieldNode")
    arr=DataArrayDouble(nbNodes) ; arr[:]=float(i)
    f.setArray(arr) ; f.setTime(float(i),i,0)
    WriteFieldUsingAlreadyWrittenMesh(fname,f)
    pass
#####################
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# create a new 'MED Reader'
testMEDReader20med = MEDReader(FileName=fname)
testMEDReader20med.AllArrays = ['TS0/mesh/ComSup0/Field@@][@@P0']
testMEDReader20med.AllTimeSteps = ['0000', '0001', '0002', '0003', '0004']

# get animation scene
animationScene1 = GetAnimationScene()

# update animation scene based on data timesteps
animationScene1.UpdateAnimationUsingDataTimeSteps()

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [610, 477]

# show data in view
testMEDReader20medDisplay = Show(testMEDReader20med, renderView1)
# trace defaults for the display properties.
testMEDReader20medDisplay.ColorArrayName = [None, '']
testMEDReader20medDisplay.GlyphType = 'Arrow'
testMEDReader20medDisplay.ScalarOpacityUnitDistance = 4.664739046219201
testMEDReader20medDisplay.SelectUncertaintyArray = [None, '']
testMEDReader20medDisplay.UncertaintyTransferFunction = 'PiecewiseFunction'
testMEDReader20medDisplay.OpacityArray = [None, '']
testMEDReader20medDisplay.RadiusArray = [None, '']
testMEDReader20medDisplay.RadiusRange = [0.0, 10.0]
testMEDReader20medDisplay.ConstantRadius = 10.0
testMEDReader20medDisplay.PointSpriteDefaultsInitialized = 1
testMEDReader20medDisplay.SelectInputVectors = [None, '']
testMEDReader20medDisplay.WriteLog = ''

# reset view to fit data
renderView1.ResetCamera()

#changing interaction mode based on data extents
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [5.0, 0.5, 10000.0]
renderView1.CameraFocalPoint = [5.0, 0.5, 0.0]

# set scalar coloring
ColorBy(testMEDReader20medDisplay, ('FIELD', 'vtkBlockColors'))

# show color bar/color legend
testMEDReader20medDisplay.SetScalarBarVisibility(renderView1, True)

# get color transfer function/color map for 'vtkBlockColors'
vtkBlockColorsLUT = GetColorTransferFunction('vtkBlockColors')

# get opacity transfer function/opacity map for 'vtkBlockColors'
vtkBlockColorsPWF = GetOpacityTransferFunction('vtkBlockColors')

# set scalar coloring
ColorBy(testMEDReader20medDisplay, ('CELLS', 'Field'))

# rescale color and/or opacity maps used to include current data range
testMEDReader20medDisplay.RescaleTransferFunctionToDataRange(True)

# show color bar/color legend
testMEDReader20medDisplay.SetScalarBarVisibility(renderView1, True)

# get color transfer function/color map for 'Field'
fieldLUT = GetColorTransferFunction('Field')

# get opacity transfer function/opacity map for 'Field'
fieldPWF = GetOpacityTransferFunction('Field')

animationScene1.GoToNext() # <- very important to see the bug play with time steps...
animationScene1.GoToNext()
animationScene1.GoToNext()
animationScene1.GoToNext()
animationScene1.GoToPrevious()
animationScene1.GoToPrevious()

# current camera placement for renderView1
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [5.0, 0.5, 10000.0]
renderView1.CameraFocalPoint = [5.0, 0.5, 0.0]
renderView1.CameraParallelScale = 5.024937810560445

#

renderView1.ViewSize =[300,300]
Render()
#WriteImage(png)

#### saving camera placements for all active views

# current camera placement for renderView1
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [5.0, 0.5, 10000.0]
renderView1.CameraFocalPoint = [5.0, 0.5, 0.0]
renderView1.CameraParallelScale = 5.024937810560445
# compare with baseline image
import os
import sys
try:
  baselineIndex = sys.argv.index('-B')+1
  baselinePath = sys.argv[baselineIndex]
except:
  print "Could not get baseline directory. Test failed."
  exit(1)
baseline_file = os.path.join(baselinePath,png)
import vtk.test.Testing
vtk.test.Testing.VTK_TEMP_DIR = vtk.util.misc.vtkGetTempDir()
vtk.test.Testing.compareImage(GetActiveView().GetRenderWindow(), baseline_file, threshold=25)
vtk.test.Testing.interact()
