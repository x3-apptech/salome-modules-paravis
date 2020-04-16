#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2015-2020  CEA/DEN, EDF R&D
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
# Author : Anthony Geay

import os
import sys

from medcoupling import *
from paraview.simple import *
from MEDReaderHelper import WriteInTmpDir,RetriveBaseLine
#### import the simple module from the paraview
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()
import vtk.test.Testing # this line must be here. If not SIGSEGV ! KW10658

def GenerateCase():
    """ This is a non regression bug revealed during training session. The bug is linked to the native Threshold VTK filter. Corrected with PV version >= 4.4. (KW10658)
    There is still a problem when both image comparison and Fetch are enable together."""
    fname="testMEDReader19.med"
    meshName="mesh"
    mm=MEDFileUMesh()
    coo=DataArrayDouble([(-0.3,-0.3),(0.2,-0.3),(0.7,-0.3),(-0.3,0.2),(0.2,0.2),(0.7,0.2),(-0.3,0.7),(0.2,0.7),(0.7,0.7)])
    conn0=[[NORM_TRI3,1,4,2],[NORM_TRI3,4,5,2],[NORM_QUAD4,0,3,4,1],[NORM_QUAD4,6,7,4,3],[NORM_QUAD4,7,8,5,4]]
    conn1=[[NORM_SEG2,4,5],[NORM_SEG2,5,2],[NORM_SEG2,1,0],[NORM_SEG2,6,7]]
    m0=MEDCouplingUMesh() ; m0.setCoords(coo) ; m0.setMeshDimension(2) ; m0.allocateCells(0)
    for c in conn0:
        m0.insertNextCell(c[0],c[1:])
    mm[0]=m0
    m1=MEDCouplingUMesh() ; m1.setCoords(coo) ; m1.setMeshDimension(1) ; m1.allocateCells(0)
    for c in conn1:
        m1.insertNextCell(c[0],c[1:])
    mm[-1]=m1
    mm.setName(meshName)
    mm.write(fname,2)
    return fname

@WriteInTmpDir
def test(baseline_file):
    fname = GenerateCase()
    # create a new 'MED Reader'
    testMEDReader19med = MEDReader(FileName=fname)
    testMEDReader19med.AllArrays = ['TS0/mesh/ComSup0/mesh@@][@@P0']
    testMEDReader19med.AllTimeSteps = ['0000']
    # Properties modified on testMEDReader19med
    testMEDReader19med.AllArrays = ['TS0/mesh/ComSup0/mesh@@][@@P0']

    if '-D' not in sys.argv:
        # get active view
        renderView1 = GetActiveViewOrCreate('RenderView')

        # reset view to fit data
        renderView1.ResetCamera()

        #changing interaction mode based on data extents
        renderView1.InteractionMode = '2D'
        renderView1.CameraPosition = [0.2, 0.2, 10000.0]
        renderView1.CameraFocalPoint = [0.2, 0.2, 0.0]
        testMEDReader19med.UpdatePipeline()
        # create a new 'Extract Cell Type'
        extractCellType1 = ExtractCellType(Input=testMEDReader19med)
        extractCellType1.AllGeoTypes = []

        # Properties modified on extractCellType1
        extractCellType1.AllGeoTypes = ['TRI3']

        # show data in view
        extractCellType1Display = Show(extractCellType1, renderView1)
        # trace defaults for the display properties.
        extractCellType1Display.ColorArrayName = [None, '']
        extractCellType1Display.ScalarOpacityUnitDistance = 0.5

        renderView1.InteractionMode = '2D'
        renderView1.CameraPosition = [0.2, 0.2, 10000.0]
        renderView1.CameraFocalPoint = [0.2, 0.2, 0.0]
        renderView1.CameraParallelScale = 0.7071067811865476

        res=servermanager.Fetch(extractCellType1,0)
        assert(res.GetBlock(0).GetNumberOfCells()==2) # problem was here in PV4.3.1

        # compare with baseline image # Waiting KW return to uncomment this part because SIGSEGV in PV5.
        import vtk.test.Testing
        from vtk.util.misc import vtkGetTempDir
        vtk.test.Testing.VTK_TEMP_DIR = vtk.util.misc.vtkGetTempDir()
        vtk.test.Testing.compareImage(GetActiveView().GetRenderWindow(), baseline_file, threshold=1)
        vtk.test.Testing.interact()

if __name__ == "__main__":
    imgName="testMEDReader19.png"
    baseline_file = RetriveBaseLine(imgName)
    test(baseline_file)
