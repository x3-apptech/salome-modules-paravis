# Copyright (C) 2017  EDF R&D
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
from math import sqrt
import numpy as np
import scipy
import scipy.sparse.linalg

def f0(sample,p,v,r):
    d=sample-p
    return d.magnitude()[0]-r

def f(sample,p,v,r):
    d=sample-p
    l=d-DataArrayDouble.Dot(d,v)[0]*v
    return l.magnitude()[0]-r

def f2(sample,zev,ff):
    p=zev[0,:3] ; v=zev[0,3:6] ; r=zev[0,6]
    return ff(sample,p,v,r)

def df(sample,zev,varid,ff):
    eps=0.0001
    zev=zev[:]
    zev2=zev[:]
    zev[0,varid]+=eps ; zev2[0,varid]-=eps
    return (f2(sample,zev,ff)-f2(sample,zev2,ff))/(2*eps)

#def df2(sample,p,v,r,varid):
#    zev=DataArrayDouble.Meld([p_s,v_s,DataArrayDouble([r_s])])
#    return df(sample,zev,varid)

def jacob(sample,p,v,r,ff):
    zev=DataArrayDouble.Meld([p_s,v_s,DataArrayDouble([r_s])])
    return DataArrayDouble([df(sample,zev,i,ff) for i in range(7)],1,7)

def jacob0(sample,p,v,r):
    zev=DataArrayDouble.Meld([p_s,v_s,DataArrayDouble([r_s])])
    return DataArrayDouble([df0(sample,zev,i) for i in range(7)],1,7)

mm=MEDFileMesh.New("example2.med")
c=mm.getCoords()
p_s=DataArrayDouble(c.accumulate(),1,3)/float(len(c))
v_s=DataArrayDouble([1,0,0],1,3)
o=DataArrayDouble(c.getMinMaxPerComponent(),3,2)
o0,o1=o.explodeComponents()
o=o1-o0
o.abs()
r_s=o.getMaxValue()[0]/2.
#
r_s=0.215598
p_s=DataArrayDouble([0.,0.,0.],1,3)
v_s=DataArrayDouble([0.,0.,1.],1,3)
#
r_s=0.2
p_s=DataArrayDouble([1.,1.,1.],1,3)
v_s=DataArrayDouble([1.,1.,1.],1,3)
#probes=[0,979,1167,2467,2862,3706,3819]
probes=[1000]+[c[:,i].getMaxValue()[1] for i in range(3)]+[c[:,i].getMinValue()[1] for i in range(3)]

p=p_s ; v=v_s ; r=r_s
for ii in range(1):
    mat=DataArrayDouble.Aggregate([jacob(c[probes[0]],p,v,r,f0)]+[jacob(c[probe],p,v,r,f) for probe in probes[1:]])#+[DataArrayDouble([0,0,0,2*v[0,0],2*v[0,1],2*v[0,2],0],1,7)]
    y=DataArrayDouble([f0(c[probes[0]],p,v,r)]+[f(c[probe],p,v,r) for probe in probes[1:]])
    #y.pushBackSilent(v.magnitude()[0]-1.)
    delta2=y[:] ; delta2.abs()
    if delta2.getMaxValueInArray()<1e-5:
        print("finished")
        break;
    mat=scipy.matrix(mat.toNumPyArray())
    print ii,np.linalg.cond(mat)
    y=scipy.matrix(y.toNumPyArray())
    y=y.transpose()
    delta=np.linalg.solve(mat,-y)
    #delta=scipy.sparse.linalg.gmres(mat,-y)[0] # cg
    delta=DataArrayDouble(delta) ; delta.rearrange(7)
    #p+=delta[0,:3]
    #v+=delta[0,3:6]
    #v/=v.magnitude()[0]
    #r+=delta[0,6]
    print ii,delta.getValues()#,yy.transpose().tolist()

    pass

mat1=mat[[0,1,2,6]]
mat1=mat1.transpose()
mat1=mat1[[0,1,2,6]]
mat1=mat1.transpose()
mm=np.linalg.solve(mat1,y[[0,1,2,6]])
