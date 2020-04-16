# Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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

# create a new 'MED Reader'
multiTSmed = MEDReader(FileName='multiTS.med')
multiTSmed.AllArrays = ['TS0/Mesh/ComSup0/Pressure@@][@@P0']
multiTSmed.AllTimeSteps = ['0000', '0001', '0002', '0003', '0004', '0005', '0006', '0007', '0008', '0009']

# get animation scene
animationScene1 = GetAnimationScene()

# update animation scene based on data timesteps
animationScene1.UpdateAnimationUsingDataTimeSteps()

# get active view
renderView1 = GetActiveViewOrCreate('RenderView')
# uncomment following to set a specific view size
# renderView1.ViewSize = [1499, 582]

# show data in view
multiTSmedDisplay = Show(multiTSmed, renderView1)

# trace defaults for the display properties.
multiTSmedDisplay.Representation = 'Surface'
multiTSmedDisplay.ColorArrayName = [None, '']
multiTSmedDisplay.OSPRayScaleArray = 'FamilyIdNode'
multiTSmedDisplay.OSPRayScaleFunction = 'PiecewiseFunction'
multiTSmedDisplay.SelectOrientationVectors = 'FamilyIdNode'
multiTSmedDisplay.ScaleFactor = 0.07399989366531372
multiTSmedDisplay.SelectScaleArray = 'FamilyIdNode'
multiTSmedDisplay.GlyphType = 'Arrow'
multiTSmedDisplay.GlyphTableIndexArray = 'FamilyIdNode'
multiTSmedDisplay.DataAxesGrid = 'GridAxesRepresentation'
multiTSmedDisplay.PolarAxes = 'PolarAxesRepresentation'
multiTSmedDisplay.ScalarOpacityUnitDistance = 0.017316274962626298
multiTSmedDisplay.GaussianRadius = 0.03699994683265686
multiTSmedDisplay.SetScaleArray = ['POINTS', 'FamilyIdNode']
multiTSmedDisplay.ScaleTransferFunction = 'PiecewiseFunction'
multiTSmedDisplay.OpacityArray = ['POINTS', 'FamilyIdNode']
multiTSmedDisplay.OpacityTransferFunction = 'PiecewiseFunction'
multiTSmedDisplay.InputVectors = [None, '']
multiTSmedDisplay.SelectInputVectors = [None, '']
multiTSmedDisplay.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
multiTSmedDisplay.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 1.1757813367477812e-38, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
multiTSmedDisplay.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 1.1757813367477812e-38, 1.0, 0.5, 0.0]

# reset view to fit data
renderView1.ResetCamera()

# update the view to ensure updated data information
renderView1.Update()

# create a new 'Developed Surface'
developedSurface1 = DevelopedSurface(Input=multiTSmed)
developedSurface1.SliceType = 'Cylinder'

# init the 'Cylinder' selected for 'SliceType'
developedSurface1.SliceType.Center = [0.0, 0.0, 0.05000000074505806]
developedSurface1.SliceType.Radius = 0.3699994683265686

# Properties modified on developedSurface1.SliceType
developedSurface1.SliceType.Center = [0.0, 0.0, 0.05]
developedSurface1.SliceType.Axis = [0.0, 0.0, 1.0]
developedSurface1.SliceType.Radius = 0.07

# Properties modified on developedSurface1.SliceType
developedSurface1.SliceType.Center = [0.0, 0.0, 0.05]
developedSurface1.SliceType.Axis = [0.0, 0.0, 1.0]
developedSurface1.SliceType.Radius = 0.07

# show data in view
developedSurface1Display = Show(developedSurface1, renderView1)

# trace defaults for the display properties.
developedSurface1Display.Representation = 'Surface'
developedSurface1Display.ColorArrayName = [None, '']
developedSurface1Display.OSPRayScaleArray = 'FamilyIdNode'
developedSurface1Display.OSPRayScaleFunction = 'PiecewiseFunction'
developedSurface1Display.SelectOrientationVectors = 'FamilyIdNode'
developedSurface1Display.ScaleFactor = 0.043982297150257116
developedSurface1Display.SelectScaleArray = 'FamilyIdNode'
developedSurface1Display.GlyphType = 'Arrow'
developedSurface1Display.GlyphTableIndexArray = 'FamilyIdNode'
developedSurface1Display.DataAxesGrid = 'GridAxesRepresentation'
developedSurface1Display.PolarAxes = 'PolarAxesRepresentation'
developedSurface1Display.GaussianRadius = 0.021991148575128558
developedSurface1Display.SetScaleArray = ['POINTS', 'FamilyIdNode']
developedSurface1Display.ScaleTransferFunction = 'PiecewiseFunction'
developedSurface1Display.OpacityArray = ['POINTS', 'FamilyIdNode']
developedSurface1Display.OpacityTransferFunction = 'PiecewiseFunction'
developedSurface1Display.InputVectors = [None, '']
developedSurface1Display.SelectInputVectors = [None, '']
developedSurface1Display.WriteLog = ''

# init the 'PiecewiseFunction' selected for 'ScaleTransferFunction'
developedSurface1Display.ScaleTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 1.1757813367477812e-38, 1.0, 0.5, 0.0]

# init the 'PiecewiseFunction' selected for 'OpacityTransferFunction'
developedSurface1Display.OpacityTransferFunction.Points = [0.0, 0.0, 0.5, 0.0, 1.1757813367477812e-38, 1.0, 0.5, 0.0]

# hide data in view
Hide(multiTSmed, renderView1)

# update the view to ensure updated data information
renderView1.Update()

#change interaction mode for render view
renderView1.InteractionMode = '2D'

# toggle 3D widget visibility (only when running from the GUI)
Hide3DWidgets(proxy=developedSurface1.SliceType)

# set scalar coloring
ColorBy(developedSurface1Display, ('CELLS', 'Pressure'))

# rescale color and/or opacity maps used to include current data range
developedSurface1Display.RescaleTransferFunctionToDataRange(True, False)

# show color bar/color legend
developedSurface1Display.SetScalarBarVisibility(renderView1, True)

# get color transfer function/color map for 'Pressure'
pressureLUT = GetColorTransferFunction('Pressure')

#### saving camera placements for all active views

# current camera placement for renderView1
renderView1.InteractionMode = '2D'
renderView1.CameraPosition = [0.18935662797765695, 0.01726656182167085, 2.08092363470839]
renderView1.CameraFocalPoint = [0.18935662797765695, 0.01726656182167085, 0.05000000074505806]
renderView1.CameraParallelScale = 0.16748564967020724

#### uncomment the following to render all views
# RenderAllViews()
# alternatively, if you want to write images, you can use SaveScreenshot(...).
Render()
