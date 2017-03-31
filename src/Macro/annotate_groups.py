# Copyright (C) 2014-2016  CEA/DEN, EDF R&D
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
# Author : Maxim Glibin

from paraview.simple import *
from paraview.servermanager import *
from vtk import *
paraview.simple._DisableFirstRenderCameraReset()

import os
import sys
import colorsys

# Separator of family name
ZE_SEP = "@@][@@"

# Available families
FAMILY_CELL = "FamilyIdCell"
FAMILY_NODE = "FamilyIdNode"

## Extract information about groups, families and IDs of these families from active source proxy.
#  @param smProxy The server manager proxy.
#  @param mapFamilies The returned map of families and its IDs.
#  @param mapGroups The returned map of groups and its families.
#  @param mapRelations The returned map of relations between families and groups.
#  @return True if all data are generated without errors, False is otherwise.
def ExctractSILInformation(smProxy, mapFamilies, mapGroups, mapRelations):
  # Check access to arguments
  if not isinstance(mapFamilies, dict) or not isinstance(mapGroups, dict) or not isinstance(mapRelations, dict):
    raise TypeError("Returned arguments have inappropriate type.")

  # Reset returned values
  mapFamilies.clear()
  mapGroups.clear()
  mapRelations.clear()

  # Gather information about server manager proxy.
  # Fills up 'silInfo' information with details about VTK object.
  silInfo = vtkPVSILInformation()
  smproxy.GatherInformation(silInfo)

  # Get SIL graph (an editable directed graph) as vtkMutableDirectedGraph
  graph = silInfo.GetSIL()
  if graph == None:
    raise RuntimeError("No SIL graph found.")

  # Get vertex data as vtkDataSetAttrbutes
  vertexData = graph.GetVertexData()
  if vertexData.GetNumberOfArrays() < 1:
    raise RuntimeError("No data arrays found.")

  # 'Names' array has 0 index
  indexNames = 0
  stringArray = graph.GetVertexData().GetAbstractArray(vertexData.GetArrayName(indexNames))
  if stringArray == None:
    raise RuntimeError("Abstract array with '%s' name is not found." % vertexData.GetArrayName(indexNames))

  # Search index for families and groups
  idx = None
  isFound = False
  for index in range(stringArray.GetNumberOfValues()):
    if stringArray.GetValue(index) == 'MeshesFamsGrps':
      isFound = True
      idx = index
      break

  if not isFound or not idx:
    raise RuntimeError("No 'MeshesFamsGrps' value in 'Names' array.")

  # Initialize the adjacent vertex iterator to iterate over all outgoing vertices from vertex 'idx',
  # i.e. the vertices which may be reached by traversing an out edge of the 'idx' vertex.
  adjacentIterator = vtkAdjacentVertexIterator()
  adjacentIterator.Initialize(graph, idx)
  if adjacentIterator.GetVertex() != idx:
    raise RuntimeError("Initialize the adjacent vertex iterator failed (main '%s')." % idx)
  
  while (adjacentIterator.HasNext()):
    vIdMesh = adjacentIterator.Next()
    meshName = stringArray.GetValue(vIdMesh)

    adjacentIt_Mesh = vtkAdjacentVertexIterator()
    graph.GetAdjacentVertices(vIdMesh, adjacentIt_Mesh);
    if adjacentIt_Mesh.GetVertex() != vIdMesh:
      raise RuntimeError("Initialize the adjacent vertex iterator failed (mesh '%s')." % vIdMesh)

    # Next edge in the graph associated with groups and its families
    vId_Groups = adjacentIt_Mesh.Next() # 'zeGrps'

    # Next edge in the graph associated with families and its IDs
    vId_Families = adjacentIt_Mesh.Next() # 'zeFamIds'

    adjacentIt_Families = vtkAdjacentVertexIterator()
    graph.GetAdjacentVertices(vId_Families, adjacentIt_Families);
    if adjacentIt_Families.GetVertex() != vId_Families:
      raise RuntimeError("Initialize the adjacent vertex iterator failed (families '%s')." % vId_Families)

    # Iterate over all families
    while adjacentIt_Families.HasNext():
      vIdFamily = adjacentIt_Families.Next()
      rawFamilyName = stringArray.GetValue(vIdFamily)

      # Get family name and its ID from raw string
      splitFamily = rawFamilyName.split(ZE_SEP)
      if len(splitFamily) != 2:
        raise RuntimeError("No valid family name [%s]" % rawFamilyName)

      familyName = splitFamily[0]
      familyID = splitFamily[1]
      if familyID.lstrip("+-").isdigit() == False:
        raise RuntimeError("Family [%s] ID is not digit: %s" % (familyName, familyID))

      # Save families and IDs in map
      mapFamilies[familyName] = int(familyID)

    adjacentIt_Groups = vtkAdjacentVertexIterator()
    graph.GetAdjacentVertices(vId_Groups, adjacentIt_Groups);
    if adjacentIt_Groups.GetVertex() != vId_Groups:
      raise RuntimeError("Initialize the adjacent vertex iterator failed (groups '%s')." % vId_Groups)

    # Iterate over all groups
    while adjacentIt_Groups.HasNext():
      vIdGroup = adjacentIt_Groups.Next()
      groupName = stringArray.GetValue(vIdGroup)

      adjacentIt_FamiliesOnGroups = vtkAdjacentVertexIterator()
      graph.GetAdjacentVertices(vIdGroup, adjacentIt_FamiliesOnGroups);
      if adjacentIt_FamiliesOnGroups.GetVertex() != vIdGroup:
        raise RuntimeError("Initialize the adjacent vertex iterator failed (group '%s')." % vIdGroup)

      familiesOnGroups = []
      # Iterate over all families on group
      while adjacentIt_FamiliesOnGroups.HasNext():
        vIdFamilyOnGroup = adjacentIt_FamiliesOnGroups.Next()
        familyNameOnGroup = stringArray.GetValue(vIdFamilyOnGroup)
        familiesOnGroups.append(familyNameOnGroup)

      # Save groups and its families in map
      mapGroups[groupName] = familiesOnGroups

  # Establish the relations between families and groups
  for family_name, family_id in mapFamilies.iteritems():
    groupNames = []
    for group_name, family_name_on_group in mapGroups.iteritems():
      if family_name in family_name_on_group:
        groupNames.append(group_name)
    if len(groupNames) > 0:
      mapRelations[family_name] = groupNames

  return True


# Get active source
source = GetActiveSource()
if source == None:
  raise RuntimeError("No active source.")

# Get server manager proxy
smproxy = source.SMProxy

# Check MEDReader class
if smproxy.GetVTKClassName() != 'vtkMEDReader':
  raise RuntimeError("VTK reader type is not supported (%s)." % smproxy.GetVTKClassName())

# Get render view in use or create new RenderView
renderView = GetActiveViewOrCreate('RenderView')
if renderView == None:
  raise RuntimeError("No render view found.")

# Get representation of active source and view
representation = GetDisplayProperties(source, renderView)
if not representation:
  raise RuntimeError("No representation found.")

# Visibility of source object
representation.Visibility = 1

# Get active or set default colour array name property
associationType = None
associationColorArray = representation.ColorArrayName.GetAssociation()
nameColorArray = representation.ColorArrayName.GetArrayName()
if not nameColorArray:
  # Reset colour array name property
  representation.ColorArrayName.SetElement(3, None)
  representation.ColorArrayName.SetElement(4, None)

  nameColorArray = FAMILY_CELL
  associationType = GetAssociationFromString('CELLS')
else:
  if associationColorArray != None:
    associationType = GetAssociationFromString(associationColorArray)

if nameColorArray not in [FAMILY_CELL, FAMILY_NODE]:
  raise RuntimeError("Selected data array [%s] is not valid." % nameColorArray)

# Get data information
data = Fetch(source)
dataArrayValues = []
scalarBarlabel = None
activeRepresentation = 'Surface'
if data.GetDataObjectType() == vtkDataObjectTypes.GetTypeIdFromClassName("vtkMultiBlockDataSet"):
  if data.GetNumberOfBlocks() < 1:
    raise RuntimeError("No blocks.")

  blockID = 0
  dataObject = data.GetBlock(blockID)
  if not dataObject.IsA("vtkDataSet"):
    raise RuntimeError("Data object has inappropriate type (%s)" % dataObject.GetDataObjectType())

  if associationType == GetAssociationFromString("CELLS"):
    cellData = dataObject.GetCellData()
    if not cellData.HasArray(FAMILY_CELL):
      raise RuntimeError("Cell data array has no '%s'." % FAMILY_CELL)

    scalarBarlabel = "CELLS"
    activeRepresentation = 'Surface'

    dataArray = cellData.GetArray(FAMILY_CELL)
    for tupleId in range(dataArray.GetNumberOfTuples()):
      dataArrayValues.append(dataArray.GetValue(tupleId))

  elif associationType == GetAssociationFromString("POINTS"):
    pointData = dataObject.GetPointData()
    if not pointData.HasArray(FAMILY_NODE):
      raise RuntimeError("Point data array has no '%s'." % FAMILY_NODE)

    scalarBarlabel = "POINTS"
    
    # Check that PointSprite plugin is available
    if hasattr(representation, "MaxPixelSize"):
        activeRepresentation = 'Point Sprite'
        if representation.MaxPixelSize == 64:
            representation.MaxPixelSize = 8
    else:
        activeRepresentation = 'Points'

    dataArray = pointData.GetArray(FAMILY_NODE)
    for tupleId in range(dataArray.GetNumberOfTuples()):
      dataArrayValues.append(dataArray.GetValue(tupleId))

  else:
    raise RuntimeError("Not implemented yet (%s)." % GetAssociationAsString(associationType))

mapFamilies = dict()
mapGroups = dict()
mapRelations = dict()

if not ExctractSILInformation(smproxy, mapFamilies, mapGroups, mapRelations):
  raise RuntimeError("Extraction SIL graph failed.")

# Sort families array by ID
sortedArray = mapFamilies.values()
sortedArray.sort()

# Prepare 'Annotation' list for lookup-table
numberValues = 0
annotationList = []
for idFamily in sortedArray:
  if idFamily not in dataArrayValues:
    continue

  if associationType == GetAssociationFromString("CELLS"):
    if idFamily > 0:
      continue
  elif associationType == GetAssociationFromString("POINTS"):
    if idFamily < 0:
      continue
  else:
    raise RuntimeError("Not implemented yet (%s)." % GetAssociationAsString(associationType))

  annotationList.append(str(idFamily))
  numberValues += 1

  # Iterate over all families to get group(s) by family name
  for famName, famID in mapFamilies.iteritems():
    if idFamily == famID:
      if mapRelations.has_key(famName):
        annotationList.append(str(', ').join(mapRelations.get(famName)))
      else:
        annotationList.append(str('No group'))

# Generate indexed colour for array
indexedColor = []
# Generate HSV colour from red to blue
if numberValues > 1:
  stepHue = int(240/(numberValues-1))
else:
  stepHue = 0.
for i in range(numberValues):
  indexedColor.extend(colorsys.hsv_to_rgb(float(i*stepHue)/360, 1, 1))

# Set scalar colouring
ColorBy(representation, (GetAssociationAsString(associationType), nameColorArray))

# Rescale colour and/or opacity maps used to include current data range
representation.RescaleTransferFunctionToDataRange(True)

# Show colour bar/colour legend
representation.SetScalarBarVisibility(renderView, True)

# Change representation type
representation.SetPropertyWithName("Representation", activeRepresentation)

# Get colour transfer function/colour map for 'FamilyIdCell' or 'FamilyIdNode'
lutFamily = GetLookupTableForArray(nameColorArray, 1)
lutFamily.SetPropertyWithName("ColorSpace", "RGB")

# Modify lookup table
lutFamily.InterpretValuesAsCategories = 1
lutFamily.Annotations = annotationList
lutFamily.IndexedColors = indexedColor
lutFamily.NumberOfTableValues = numberValues

# Modify scalar bar of view
lutScalarBar = GetScalarBar(lutFamily, renderView)
lutScalarBar.AddRangeAnnotations = 1
lutScalarBar.AddRangeLabels = 0
lutScalarBar.DrawAnnotations = 1
lutScalarBar.Title = 'Groups of %s' % scalarBarlabel

Render()
