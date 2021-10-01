# This Python script creates three custom views for the three display types of
# the filter 'Rosettes de contrainte'.

from paraview.simple import *

# Get active view
view1 = FindViewOrCreate('RenderView1', viewtype='RenderView')

# Get the layout
layout = GetLayout()

# Create second view
layout.SplitVertical(0,0.5)
view2 = CreateView('RenderView')
AssignViewToLayout(view=view2, layout=layout)

# Create third view
layout.SplitHorizontal(2,0.5)
view3 = CreateView('RenderView')
AssignViewToLayout(view=view3, layout=layout)

# Link cameras of new views to the initial one
AddCameraLink(view2, view1, 'CameraLink2')
AddCameraLink(view3, view1, 'CameraLink3')

# Retrieve active source
source = GetActiveSource()

#================
#   First view
#================

# Apply 'Rosettes de contrainte' filter
rosette1 = Rosettesdecontrainte(Input=source)

# Show filter output and hide initial source
Show(rosette1, view1)
Hide(source, view1)

# Set NaN color to black
rosette1LUT = GetColorTransferFunction('CompressionOrTraction')
rosette1LUT.NanColor = [0.0, 0.0, 0.0]

#================
#  Second view
#================

# Apply 'Rosettes de contrainte' filter
rosette2 = Rosettesdecontrainte(Input=source)

# Change type of display
rosette2.TypeOfDisplay = 'T1 only'

# Show filter output
rosette2Display = Show(rosette2, view2)

# Change representation type
rosette2Display.SetRepresentationType('Surface With Edges')

# Set edge color to black
rosette2Display.EdgeColor = [0.0, 0.0, 0.0]

# Show scalar color bar
rosette2Display.SetScalarBarVisibility(view2, True)

# Retrieve lookup table
rosette2LUT = GetColorTransferFunction('Contraintespecifique1')

# Change number of discrete colors
rosette2LUT.NumberOfTableValues = 10

# Change color map preset
rosette2LUT.ApplyPreset('Rainbow Uniform', True)

# Retrieve scalar color bar
rosette2ColorBar = GetScalarBar(rosette2LUT, view2)

# Change label format
rosette2ColorBar.AutomaticLabelFormat = 0
rosette2ColorBar.LabelFormat = '%-#6.1e'

# Remove unwanted tick marks and labels
rosette2ColorBar.DrawTickMarks = 0
rosette2ColorBar.DrawTickLabels = 0
rosette2ColorBar.AddRangeLabels = 0
rosette2ColorBar.AutomaticAnnotations = 1

# Change color bar size
rosette2ColorBar.ScalarBarLength = 0.6

#================
#   Third view
#================

# Apply 'Rosettes de contrainte' filter
rosette3 = Rosettesdecontrainte(Input=source)

# Change type of display
rosette3.TypeOfDisplay = 'T2 only'

# Show filter output
rosette3Display = Show(rosette3, view3)

# Change representation type
rosette3Display.SetRepresentationType('Surface With Edges')

# Set edge color to black
rosette3Display.EdgeColor = [0.0, 0.0, 0.0]

# Show scalar color bar
rosette3Display.SetScalarBarVisibility(view3, True)

# Retrieve lookup table
rosette3LUT = GetColorTransferFunction('Contraintespecifique3')

# Change number of discrete colors
rosette3LUT.NumberOfTableValues = 10

# Change color map preset
rosette3LUT.ApplyPreset('Rainbow Uniform', True)

# Retrieve scalar color bar
ros3ColorBar = GetScalarBar(rosette3LUT, view3)

# Change label format
ros3ColorBar.AutomaticLabelFormat = 0
ros3ColorBar.LabelFormat = '%-#6.1e'

# Remove unwanted tick marks and labels
ros3ColorBar.DrawTickMarks = 0
ros3ColorBar.DrawTickLabels = 0
ros3ColorBar.AddRangeLabels = 0
ros3ColorBar.AutomaticAnnotations = 1

# Change color bar size
ros3ColorBar.ScalarBarLength = 0.6

# Reset camera
view1.ResetCamera()

