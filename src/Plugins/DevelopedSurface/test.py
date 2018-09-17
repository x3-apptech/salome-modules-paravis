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
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# create a new 'Wavelet'
wavelet1 = Wavelet()

# Properties modified on wavelet1
wavelet1.WholeExtent = [0, 10, 0, 10, 0, 10]

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [1168, 582]

# show data in view
wavelet1Display = Show(wavelet1, renderView1)

# trace defaults for the display properties.
wavelet1Display.Representation = 'Outline'
wavelet1Display.ColorArrayName = ['POINTS', '']
wavelet1Display.OSPRayScaleArray = 'RTData'
wavelet1Display.OSPRayScaleFunction = 'PiecewiseFunction'
wavelet1Display.SelectOrientationVectors = 'None'
wavelet1Display.SelectScaleArray = 'RTData'
wavelet1Display.GlyphType = 'Arrow'
wavelet1Display.GlyphTableIndexArray = 'RTData'
wavelet1Display.DataAxesGrid = 'GridAxesRepresentation'
wavelet1Display.PolarAxes = 'PolarAxesRepresentation'
wavelet1Display.ScalarOpacityUnitDistance = 1.7320508075688779
wavelet1Display.Slice = 5
wavelet1Display.GaussianRadius = 0.5
wavelet1Display.SetScaleArray = ['POINTS', 'RTData']
wavelet1Display.ScaleTransferFunction = 'PiecewiseFunction'
wavelet1Display.OpacityArray = ['POINTS', 'RTData']
wavelet1Display.OpacityTransferFunction = 'PiecewiseFunction'
wavelet1Display.InputVectors = [None, '']
wavelet1Display.SelectInputVectors = [None, '']
wavelet1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
wavelet1Display.ScaleTransferFunction.Points = [-16.577068328857422, 0.0, 0.5, 0.0, 260.0, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
wavelet1Display.OpacityTransferFunction.Points = [-16.577068328857422, 0.0, 0.5, 0.0, 260.0, 1.0, 0.5, 0.0]

# reset view to fit data
renderView1.ResetCamera()

# update the view to ensure updated data information
renderView1.Update()

# create a new 'Slice'
slice1 = Slice(Input=wavelet1)
slice1.SliceType = 'Plane'
slice1.SliceOffsetValues = [0.0]

# init the 'Plane' selected for 'SliceType'
slice1.SliceType.Origin = [5.0, 5.0, 5.0]

# toggle 3D widget visibility (only when running from the GUI)
Show3DWidgets(proxy=slice1.SliceType)

# Properties modified on slice1
slice1.SliceType = 'Cylinder'

# show data in view
slice1Display = Show(slice1, renderView1)

# get color transfer function/color map for 'RTData'
rTDataLUT = GetColorTransferFunction('RTData')

# trace defaults for the display properties.
slice1Display.Representation = 'Surface'
slice1Display.ColorArrayName = ['POINTS', 'RTData']
slice1Display.LookupTable = rTDataLUT
slice1Display.OSPRayScaleArray = 'RTData'
slice1Display.OSPRayScaleFunction = 'PiecewiseFunction'
slice1Display.SelectOrientationVectors = 'None'
slice1Display.SelectScaleArray = 'RTData'
slice1Display.GlyphType = 'Arrow'
slice1Display.GlyphTableIndexArray = 'RTData'
slice1Display.DataAxesGrid = 'GridAxesRepresentation'
slice1Display.PolarAxes = 'PolarAxesRepresentation'
slice1Display.GaussianRadius = 0.5
slice1Display.SetScaleArray = ['POINTS', 'RTData']
slice1Display.ScaleTransferFunction = 'PiecewiseFunction'
slice1Display.OpacityArray = ['POINTS', 'RTData']
slice1Display.OpacityTransferFunction = 'PiecewiseFunction'
slice1Display.InputVectors = [None, '']
slice1Display.SelectInputVectors = [None, '']
slice1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
slice1Display.ScaleTransferFunction.Points = [-14.761041641235352, 0.0, 0.5, 0.0, 232.8310546875, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
slice1Display.OpacityTransferFunction.Points = [-14.761041641235352, 0.0, 0.5, 0.0, 232.8310546875, 1.0, 0.5, 0.0]

# show color bar/color legend
slice1Display.SetScalarBarVisibility(renderView1, True)

# update the view to ensure updated data information
renderView1.Update()

# create a new 'Developed Surface'
developedSurface1 = DevelopedSurface(Input=slice1)

#### saving camera placements for all active views

# current camera placement for renderView1
renderView1.CameraPosition = [5.0, 5.0, 38.46065214951232]
renderView1.CameraFocalPoint = [5.0, 5.0, 5.0]
renderView1.CameraParallelScale = 8.660254037844387

#### uncomment the following to render all views
# RenderAllViews()
# alternatively, if you want to write images, you can use SaveScreenshot(...).
