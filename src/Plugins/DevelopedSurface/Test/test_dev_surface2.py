# Copyright (C) 2017  EDF R&D
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

#### import the simple module from the paraview
from paraview.simple import *
from math import pi
TMPFileName="test2.med"

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# create a new 'Mandelbrot'
mandelbrot1 = Mandelbrot()

# Properties modified on mandelbrot1
mandelbrot1.WholeExtent = [0, 50, 0, 50, 0, 50]

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [1017, 317]

# show data in view
mandelbrot1Display = Show(mandelbrot1, renderView1)

# trace defaults for the display properties.
mandelbrot1Display.Representation = 'Outline'
mandelbrot1Display.ColorArrayName = ['POINTS', '']
mandelbrot1Display.OSPRayScaleArray = 'Iterations'
mandelbrot1Display.OSPRayScaleFunction = 'PiecewiseFunction'
mandelbrot1Display.SelectOrientationVectors = 'Iterations'
mandelbrot1Display.ScaleFactor = 0.25
mandelbrot1Display.SelectScaleArray = 'Iterations'
mandelbrot1Display.GlyphType = 'Arrow'
mandelbrot1Display.GlyphTableIndexArray = 'Iterations'
mandelbrot1Display.DataAxesGrid = 'GridAxesRepresentation'
mandelbrot1Display.PolarAxes = 'PolarAxesRepresentation'
mandelbrot1Display.ScalarOpacityUnitDistance = 0.08124038404635964
mandelbrot1Display.Slice = 25
mandelbrot1Display.GaussianRadius = 0.125
mandelbrot1Display.SetScaleArray = ['POINTS', 'Iterations']
mandelbrot1Display.ScaleTransferFunction = 'PiecewiseFunction'
mandelbrot1Display.OpacityArray = ['POINTS', 'Iterations']
mandelbrot1Display.OpacityTransferFunction = 'PiecewiseFunction'
mandelbrot1Display.InputVectors = [None, '']
mandelbrot1Display.SelectInputVectors = [None, '']
mandelbrot1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
mandelbrot1Display.ScaleTransferFunction.Points = [1.0, 0.0, 0.5, 0.0, 100.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
mandelbrot1Display.OpacityTransferFunction.Points = [1.0, 0.0, 0.5, 0.0, 100.0, 1.0, 0.5, 0.0]

# reset view to fit data
renderView1.ResetCamera()

# update the view to ensure updated data information
renderView1.Update()

# create a new 'Developed Surface'
developedSurface1 = DevelopedSurface(Input=mandelbrot1)
developedSurface1.SliceType = 'Cylinder'

# init the 'Cylinder' selected for 'SliceType'
developedSurface1.SliceType.Center = [-0.5, 0.0, 1.0]
developedSurface1.SliceType.Radius = 0.5 #1.25

# show data in view
developedSurface1Display = Show(developedSurface1, renderView1)

# get color transfer function/color map for 'Iterations'
iterationsLUT = GetColorTransferFunction('Iterations')

# trace defaults for the display properties.
developedSurface1Display.Representation = 'Surface'
developedSurface1Display.ColorArrayName = ['POINTS', 'Iterations']
developedSurface1Display.LookupTable = iterationsLUT
developedSurface1Display.OSPRayScaleArray = 'Iterations'
developedSurface1Display.OSPRayScaleFunction = 'PiecewiseFunction'
developedSurface1Display.SelectOrientationVectors = 'Iterations'
developedSurface1Display.ScaleFactor = 0.7853981633974483
developedSurface1Display.SelectScaleArray = 'Iterations'
developedSurface1Display.GlyphType = 'Arrow'
developedSurface1Display.GlyphTableIndexArray = 'Iterations'
developedSurface1Display.DataAxesGrid = 'GridAxesRepresentation'
developedSurface1Display.PolarAxes = 'PolarAxesRepresentation'
developedSurface1Display.GaussianRadius = 0.39269908169872414
developedSurface1Display.SetScaleArray = ['POINTS', 'Iterations']
developedSurface1Display.ScaleTransferFunction = 'PiecewiseFunction'
developedSurface1Display.OpacityArray = ['POINTS', 'Iterations']
developedSurface1Display.OpacityTransferFunction = 'PiecewiseFunction'
developedSurface1Display.InputVectors = [None, '']
developedSurface1Display.SelectInputVectors = [None, '']
developedSurface1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
developedSurface1Display.ScaleTransferFunction.Points = [1.0, 0.0, 0.5, 0.0, 100.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
developedSurface1Display.OpacityTransferFunction.Points = [1.0, 0.0, 0.5, 0.0, 100.0, 1.0, 0.5, 0.0]

# hide data in view
Hide(mandelbrot1, renderView1)

# show color bar/color legend
developedSurface1Display.SetScalarBarVisibility(renderView1, True)

# update the view to ensure updated data information
renderView1.Update()

# toggle 3D widget visibility (only when running from the GUI)
Hide3DWidgets(proxy=developedSurface1.SliceType)

#### saving camera placements for all active views

# current camera placement for renderView1
renderView1.CameraPosition = [4.090024784500779, -0.15919161102314858, 7.485304552729019]
renderView1.CameraFocalPoint = [4.090024784500779, -0.15919161102314858, 1.0]
renderView1.CameraParallelScale = 2.03100960115899

#### uncomment the following to render all views
# RenderAllViews()
# alternatively, if you want to write images, you can use SaveScreenshot(...).

mand=servermanager.Fetch(mandelbrot1)
axisId=1
high_out=mand.GetSpacing()[axisId]*(mand.GetExtent()[2*axisId+1]-mand.GetExtent()[2*axisId+0])

vtp=servermanager.Fetch(developedSurface1)
arr=vtp.GetPointData().GetArray(0)
assert(arr.GetName()=="Iterations")
a,b=arr.GetRange()
assert(a>=1 and a<=2)
assert(b==100.)
SaveData(TMPFileName, proxy=developedSurface1)
from MEDLoader import *

mm=MEDFileMesh.New(TMPFileName)
m0=mm[0]
area=m0.getMeasureField(True).getArray().accumulate()[0]

zeResu0=area/high_out/developedSurface1.SliceType.Radius
assert(abs(zeResu0-2*pi)<1e-5)

fs=MEDFileFields(TMPFileName)
f=fs["Iterations"][0].field(mm)
nodeIds=f.getArray().convertToDblArr().findIdsInRange(99.,101.)
cellIds=m0.getCellIdsLyingOnNodes(nodeIds,True)
zeResu1=m0[cellIds].getMeasureField(True).getArray().accumulate()[0]

assert(abs(zeResu1-1.1427)<1e-2)

