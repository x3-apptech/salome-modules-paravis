# Copyright (C) 2017-2019  EDF R&D
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

from MEDLoader import *

fname="VoroGauss1.med"
meshName="mesh"
mm=MEDFileUMesh()
coords=DataArrayDouble([0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 0,1, 1,1, 2,1, 0,2, 1,2, 3,1, 4,1],13,2)
m0=MEDCouplingUMesh(meshName,2)
m0.setCoords(coords)
m0.allocateCells()
m0.insertNextCell(NORM_TRI3,[2,3,8])
m0.insertNextCell(NORM_TRI3,[3,4,11])
m0.insertNextCell(NORM_TRI3,[4,5,12])
m0.insertNextCell(NORM_TRI3,[6,7,9])
m0.insertNextCell(NORM_TRI3,[7,8,10])
m0.insertNextCell(NORM_QUAD4,[0,1,7,6])
m0.insertNextCell(NORM_QUAD4,[1,2,8,7])
mm[0]=m0
m1=MEDCouplingUMesh(meshName,1)
m1.setCoords(coords)
m1.allocateCells()
m1.insertNextCell(NORM_SEG2,[0,1])
m1.insertNextCell(NORM_SEG2,[1,2])
m1.insertNextCell(NORM_SEG2,[2,3])
m1.insertNextCell(NORM_SEG2,[3,4])
m1.insertNextCell(NORM_SEG2,[4,5])
mm[-1]=m1
mm.setFamilyFieldArr(0,DataArrayInt([-1,-1,-2,-3,-3,-1,-3]))
mm.setFamilyFieldArr(-1,DataArrayInt([-1,-4,-4,-4,-1]))
for i in [-1,-2,-3,-4]:
    mm.setFamilyId("Fam_%d"%i,i)
    mm.setFamiliesOnGroup("G%d"%(abs(i)),["Fam_%d"%i])
    pass
mm.write(fname,2)
#
f0=MEDCouplingFieldDouble(ON_GAUSS_PT)
f0.setMesh(m0)
f0.setName("MyFieldPG") ; f0.setMesh(m0)
f0.setGaussLocalizationOnType(NORM_TRI3,[0,0, 1,0, 0,1],[0.1,0.1, 0.8,0.1, 0.1,0.8],[0.3,0.3,0.4])
f0.setGaussLocalizationOnType(NORM_QUAD4,[-1,-1, 1,-1, 1,1, -1,1],[-0.57735,-0.57735,0.57735,-0.57735,0.57735,0.57735,-0.57735,0.57735],[0.25,0.25,0.25,0.25])
arr=DataArrayDouble(f0.getNumberOfTuplesExpected()) ; arr.iota()
arr=DataArrayDouble.Meld(arr,arr)
arr.setInfoOnComponents(["comp0","comp1"])
f0.setArray(arr)
WriteFieldUsingAlreadyWrittenMesh(fname,f0)
#
f1=MEDCouplingFieldDouble(ON_CELLS)
f1.setMesh(m0)
f1.setName("MyFieldCell") ; f1.setMesh(m0)
arr=DataArrayDouble(f1.getNumberOfTuplesExpected()) ; arr.iota()
arr=DataArrayDouble.Meld(arr,arr)
arr.setInfoOnComponents(["comp2","comp3"])
f1.setArray(arr)
WriteFieldUsingAlreadyWrittenMesh(fname,f1)

