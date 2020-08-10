#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2020  CEA/DEN, EDF R&D
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


__doc__ = """
Test of GroupsAsMultiBlocks filter in the MEDReader plugin.
"""

from paraview.simple import *
paraview.simple._DisableFirstRenderCameraReset()
from vtk.util import numpy_support
import numpy as np
import medcoupling as mc
from MEDReaderHelper import WriteInTmpDir,RetriveBaseLine

def MyAssert(clue):
    if not clue:
        raise RuntimeError("Assertion failed !")

def generateCase(fname):
    arr = mc.DataArrayDouble([0,1,2,3,4,5])
    m = mc.MEDCouplingCMesh()
    m.setCoords(arr,arr)
    m = m.buildUnstructured()
    m.changeSpaceDimension(3,0.)
    m.setName("mesh")
    mm = mc.MEDFileUMesh()
    mm[0] = m
    grp_BottomLeft = mc.DataArrayInt([0,1,5,6]) ; grp_BottomLeft.setName("BottomLeft")
    grp_BottomRight = mc.DataArrayInt([3,4,8,9]) ; grp_BottomRight.setName("BottomRight")
    grp_TopLeft = mc.DataArrayInt([15,16,20,21]) ; grp_TopLeft.setName("TopLeft")
    grp_TopRight = mc.DataArrayInt([18,19,23,24]) ; grp_TopRight.setName("TopRight")
    mm.setGroupsAtLevel(0,[grp_BottomLeft,grp_BottomRight,grp_TopLeft,grp_TopRight])
    mm.write(fname,2)
    f = mc.MEDCouplingFieldDouble(mc.ON_CELLS)
    f.setMesh(m)
    f.setArray(m.computeCellCenterOfMass().magnitude())
    f.setName("field")
    mc.WriteFieldUsingAlreadyWrittenMesh(fname,f)
    f2 = mc.MEDCouplingFieldDouble(mc.ON_NODES)
    f2.setMesh(m)
    f2.setArray(m.getCoords().magnitude())
    f2.setName("field2")
    mc.WriteFieldUsingAlreadyWrittenMesh(fname,f2)
    return f,f2,grp_BottomLeft,grp_BottomRight,grp_TopLeft,grp_TopRight

@WriteInTmpDir
def test():
    fname = "testMEDReader22.med"
    f,f2,grp_BottomLeft,grp_BottomRight,grp_TopLeft,grp_TopRight = generateCase(fname)
    reader = MEDReader(FileName=fname)
    reader.AllArrays = ['TS0/mesh/ComSup0/field@@][@@P0','TS0/mesh/ComSup0/field2@@][@@P1','TS0/mesh/ComSup0/mesh@@][@@P0']
    reader.AllTimeSteps = ['0000']

    groupsAsMultiBlocks = GroupsAsMultiBlocks(Input=reader)
    groupsAsMultiBlocks.UpdatePipeline()
    blocks = servermanager.Fetch(groupsAsMultiBlocks)
    MyAssert(blocks.GetNumberOfBlocks()==4)
    # test of bottom left
    ds_bl = blocks.GetBlock(0)
    MyAssert(ds_bl.GetNumberOfCells()==4)
    bl_ref_conn = np.array([ 1,  0,  6,  7,  2,  1,  7,  8,  7,  6, 12, 13,  8,  7, 13, 14],dtype=np.int32)
    MyAssert(ds_bl.GetCellData().GetNumberOfArrays() == 3 )# 3 for field, mesh and FamilyIdCell
    MyAssert(np.all( bl_ref_conn == numpy_support.vtk_to_numpy( ds_bl.GetCells().GetConnectivityArray() )) )
    MyAssert( mc.DataArrayDouble(numpy_support.vtk_to_numpy( ds_bl.GetCellData().GetArray("field") )).isEqual(f.getArray()[grp_BottomLeft],1e-12) )
    # test of bottom right
    ds_br = blocks.GetBlock(1)
    MyAssert(ds_br.GetNumberOfCells()==4)
    br_ref_conn = np.array([ 4,  3,  9, 10,  5,  4, 10, 11, 10,  9, 15, 16, 11, 10, 16, 17],dtype=np.int32)
    MyAssert(np.all( br_ref_conn == numpy_support.vtk_to_numpy( ds_br.GetCells().GetConnectivityArray() )) )
    MyAssert( mc.DataArrayDouble(numpy_support.vtk_to_numpy( ds_br.GetCellData().GetArray("field") )).isEqual(f.getArray()[grp_BottomRight],1e-12) )
    # test of top left
    ds_tl = blocks.GetBlock(2)
    MyAssert(ds_tl.GetNumberOfCells()==4)
    tl_ref_conn = np.array([ 19, 18, 24, 25, 20, 19, 25, 26, 25, 24, 30, 31, 26, 25, 31, 32],dtype=np.int32)
    MyAssert(np.all( tl_ref_conn == numpy_support.vtk_to_numpy( ds_tl.GetCells().GetConnectivityArray() )) )
    MyAssert( mc.DataArrayDouble(numpy_support.vtk_to_numpy( ds_tl.GetCellData().GetArray("field") )).isEqual(f.getArray()[grp_TopLeft],1e-12) )
    # test of top right
    ds_tr = blocks.GetBlock(3)
    MyAssert(ds_tr.GetNumberOfCells()==4)
    tr_ref_conn = np.array([ 22, 21, 27, 28, 23, 22, 28, 29, 28, 27, 33, 34, 29, 28, 34, 35],dtype=np.int32)
    MyAssert(np.all( tr_ref_conn == numpy_support.vtk_to_numpy( ds_tr.GetCells().GetConnectivityArray() )) )
    MyAssert( mc.DataArrayDouble(numpy_support.vtk_to_numpy( ds_tr.GetCellData().GetArray("field") )).isEqual(f.getArray()[grp_TopRight],1e-12) )
    #
    for ds in [ds_bl,ds_br,ds_tl,ds_tr]:
        MyAssert(ds.GetPointData().GetNumberOfArrays() == 1 )# 1 for field2
        MyAssert(ds.GetNumberOfPoints()==36) # for performance reasons all blocks share the same input coordinates
        MyAssert(mc.DataArrayDouble( numpy_support.vtk_to_numpy( ds.GetPointData().GetArray("field2") ) ).isEqual(f2.getArray(),1e-12) )

if __name__ == "__main__":
    test()
    
