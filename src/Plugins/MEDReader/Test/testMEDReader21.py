#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2015-2016  CEA/DEN, EDF R&D
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

from paraview.simple import *

from MEDLoader import *

""" This is a non regression EDF12599"""
fname="testMEDReader21.med"
imgName="testMEDReader21.png"
fieldName="MyField"
meshName="mesh"
m=MEDFileUMesh()
m0=MEDCouplingUMesh(meshName,1)
m0.setCoords(DataArrayDouble([(0.,0.,0),(1.,0.,0.),(2.,0.,0.)]))
m0.allocateCells()
m0.insertNextCell(NORM_SEG2,[0,1])
m0.insertNextCell(NORM_SEG2,[1,2])
m[0]=m0
#
f=MEDCouplingFieldDouble(ON_GAUSS_NE) ; f.setName(fieldName)
f.setMesh(m0) ; f.setArray(DataArrayDouble([1.,7.,3.,2.]))
WriteField(fname,f,True)
########
testTotomed = MEDReader(FileName=fname)
testTotomed.AllArrays = ['TS0/%s/ComSup0/%s@@][@@GSSNE'%(meshName,fieldName)]
testTotomed.AllTimeSteps = ['0000']
# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [739, 503]

# show data in view
testTotomedDisplay = Show(testTotomed, renderView1)
# trace defaults for the display properties.
testTotomedDisplay.ColorArrayName = [None, '']
testTotomedDisplay.GlyphType = 'Arrow'
testTotomedDisplay.ScalarOpacityUnitDistance = 1.5874010519681994
testTotomedDisplay.SelectUncertaintyArray = [None, '']
testTotomedDisplay.UncertaintyTransferFunction = 'PiecewiseFunction'
testTotomedDisplay.OpacityArray = [None, '']
testTotomedDisplay.RadiusArray = [None, '']
testTotomedDisplay.RadiusRange = [0.0, 2.0]
testTotomedDisplay.ConstantRadius = 2.0
testTotomedDisplay.PointSpriteDefaultsInitialized = 1
testTotomedDisplay.SelectInputVectors = [None, '']
testTotomedDisplay.WriteLog = ''

# reset view to fit data
renderView1.ResetCamera()

#changing interaction mode based on data extents
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [1.0, 10000.0, 10000.0]
renderView1.CameraFocalPoint = [1.0, 0.0, 0.0]
renderView1.CameraViewUp = [1.0, 1.0, 0.0]

# set scalar coloring
ColorBy(testTotomedDisplay, ('FIELD', 'vtkBlockColors'))

# show color bar/color legend
testTotomedDisplay.SetScalarBarVisibility(renderView1, True)

# get color transfer function/color map for 'vtkBlockColors'
vtkBlockColorsLUT = GetColorTransferFunction('vtkBlockColors')

# get opacity transfer function/opacity map for 'vtkBlockColors'
vtkBlockColorsPWF = GetOpacityTransferFunction('vtkBlockColors')

# create a new 'ELNO Mesh'
eLNOMesh1 = ELNOMesh(Input=testTotomed)

# Properties modified on eLNOMesh1
eLNOMesh1.ShrinkFactor = 0.5 # <- test is here !!!!!!!!

# show data in view
eLNOMesh1Display = Show(eLNOMesh1, renderView1)
# trace defaults for the display properties.
eLNOMesh1Display.ColorArrayName = [None, '']
eLNOMesh1Display.GlyphType = 'Arrow'
eLNOMesh1Display.ScalarOpacityUnitDistance = 1.1905507889761495
eLNOMesh1Display.SelectUncertaintyArray = ['POINTS', 'MyField']
eLNOMesh1Display.UncertaintyTransferFunction = 'PiecewiseFunction'
eLNOMesh1Display.OpacityArray = [None, '']
eLNOMesh1Display.RadiusArray = [None, '']
eLNOMesh1Display.RadiusRange = [0.25, 1.75]
eLNOMesh1Display.ConstantRadius = 1.75
eLNOMesh1Display.PointSpriteDefaultsInitialized = 1
eLNOMesh1Display.SelectInputVectors = [None, '']
eLNOMesh1Display.WriteLog = ''

# hide data in view
Hide(testTotomed, renderView1)

# set scalar coloring
ColorBy(eLNOMesh1Display, ('FIELD', 'vtkBlockColors'))

# show color bar/color legend
eLNOMesh1Display.SetScalarBarVisibility(renderView1, False)

# set scalar coloring
ColorBy(eLNOMesh1Display, ('POINTS', 'MyField'))

# rescale color and/or opacity maps used to include current data range
eLNOMesh1Display.RescaleTransferFunctionToDataRange(True)

# show color bar/color legend
eLNOMesh1Display.SetScalarBarVisibility(renderView1, False)

# get color transfer function/color map for 'MyField'
myFieldLUT = GetColorTransferFunction('MyField')

# get opacity transfer function/opacity map for 'MyField'
myFieldPWF = GetOpacityTransferFunction('MyField')

# hide color bar/color legend
eLNOMesh1Display.SetScalarBarVisibility(renderView1, False)

# create a new 'Glyph'
glyph1 = Glyph(Input=eLNOMesh1,
    GlyphType='Arrow')
glyph1.Scalars = ['POINTS', 'MyField']
glyph1.Vectors = ['POINTS', 'None']
glyph1.ScaleFactor = 0.15000000000000002
glyph1.GlyphTransform = 'Transform2'

# Properties modified on glyph1
glyph1.GlyphType = 'Sphere'
glyph1.ScaleFactor = 0.15

# show data in view
glyph1Display = Show(glyph1, renderView1)
# trace defaults for the display properties.
glyph1Display.ColorArrayName = ['POINTS', 'MyField']
glyph1Display.LookupTable = myFieldLUT
glyph1Display.GlyphType = 'Arrow'
glyph1Display.SelectUncertaintyArray = ['POINTS', 'MyField']
glyph1Display.UncertaintyTransferFunction = 'PiecewiseFunction'
glyph1Display.OpacityArray = [None, '']
glyph1Display.RadiusArray = [None, '']
glyph1Display.RadiusRange = [0.17688040435314178, 1.8231196403503418]
glyph1Display.ConstantRadius = 1.8231196403503418
glyph1Display.PointSpriteDefaultsInitialized = 1
glyph1Display.SelectInputVectors = ['POINTS', 'Normals']
glyph1Display.WriteLog = ''

# show color bar/color legend
glyph1Display.SetScalarBarVisibility(renderView1, True)

#### saving camera placements for all active views

# current camera placement for renderView1
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [0.9999999999999908, 9999.999999999995, 9999.999999999993]
renderView1.CameraFocalPoint = [1.0, 0.0, 0.0]
renderView1.CameraViewUp = [0.6331899945158901, 0.547298104713038, -0.5472981047130381]
renderView1.CameraParallelScale = 0.6930835077290218
renderView1.ViewSize = [739,503]

import os
import sys
try:
  baselineIndex = sys.argv.index('-B')+1
  baselinePath = sys.argv[baselineIndex]
except:
  print "Could not get baseline directory. Test failed."
  exit(1)
baseline_file = os.path.join(baselinePath, imgName)
import vtk.test.Testing
vtk.test.Testing.VTK_TEMP_DIR = vtk.util.misc.vtkGetTempDir()
vtk.test.Testing.compareImage(GetActiveView().GetRenderWindow(), baseline_file, threshold=25)
vtk.test.Testing.interact()

