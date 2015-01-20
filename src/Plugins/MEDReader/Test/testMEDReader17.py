#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2007-2014  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

from MEDLoader import *

""" This test is a non regression test of EDF9622."""

fname="testMEDReader17.med"

arr=DataArrayDouble([0,1,2])
m=MEDCouplingCMesh() ; m.setCoords(arr,arr) ; m=m.buildUnstructured() ; m.setName("Mesh")
mm=MEDFileUMesh() ; mm.setMeshAtLevel(0,m)
grp0=DataArrayInt([0,1]) ; grp0.setName("grp0")
grp1=DataArrayInt([2,3]) ; grp1.setName("grp1")
grp2=DataArrayInt([1,2]) ; grp2.setName("grp2")
grp3=DataArrayInt([0,1,2,3]) ; grp3.setName("grp3")
mm.setGroupsAtLevel(0,[grp0,grp1,grp2,grp3])
f=MEDCouplingFieldDouble(ON_GAUSS_NE) ; f.setMesh(m) ; f.setName("MyField") ; f.setTime(0.,0,0)
arr2=DataArrayDouble(4*4*2) ; arr2.iota() ; arr2.rearrange(2) ; arr2.setInfoOnComponents(["aa","bbb"])
f.setArray(arr2) ; arr2+=0.1 ; f.checkCoherency()
mm.write(fname,2)
MEDLoader.WriteFieldUsingAlreadyWrittenMesh(fname,f)
#
from paraview.simple import *
from paraview import servermanager

paraview.simple._DisableFirstRenderCameraReset()
reader=MEDReader(FileName=fname)
ExpectedEntries=['TS0/Mesh/ComSup0/MyField@@][@@GSSNE','TS1/Mesh/ComSup0/Mesh@@][@@P0']
assert(reader.GetProperty("FieldsTreeInfo")[::2]==ExpectedEntries)
reader.AllArrays=['TS0/Mesh/ComSup0/MyField@@][@@GSSNE']
ExtractGroup1 = ExtractGroup(Input=reader)
ExtractGroup1.UpdatePipelineInformation()
ExtractGroup1.AllGroups=["GRP_grp1"]
ELNOPoints1=ELNOPoints(Input=ExtractGroup1)
ELNOPoints1.SelectSourceArray=['ELNO@MyField']
#
ELNOPoints1=ELNOPoints(Input=reader)
ELNOPoints1.SelectSourceArray=['ELNO@MyField']
ExtractGroup1 = ExtractGroup(Input=ELNOPoints1)
ExtractGroup1.UpdatePipelineInformation()
ExtractGroup1.AllGroups=["GRP_grp1"]
ExtractGroup1.UpdatePipeline()
res=servermanager.Fetch(ExtractGroup1,0)
assert(res.GetBlock(0).GetNumberOfCells()==8)
vtkArrToTest=res.GetBlock(0).GetPointData().GetArray("MyField")
assert(vtkArrToTest.GetNumberOfComponents()==2)
assert(vtkArrToTest.GetNumberOfTuples()==8)
vals=[vtkArrToTest.GetValue(i) for i in xrange(16)]
assert(DataArrayDouble([(16.1,17.1),(18.1,19.1),(20.1,21.1),(22.1,23.1),(24.1,25.1),(26.1,27.1),(28.1,29.1),(30.1,31.1)]).isEqual(DataArrayDouble(vals,8,2),1e-12))
