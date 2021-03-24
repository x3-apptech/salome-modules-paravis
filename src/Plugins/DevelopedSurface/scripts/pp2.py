# Copyright (C) 2017-2021  CEA/DEN, EDF R&D
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
from math import pi,asin,acos,atan2
import numpy as np

invertThetaInc=False

mm=MEDFileMesh.New("example2_2.med")
c=mm.getCoords()

v=DataArrayDouble([0.70710678118617443,-0.70710678118572623,1],1,3)
v/=v.magnitude()[0]
vRef=DataArrayDouble([0,0,1],1,3)
vRot=DataArrayDouble.CrossProduct(v,vRef)
ang=asin(vRot.magnitude()[0])
MEDCouplingUMesh.Rotate3DAlg([0,0,0],vRot.getValues(),ang,c)

c2=c[:,[0,1]]
#
a,b=c2.findCommonTuples(1e-5)
pool=c2[a[b[:-1]]]
probes=[pool[:,0].getMaxValue()[1],pool[:,1].getMaxValue()[1],pool[:,0].getMinValue()[1]]
center,radius,_=pool[probes].asArcOfCircle()
assert(((c2-center).magnitude()).isUniform(radius,1e-5))
theta=((c2-center)/radius)
theta=DataArrayDouble([atan2(y,x) for x,y in theta])
zeTheta=theta-theta[0]
 
mm.write("tmp.med",2)
###########
c_cyl=DataArrayDouble.fromCartToCyl(c-(center+[0.]))
assert(c_cyl[:,0].isUniform(radius,1e-5))
tetha0=c_cyl[0,1]
c_cyl[:,1]-=tetha0
c_cyl_2=c_cyl[:,1] ; c_cyl_2.abs()
c_cyl[c_cyl_2.findIdsInRange(0.,1e-6),1]=0.
########
m0=mm[0]
c0s=m0.getCellIdsLyingOnNodes([0],False)
assert(len(c0s)!=0)
tmp=c_cyl[m0.getNodeIdsOfCell(c0s[0]),1].getMaxAbsValueInArray()
a=tmp>0.
if a^(not invertThetaInc):
    c_cyl[:,1]=-c_cyl[:,1]
    pass
#
c_cyl_post=c_cyl[:,1]
m0.convertAllToPoly()
tmp=MEDCoupling1DGTUMesh(m0) ; c=tmp.getNodalConnectivity() ; ci=tmp.getNodalConnectivityIndex()
for elt in c_cyl_post:
    if float(elt)<0:
        elt[:]=float(elt)+2*pi

nbCells=m0.getNumberOfCells()
nbNodes=m0.getNumberOfNodes()
newCoo=DataArrayInt(0,1)
cellsWithPb=[]
newConn=[]
for i in xrange(nbCells):
    tmp=c_cyl_post[c[ci[i]:ci[i+1]]]
    if tmp.getMaxValueInArray()-tmp.getMinValueInArray()>pi:
        cellsWithPb.append(i)
        # dup of low val
        newLocConn=[]
        for elt in c[ci[i]:ci[i+1]]:
            if float(c_cyl_post[elt])<=pi:
                newLocConn.append(nbNodes+len(newCoo))
                newCoo.pushBackSilent(int(elt))
                pass
            else:
                newLocConn.append(int(elt))
                pass
            pass
        newConn.append(newLocConn[:])
        pass


c_cyl_post=DataArrayDouble.Aggregate([c_cyl_post,c_cyl_post[newCoo]+2*pi])
z_post=DataArrayDouble.Aggregate([c_cyl[:,2],c_cyl[newCoo,2]])
newCoords=DataArrayDouble.Meld(radius*c_cyl_post,z_post)

z=DataArrayInt([len(elt) for elt in newConn])
z.computeOffsetsFull()
#
cellsWithPbMesh=MEDCoupling1DGTUMesh("",NORM_POLYGON)
cellsWithPbMesh.setNodalConnectivity(DataArrayInt(sum(newConn,[])),z)
cellsWithPbMesh.setCoords(newCoords)
#
ko_part_ids=DataArrayInt(cellsWithPb)
part_ids=ko_part_ids.buildComplement(nbCells)
part=m0[part_ids]
part.setCoords(newCoords)
whole=MEDCouplingUMesh.MergeUMeshesOnSameCoords([part,cellsWithPbMesh.buildUnstructured()])

o2n=DataArrayInt.Aggregate([part_ids,ko_part_ids])
whole=whole[o2n.invertArrayO2N2N2O(len(o2n))]
whole.setName("mesh")
WriteMesh("tmp3.med",whole,True)

#sk=m0.computeSkin().computeFetchedNodeIds()
#sk_part=part.computeSkin().computeFetchedNodeIds()
#m0[DataArrayInt(cellsWithPb)].computeFetchedNodeIds()
