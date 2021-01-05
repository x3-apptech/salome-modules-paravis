# Copyright (C) 2016-2021  CEA/DES, EDF R&D
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

#### import the simple module from the paraview
from paraview.simple import *
import MEDLoader as ml
import os
from math import pi,sqrt

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

#fname_int32='testMEDWriter_int32.med'
fname_int64 = 'testMEDWriter_int64.med'
fname_int64_exported = 'testMEDWriter_int64_exported.med'

### test with int array field

fieldNameOnCells = "field_on_cells"
fieldNameOnNodes = "field_on_nodes"

c=ml.DataArrayDouble([0.,0., 1.,0., 1.,1., 0.,1.],4,2)
c.setInfoOnComponents(["X","Y"])
#
mName="mesh"
m1=ml.MEDCouplingUMesh(mName,2) ; m1.allocateCells() ; m1.setCoords(c)
m1.insertNextCell(ml.NORM_TRI3,[0,1,2])
m1.insertNextCell(ml.NORM_TRI3,[0,2,3])
mm1=ml.MEDFileUMesh()
mm1[0]=m1
# family on cell
mm1.setFamilyFieldArr(0,ml.DataArrayInt([1, 2]))
mm1.setFamilyId("Fam1",1)
mm1.setFamilyId("Fam2",2)
mm1.setFamiliesOnGroup("tri1",["Fam1"])
mm1.setFamiliesOnGroup("tri2",["Fam2"])
# family on nodes
mm1.setFamilyFieldArr(1,ml.DataArrayInt([-1, -1, -2, -2]))
mm1.setFamilyId("Fam_1",-1)
mm1.setFamilyId("Fam_2",-2)
mm1.setFamiliesOnGroup("bottom",["Fam_1"])
mm1.setFamiliesOnGroup("top",["Fam_2"])
# ids of cells (write the cells ids as it is done is SMESH)
mm1.setRenumFieldArr(0, ml.DataArrayInt([0,1]))
# ids of nodes (write the nodes ids as it is done is SMESH)
mm1.setRenumFieldArr(1, ml.DataArrayInt([0,1,2,3]))
# write mesh
mm1.write(fname_int64,2)

# field of int64 on cells
f1ts12=ml.MEDFileInt64Field1TS()
f1=ml.MEDCouplingFieldInt64(ml.ON_CELLS)
f1.setName(fieldNameOnCells)
f1.setMesh(m1)
f1.setArray(ml.DataArrayInt64([0,1]))
f1.setTime(0,0,0)
f1ts12.setFieldNoProfileSBT(f1)
f1ts12.write(fname_int64,0)
# field of int64 on nodes
f1ts12n=ml.MEDFileInt64Field1TS()
f1n=ml.MEDCouplingFieldInt64(ml.ON_NODES)
f1n.setName(fieldNameOnNodes)
arr=ml.DataArrayInt64([0,1,2,3])
f1n.setMesh(m1)
f1n.setArray(arr)
f1n.setTime(0,0,0)
f1ts12n.setFieldNoProfileSBT(f1n)
f1ts12n.write(fname_int64,0)

test12=MEDReader(FileName=fname_int64)
#test12.AllArrays=['TS0/%s/ComSup0/%s@@][@@P0'%(mName,fieldNameOnCells),'TS0/%s/ComSup0/%s@@][@@P1'%(mName,fieldNameOnNodes)]
#test12.AllTimeSteps = ['0000']
SaveData(fname_int64_exported, WriteAllTimeSteps=1)
### test content of fname_int64_exported
mfd2=ml.MEDFileData(fname_int64_exported)
mm2=mfd2.getMeshes()[0]
c1=mm2.getCoords()
assert(c.isEqualWithoutConsideringStr(c1[:,:2],1e-12))
fs2=ml.MEDFileFields(fname_int64_exported)

# check mesh
assert(mm2.getSpaceDimension()==3)
assert(mm2.getCoords()[:,2].isUniform(0.,0.))
m2_0=mm2[0].deepCopy()
m2_0.changeSpaceDimension(2,0.)
m2_0.getCoords().setInfoOnComponents(mm1[0].getCoords().getInfoOnComponents())
assert(m2_0.isEqual(mm1[0],1e-12))

# check field on cells
f2_0=mfd2.getFields()[fieldNameOnCells][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm2)
f2_0.setMesh(m2_0)
#assert(f1ts12.getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm1).isEqual(f2_0,1e-12,0))
assert(f1.isEqual(f2_0,1e-12,0))

# check field on nodes
f2_1=mfd2.getFields()[fieldNameOnNodes][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,mm2)
f2_1.setMesh(m2_0)
#assert(f1ts12n.getFieldOnMeshAtLevel(ml.ON_NODES,0,mm1).isEqual(f2_1,1e-12,0))
assert(f1n.isEqual(f2_1,1e-12,0))

# check field of cells numbers (set by setRenumFieldArr at level 0)
f3=f1.deepCopy()
f3.setName("NumIdCell")
f3_0=mfd2.getFields()["NumIdCell"][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm2)
f3_0.setMesh(m2_0)
assert(f3.isEqual(f3_0,1e-12,0))

# check field of nodes numbers (set by setRenumFieldArr at level 1)
f4=f1n.deepCopy()
f4.setName("NumIdNode")
f4_1=mfd2.getFields()["NumIdNode"][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,mm2)
f4_1.setMesh(m2_0)
assert(f4.isEqual(f4_1,1e-12,0))
