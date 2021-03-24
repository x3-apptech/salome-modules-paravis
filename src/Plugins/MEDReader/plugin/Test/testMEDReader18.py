#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2015-2021  CEA/DEN, EDF R&D
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


from medcoupling import *
from paraview.simple import *
from MEDReaderHelper import WriteInTmpDir,RetriveBaseLine

def GenerateCase():
    """Non regression test for bug EDF11343. Extract group on groups mixing cells entities and node entities."""
    fname="testMEDReader18.med"
    arr1=DataArrayDouble(5) ; arr1.iota()
    arr2=DataArrayDouble([0,1])
    m=MEDCouplingCMesh() ; m.setCoords(arr1,arr2)
    m.setName("mesh")
    m=m.buildUnstructured()
    #
    mm=MEDFileUMesh()
    mm[0]=m
    #
    grp0=DataArrayInt([1,2]) ; grp0.setName("grp0")
    grp1=DataArrayInt([3,4,8,9]) ; grp1.setName("grp1")
    #
    mm.addGroup(0,grp0)
    mm.addGroup(1,grp1)
    #
    mm.write(fname,2)
    return fname

@WriteInTmpDir
def test():
  fname = GenerateCase()
  reader=MEDReader(FileName=fname)
  reader.AllArrays=['TS0/mesh/ComSup0/mesh@@][@@P0']
  ExtractGroup1 = ExtractGroup(Input=reader)
  ExtractGroup1.AllGroups=["GRP_grp0","GRP_grp1"]
  #ExtractGroup1.UpdatePipelineInformation()
  res=servermanager.Fetch(ExtractGroup1,0)
  assert(res.GetNumberOfBlocks()==2)
  assert(res.GetBlock(1).GetNumberOfCells()==1)
  assert(res.GetBlock(0).GetNumberOfCells()==2)
  pass

if __name__ == "__main__":
    test()
    pass


