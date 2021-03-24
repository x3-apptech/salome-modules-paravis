# Copyright (C) 2016-2021  CEA/DEN, EDF R&D
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
import MEDLoader as ml
import os
from math import pi,sqrt
import tempfile

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

tmpdir = tempfile.TemporaryDirectory(prefix="MEDWriter_")

pat=os.path.join(tmpdir.name, 'testMEDWriter_%i.med')
fname0=pat%0
fname1=pat%1
fname2=pat%2
fname3=pat%3
fname4=pat%4
fname4_vtp=os.path.splitext(pat%4)[0]+".vtp"
fname5=pat%5
fname6_vtu=os.path.splitext(pat%6)[0]+".vtu"
fname6=pat%6
fname7_vtu=os.path.splitext(pat%7)[0]+".vtu"
fname7=pat%7
fname8_vtr=os.path.splitext(pat%8)[0]+".vtr"
fname8=pat%8

##### First test with a simple sphere

plane1 = Sphere()
SaveData(fname0,proxy=plane1,WriteAllTimeSteps=1)
#
totomed=MEDReader(FileName=fname0)
totomed.AllArrays=['TS1/Mesh/ComSup0/Mesh@@][@@P0']
totomed.AllTimeSteps=['0000']
SaveData(fname1,proxy=totomed,WriteAllTimeSteps=1)
# Sphere has been written. Try to check to write it in MED file !
mfd=ml.MEDFileData(fname0)
mm=mfd.getMeshes()[0] ; m0=mm[0]
area=m0.getMeasureField(True).accumulate()[0]
assert(abs(sqrt(area/(4*pi))-0.975/2.)<0.01) # 4*pi*radius**2
f=mfd.getFields()[0][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,mm)
f_array_double = f.getArray().convertToDblArr() # f is MEDCouplingFieldFloat
assert(abs(ml.DataArrayDouble(f_array_double.accumulate(),1,3).magnitude()[0])<1e-12) # sum of all normal vector should be 0


##### Build a MED file from scratch

fieldName0="F0"
fieldName1="F1"
c=ml.DataArrayDouble([0.,0.,1.,1.,2.,2.,1.,0.,2.,0.,0.,2.,1.,2.,0.,1.,2.,1.],9,2)
c.setInfoOnComponents(["X abc","Y defg"])
#
mName="mesh"
m0=ml.MEDCouplingUMesh(mName,2) ; m0.allocateCells() ; m0.setCoords(c)
m0.insertNextCell(ml.NORM_TRI3,[3,1,4])
m0.insertNextCell(ml.NORM_TRI3,[1,8,4])
m0.insertNextCell(ml.NORM_QUAD4,[0,7,1,3])
m0.insertNextCell(ml.NORM_QUAD4,[1,6,2,4])
m0.insertNextCell(ml.NORM_QUAD4,[7,5,6,1])
m1=ml.MEDCouplingUMesh(mName,1) ; m1.allocateCells() ; m1.setCoords(c)
m1.insertNextCell(ml.NORM_SEG2,[0,7])
m1.insertNextCell(ml.NORM_SEG2,[7,5])
m1.insertNextCell(ml.NORM_SEG2,[5,6])
mm=ml.MEDFileUMesh() ; mm[0]=m0 ; mm[-1]=m1
mm.setFamilyFieldArr(1,ml.DataArrayInt([200,201,202,203,204,205,206,207,208]))
mm.setFamilyFieldArr(0,ml.DataArrayInt([100,101,102,103,104]))
mm.setFamilyFieldArr(-1,ml.DataArrayInt([105,106,107]))
mm.setFamilyId("Fam3",3) ; mm.setFamilyId("Fam5",7)
mm.setFamiliesOnGroup("gr0",["Fam3"])
mm.setFamiliesOnGroup("gr1",["Fam5"])
mm.setFamiliesOnGroup("gr2",["Fam3","Fam5"])
mm.write(fname2,2)
#
f1ts0=ml.MEDFileField1TS()
f0=ml.MEDCouplingFieldDouble(ml.ON_CELLS) ; f0.setName(fieldName0)
f0.setMesh(m0) ; f0.setArray(ml.DataArrayDouble([8,7,6,5,4]))
f1ts0.setFieldNoProfileSBT(f0)
f0=ml.MEDCouplingFieldDouble(ml.ON_CELLS) ; f0.setName(fieldName0)
f0.setMesh(m1) ; f0.setArray(ml.DataArrayDouble([3,2,1]))
f0.setTime(0,0,0)
f1ts0.setFieldNoProfileSBT(f0)
f1ts0.write(fname2,0)
#
f1ts1=ml.MEDFileField1TS()
f0=ml.MEDCouplingFieldDouble(ml.ON_NODES) ; f0.setName(fieldName1)
arr=ml.DataArrayDouble([9,109,8,108,7,107,6,106,5,105,4,104,3,103,2,102,1,101],9,2)
arr.setInfoOnComponents(["aa","bbb"])
f0.setMesh(m0) ; f0.setArray(arr)
f0.setTime(0,0,0)
f1ts1.setFieldNoProfileSBT(f0)
f1ts1.write(fname2,0)
#
test3=MEDReader(FileName=fname2)
test3.AllArrays=['TS0/%s/ComSup0/%s@@][@@P0'%(mName,fieldName0),'TS0/%s/ComSup0/%s@@][@@P1'%(mName,fieldName1)]
test3.AllTimeSteps = ['0000']
SaveData(fname3,proxy=test3,WriteAllTimeSteps=1)
### test content of fname3
mfd2=ml.MEDFileData(fname3)
mm2=mfd2.getMeshes()[0]
c1=mm2.getCoords()
assert(c.isEqualWithoutConsideringStr(c1[:,:2],1e-12))
fs2=ml.MEDFileFields(fname3)
assert(len(fs2)==2)
assert(mm2.getSpaceDimension()==3) ; assert(mm2.getCoords()[:,2].isUniform(0.,0.))
m2_0=mm2[0].deepCopy() ; m2_0.changeSpaceDimension(2,0.) ; m2_0.getCoords().setInfoOnComponents(mm[0].getCoords().getInfoOnComponents())
assert(m2_0.isEqual(mm[0],1e-12))
m2_1=mm2[-1].deepCopy() ; m2_1.changeSpaceDimension(2,0.) ; m2_1.getCoords().setInfoOnComponents(mm[0].getCoords().getInfoOnComponents())
assert(m2_1.isEqual(mm[-1],1e-12))
f2_0=mfd2.getFields()[fieldName0][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm2) ; f2_0.setMesh(m2_0)
assert(f1ts0.getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm).isEqual(f2_0,1e-12,1e-12))
f2_1=mfd2.getFields()[fieldName1][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,mm2) ; f2_1.setMesh(m2_0)
assert(f1ts1.getFieldOnMeshAtLevel(ml.ON_NODES,0,mm).isEqual(f2_1,1e-12,1e-12))
assert(mm2.getGroupsNames()==('gr0','gr1','gr2'))
assert(mm2.getFamiliesOnGroup("gr0")==("Fam3",))
assert(mm2.getFamiliesOnGroup("gr1")==("Fam5",))
assert(mm2.getFamiliesOnGroup("gr2")==("Fam3","Fam5"))
assert(mm2.getFamiliesNames()==('FAMILLE_ZERO','Fam3','Fam5'))
assert([mm2.getFamilyId(elt) for elt in ['FAMILLE_ZERO','Fam3','Fam5']]==[0,3,7])
assert(mm2.getFamilyFieldAtLevel(0).isEqual(mm.getFamilyFieldAtLevel(0)))
assert(mm2.getFamilyFieldAtLevel(-1).isEqual(mm.getFamilyFieldAtLevel(-1)))
assert(mm2.getFamilyFieldAtLevel(1).isEqual(mm.getFamilyFieldAtLevel(1)))
# Write a polydata mesh
mergeBlocks1 = MergeBlocks(Input=test3)
extractSurface1 = ExtractSurface(Input=mergeBlocks1)
SaveData(fname4_vtp,proxy=extractSurface1)
test4vtp = XMLPolyDataReader(FileName=[fname4_vtp])
test4vtp.CellArrayStatus = ['F0', 'FamilyIdCell']
SaveData(fname5,proxy=test4vtp,WriteAllTimeSteps=1)
### test content of fname5
mfd5=ml.MEDFileData(fname5)
m5=mfd5.getMeshes()[0][0].deepCopy()
assert(m5.getSpaceDimension()==3) # 
m5.setName(mm.getName()) ; m5.changeSpaceDimension(2,0.) ; m5.getCoords().setInfoOnComponents(mm[0].getCoords().getInfoOnComponents())
bary5=m5.computeCellCenterOfMass()
bary=mm[0].computeCellCenterOfMass()
a,b=bary5.areIncludedInMe(bary,1e-12) ; assert(a)
a,c=mm[0].getCoords().areIncludedInMe(m5.getCoords(),1e-12) ; assert(a)
m5.renumberNodes(c,len(c))#c.invertArrayO2N2N2O(len(c)))
assert(m5.unPolyze())
assert(m5.getCoords().isEqual(mm[0].getCoords(),1e-12))
assert(m5.isEqual(mm[0],1e-12))
f5_0=mfd5.getFields()[fieldName0][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mfd5.getMeshes()[0]) ; f5_0.setMesh(m5)
assert(f1ts0.getFieldOnMeshAtLevel(ml.ON_CELLS,0,mm).isEqual(f5_0,1e-12,1e-12))
f5_1=mfd5.getFields()[fieldName1][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,mfd5.getMeshes()[0]) ; f5_1.setMesh(m5)
f5_1.setArray(f5_1.getArray()[c.invertArrayO2N2N2O(len(c))])
assert(f1ts1.getFieldOnMeshAtLevel(ml.ON_NODES,0,mm).isEqual(f5_1,1e-12,1e-12))

### test with a non geo types non sorted

c=ml.DataArrayDouble([0.,0.,1.,1.,2.,2.,1.,0.,2.,0.,0.,2.,1.,2.,0.,1.,2.,1.],9,2)
c.setInfoOnComponents(["X abc","Y defg"])
m6=ml.MEDCouplingUMesh(mName,2) ; m6.allocateCells() ; m6.setCoords(c)
m6.insertNextCell(ml.NORM_TRI3,[3,1,4])
m6.insertNextCell(ml.NORM_QUAD4,[0,7,1,3])
m6.insertNextCell(ml.NORM_TRI3,[1,8,4])
m6.insertNextCell(ml.NORM_QUAD4,[1,6,2,4])
m6.insertNextCell(ml.NORM_QUAD4,[7,5,6,1])
fieldName6="F6"
f6=ml.MEDCouplingFieldDouble(ml.ON_CELLS) ; f6.setMesh(m6) ; f6.setName(fieldName6)
f6.setArray(ml.DataArrayDouble([20,21,22,23,24]))
f6.writeVTK(fname6_vtu)
test6vtu=XMLUnstructuredGridReader(FileName=[fname6_vtu])
SaveData(fname6,proxy=test6vtu,WriteAllTimeSteps=1)
mfd7=ml.MEDFileData(fname6)
assert(len(mfd7.getMeshes())==1)
m7=mfd7.getMeshes()[0][0]
assert(len(mfd7.getFields())==1)
f7=mfd7.getFields()[0][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mfd7.getMeshes()[0])
assert(f7.getMesh().isEqual(m7,1e-12))
assert(m7.getCoords()[:,:2].isEqualWithoutConsideringStr(m6.getCoords(),1e-12))
assert(m7.getNodalConnectivity().isEqual(ml.DataArrayInt([3,3,1,4,3,1,8,4,4,0,7,1,3,4,1,6,2,4,4,7,5,6,1]))) # there is a permutation of cells
assert(m7.getNodalConnectivityIndex().isEqual(ml.DataArrayInt([0,4,8,13,18,23]))) # there is a permutation of cells
assert(f7.getArray().isEqual(ml.DataArrayFloat([20,22,21,23,24]),1e-12)) # there is a permutation of cells

### test with polyhedron

m8=ml.MEDCouplingCMesh() ; m8.setCoords(ml.DataArrayDouble([0,1,2,3]),ml.DataArrayDouble([0,1]),ml.DataArrayDouble([0,1]))
m8=m8.buildUnstructured()
m8.getCoords().setInfoOnComponents(['X', 'Y', 'Z'])
m8_0=m8[0] ; m8_0.simplexize(ml.PLANAR_FACE_5)
m8_1=m8[1:]
m8_1.convertAllToPoly()
m8=ml.MEDCouplingUMesh.MergeUMeshesOnSameCoords([m8_0,m8_1])
m8=m8[[0,5,1,2,6,3,4]]
fieldName8="F8"
f8=ml.MEDCouplingFieldDouble(ml.ON_CELLS) ; f8.setMesh(m8) ; f8.setName(fieldName8)
f8.setArray(ml.DataArrayDouble([20,21,22,23,24,25,26]))
f8.writeVTK(fname7_vtu)
test8vtu=XMLUnstructuredGridReader(FileName=[fname7_vtu])
SaveData(fname7,proxy=test8vtu,WriteAllTimeSteps=1)
mfd9=ml.MEDFileData(fname7)
assert(len(mfd9.getMeshes())==1)
m9=mfd9.getMeshes()[0][0]
assert(len(mfd9.getFields())==1)
assert(m9.getCoords().isEqual(m8.getCoords(),1e-12))
c9=ml.DataArrayInt([0,2,3,5,6,1,4])
assert(m8[c9].isEqualWithoutConsideringStr(m9,1e-12))
f9=mfd9.getFields()[0][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,mfd9.getMeshes()[0])
f9_array_double = f9.getArray().convertToDblArr() # f9 is MEDCouplingFieldFloat
assert(f9_array_double.isEqual(f8.getArray()[c9],1e-12))

### test with cartesian

FieldName10="F10"
FieldName10_n="F10_n"
m10=ml.MEDCouplingCMesh()
m10.setCoordsAt(0,ml.DataArrayDouble([0,1,2]))
m10.setCoordsAt(1,ml.DataArrayDouble([1,2,3,4]))
m10.setCoordsAt(2,ml.DataArrayDouble([3,5,6,7,8]))
f10=ml.MEDCouplingFieldDouble(ml.ON_CELLS) ; f10.setMesh(m10)
f10.setName(FieldName10)
f10.setArray(ml.DataArrayInt.Range(0,m10.getNumberOfCells(),1).convertToDblArr()) ; f10.checkConsistencyLight()
f10_n=ml.MEDCouplingFieldDouble(ml.ON_NODES) ; f10_n.setMesh(m10)
f10_n.setName(FieldName10_n)
f10_n.setArray(ml.DataArrayInt.Range(0,m10.getNumberOfNodes(),1).convertToDblArr()) ; f10_n.checkConsistencyLight()
ml.MEDCouplingFieldDouble.WriteVTK(fname8_vtr,[f10,f10_n])
test10vtr=XMLRectilinearGridReader(FileName=[fname8_vtr])
SaveData(fname8,proxy=test10vtr,WriteAllTimeSteps=1)
mfd11=ml.MEDFileData(fname8)
assert(len(mfd11.getMeshes())==1)
assert(len(mfd11.getFields())==2)
mfd11=ml.MEDFileData(fname8)
m11=mfd11.getMeshes()[0]
assert(isinstance(m11,ml.MEDFileCMesh))
f11=mfd11.getFields()[FieldName10][0].getFieldOnMeshAtLevel(ml.ON_CELLS,0,m11)
f11_n=mfd11.getFields()[FieldName10_n][0].getFieldOnMeshAtLevel(ml.ON_NODES,0,m11)
f11_array_double = f11.getArray().convertToDblArr() # f11 is MEDCouplingFieldFloat
assert(f11_array_double.isEqualWithoutConsideringStr(f10.getArray(),1e-12))
f11_n_array_double = f11_n.getArray().convertToDblArr() # f11_n is MEDCouplingFieldFloat
assert(f11_n_array_double.isEqualWithoutConsideringStr(f10_n.getArray(),1e-12))

