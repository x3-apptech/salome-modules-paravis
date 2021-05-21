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
# Author : Yoann Audouin (EDF R&D)

#### import the simple module from the paraview
from paraview.simple import *
from vtk.util import numpy_support
import numpy as np

from medcoupling import *
from MEDLoader import *

#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

def MyAssert(clue):
    if not clue:
        raise RuntimeError("Assertion failed !")


def test_VoroGauss(result, ref, field_name):
    """
    Test result of VoroGauss filter
    """
    ds0 = servermanager.Fetch(result)
    block = ds0.GetBlock(0)

    data = numpy_support.vtk_to_numpy(block.GetCellData().GetArray(field_name))

    MyAssert(np.allclose(data, ref))

###
# Test of testMEDReader14
###
# create a new 'MED Reader'
testMEDReader14med = MEDReader(registrationName='testMEDReader14.med',
                               FileName='testMEDReader14.med')
testMEDReader14med.AllTimeSteps = ['0000', '0001', '0002', '0003', '0004']

fields = [(['zeField0'],
           ['TS0/Mesh/ComSup0/zeField0_MM0@@][@@GAUSS'],
           [[[100.,101.], [102.,103.], [104.,105.], [106.,107.], [108.,109.], [110.,111.], [112.,113.], [114.,115.], [  0.,  1.], [  2.,  3.], [  4.,  5.], [  6.,  7.], [  8.,  9.], [ 10., 11.], [ 12., 13.], [ 14., 15.], [ 16., 17.], [ 18., 19.], [ 20., 21.], [ 22., 23.], [ 24., 25.], [ 26., 27.], [ 28., 29.], [ 30., 31.], [ 32., 33.], [ 34., 35.], [ 36., 37.], [ 38., 39.], [ 40., 41.], [ 42., 43.], [ 44., 45.], [ 46., 47.], [ 48., 49.], [ 50., 51.], [ 52., 53.], [ 54., 55.], [ 56., 57.], [ 58., 59.], [ 60., 61.], [ 62., 63.], [ 64., 65.], [ 66., 67.], [ 68., 69.], [ 70., 71.], [ 72., 73.], [ 74., 75.], [ 76., 77.], [ 78., 79.], [ 80., 81.], [ 82., 83.], [ 84., 85.], [ 86., 87.], [ 88., 89.], [ 90., 91.], [ 92., 93.], [ 94., 95.], [ 96., 97.], [ 98., 99.], [100.,101.], [102.,103.], [104.,105.], [106.,107.], [108.,109.], [110.,111.], [112.,113.], [114.,115.], [116.,117.], [118.,119.], [120.,121.], [122.,123.], [124.,125.], [126.,127.], [128.,129.], [130.,131.], [132.,133.], [134.,135.], [136.,137.], [138.,139.], [140.,141.], [142.,143.], [144.,145.], [146.,147.], [148.,149.], [150.,151.], [152.,153.], [154.,155.], [156.,157.], [158.,159.], [160.,161.], [162.,163.], [164.,165.], [166.,167.]]]
          ),
          (['zeField0'],
           ['TS0/Mesh/ComSup1/zeField0_MM1@@][@@GAUSS'],
           [[[116.,117.], [118.,119.], [120.,121.], [122.,123.], [124.,125.], [126.,127.], [128.,129.], [130.,131.], [132.,133.], [134.,135.], [136.,137.], [138.,139.], [140.,141.], [142.,143.], [144.,145.], [146.,147.], [148.,149.], [150.,151.]]]
          ),
          (['zeField0', 'zeField1'],
           ['TS0/Mesh/ComSup2/zeField0_MM2@@][@@GAUSS',
            'TS0/Mesh/ComSup2/zeField1_MM0@@][@@GAUSS'],
           [[[152.,153.], [154.,155.], [156.,157.], [158.,159.], [160.,161.], [162.,163.], [164.,165.], [166.,167.], [168.,169.], [170.,171.], [172.,173.], [174.,175.], [176.,177.], [178.,179.], [180.,181.], [182.,183.]],
            [[500.,501.], [502.,503.], [504.,505.], [506.,507.], [508.,509.], [510.,511.], [512.,513.], [514.,515.], [516.,517.], [518.,519.], [520.,521.], [522.,523.], [524.,525.], [526.,527.], [528.,529.], [530.,531.]]]
          ),
          (['zeField1'],
           ['TS0/Mesh/ComSup3/zeField1_MM1@@][@@GAUSS'],
           [[[532.,533.], [534.,535.], [536.,537.], [538.,539.], [540.,541.], [542.,543.], [544.,545.], [546.,547.], [548.,549.], [550.,551.], [552.,553.], [554.,555.], [556.,557.], [558.,559.], [560.,561.], [562.,563.], [564.,565.], [566.,567.], [568.,569.], [570.,571.], [572.,573.], [574.,575.], [576.,577.], [578.,579.], [580.,581.], [582.,583.], [584.,585.], [586.,587.], [588.,589.], [590.,591.]]]
          )
         ]

for field_names, field, refs in fields:
    testMEDReader14med.AllArrays = field
    testMEDReader14med.UpdatePipeline()

    # create a new 'ELGA field To Surface'
    eLGAfieldToSurface1 = ELGAfieldToSurface(registrationName='ELGAfieldToSurface1',
                                             Input=testMEDReader14med)
    eLGAfieldToSurface1.UpdatePipeline()
    for field_name, ref in zip(field_names, refs):
        print("Testing  for leaf {} and field {}".format(field, field_name))
        test_VoroGauss(eLGAfieldToSurface1, np.array(ref), field_name)


###
# Test of PG_3D.med
###
# create a new 'MED Reader'
pG_3Dmed = MEDReader(registrationName='PG_3D.med', FileName='PG_3D.med')
pG_3Dmed.AllArrays = ['TS0/Extruded/ComSup0/Extruded@@][@@P0',
                      'TS0/Extruded/ComSup0/MyFieldPG@@][@@GAUSS']
pG_3Dmed.UpdatePipeline()

fields = [('Extruded',
           [ 0., 0., 0., 0., 0., 0., 0., 1., 1., 1., 1., 1., 1., 1., 2., 2., 2., 2., 2., 2., 2., 3., 3., 3., 3., 3., 3., 3., 4., 4., 4., 4., 4., 4., 4., 5., 5., 5., 5., 5., 5., 5., 6., 6., 6., 6., 6., 6., 6., 7., 7., 7., 7., 7., 7., 7., 8., 8., 8., 8., 8., 8., 8., 9., 9., 9., 9., 9., 9., 9.,10.,10., 10.,10.,10.,10.,10.,11.,11.,11.,11.,11.,11.,11.,12.,12.,12.,12.,12.,12., 12.,13.,13.,13.,13.,13.,13.,13.,14.,14.,14.,14.,14.,14.,14.,15.,15.,15., 15.,15.,15.,15.,16.,16.,16.,16.,16.,16.,16.,17.,17.,17.,17.,17.,17.,17., 18.,18.,18.,18.,18.,18.,18.,19.,19.,19.,19.,19.,19.,19.,20.,20.,20.,20., 20.,20.,20.,21.,21.,21.,21.,21.,21.,21.,22.,22.,22.,22.,22.,22.,22.,23., 23.,23.,23.,23.,23.,23.,24.,24.,24.,24.,24.,24.,24.,25.,25.,25.,25.,25., 25.,25.,26.,26.,26.,26.,26.,26.,26.,27.,27.,27.,27.,27.,27.,27.,28.,28., 28.,28.,28.,28.,28.,29.,29.,29.,29.,29.,29.,29.,30.,30.,30.,30.,30.,30., 30.,31.,31.,31.,31.,31.,31.,31.,32.,32.,32.,32.,32.,32.,32.,33.,33.,33., 33.,33.,33.,33.,34.,34.,34.,34.,34.,34.,34.,35.,35.,35.,35.,35.,35.,35., 36.,36.,36.,36.,36.,36.,36.,37.,37.,37.,37.,37.,37.,37.,38.,38.,38.,38., 38.,38.,38.,39.,39.,39.,39.,39.,39.,39.,40.,40.,40.,40.,40.,40.,40.,41., 41.,41.,41.,41.,41.,41.,42.,42.,42.,42.,42.,42.,42.,43.,43.,43.,43.,43., 43.,43.,44.,44.,44.,44.,44.,44.,44.,45.,45.,45.,45.,45.,45.,45.,46.,46., 46.,46.,46.,46.,46.,47.,47.,47.,47.,47.,47.,47.,48.,48.,48.,48.,48.,48., 48.,49.,49.,49.,49.,49.,49.,49.,50.,50.,50.,50.,50.,50.,50.,51.,51.,51., 51.,51.,51.,51.,52.,52.,52.,52.,52.,52.,52.,53.,53.,53.,53.,53.,53.,53., 54.,54.,54.,54.,54.,54.,54.,55.,55.,55.,55.,55.,55.,55.,56.,56.,56.,56., 56.,56.,56.,57.,57.,57.,57.,57.,57.,57.,58.,58.,58.,58.,58.,58.,58.,59., 59.,59.,59.,59.,59.,59.,60.,60.,60.,60.,60.,60.,60.,61.,61.,61.,61.,61., 61.,61.,62.,62.,62.,62.,62.,62.,62.,63.,63.,63.,63.,63.,63.,63.]
          ),
          ('MyFieldPG',
           [  0.,   1.,   2.,   3.,   4.,   5.,   6.,   7.,   8.,   9.,  10.,  11.,  12.,  13.,  14.,  15.,  16.,  17.,  18.,  19.,  20.,  21.,  22.,  23.,  24.,  25.,  26.,  27.,  28.,  29.,  30.,  31.,  32.,  33.,  34.,  35.,  36.,  37.,  38.,  39.,  40.,  41.,  42.,  43.,  44.,  45.,  46.,  47.,  48.,  49.,  50.,  51.,  52.,  53.,  54.,  55.,  56.,  57.,  58.,  59.,  60.,  61.,  62.,  63.,  64.,  65.,  66.,  67.,  68.,  69.,  70.,  71.,  72.,  73.,  74.,  75.,  76.,  77.,  78.,  79.,  80.,  81.,  82.,  83.,  84.,  85.,  86.,  87.,  88.,  89.,  90.,  91.,  92.,  93.,  94.,  95.,  96.,  97.,  98.,  99., 100., 101., 102., 103., 104., 105., 106., 107., 108., 109., 110., 111.,  112., 113., 114., 115., 116., 117., 118., 119., 120., 121., 122., 123., 124., 125.,  126., 127., 128., 129., 130., 131., 132., 133., 134., 135., 136., 137., 138., 139.,  140., 141., 142., 143., 144., 145., 146., 147., 148., 149., 150., 151., 152., 153.,  154., 155., 156., 157., 158., 159., 160., 161., 162., 163., 164., 165., 166., 167.,  168., 169., 170., 171., 172., 173., 174., 175., 176., 177., 178., 179., 180., 181.,  182., 183., 184., 185., 186., 187., 188., 189., 190., 191., 192., 193., 194., 195.,  196., 197., 198., 199., 200., 201., 202., 203., 204., 205., 206., 207., 208., 209.,  210., 211., 212., 213., 214., 215., 216., 217., 218., 219., 220., 221., 222., 223.,  224., 225., 226., 227., 228., 229., 230., 231., 232., 233., 234., 235., 236., 237.,  238., 239., 240., 241., 242., 243., 244., 245., 246., 247., 248., 249., 250., 251.,  252., 253., 254., 255., 256., 257., 258., 259., 260., 261., 262., 263., 264., 265.,  266., 267., 268., 269., 270., 271., 272., 273., 274., 275., 276., 277., 278., 279.,  280., 281., 282., 283., 284., 285., 286., 287., 288., 289., 290., 291., 292., 293.,  294., 295., 296., 297., 298., 299., 300., 301., 302., 303., 304., 305., 306., 307.,  308., 309., 310., 311., 312., 313., 314., 315., 316., 317., 318., 319., 320., 321.,  322., 323., 324., 325., 326., 327., 328., 329., 330., 331., 332., 333., 334., 335.,  336., 337., 338., 339., 340., 341., 342., 343., 344., 345., 346., 347., 348., 349.,  350., 351., 352., 353., 354., 355., 356., 357., 358., 359., 360., 361., 362., 363.,  364., 365., 366., 367., 368., 369., 370., 371., 372., 373., 374., 375., 376., 377.,  378., 379., 380., 381., 382., 383., 384., 385., 386., 387., 388., 389., 390., 391.,  392., 393., 394., 395., 396., 397., 398., 399., 400., 401., 402., 403., 404., 405.,  406., 407., 408., 409., 410., 411., 412., 413., 414., 415., 416., 417., 418., 419.,  420., 421., 422., 423., 424., 425., 426., 427., 428., 429., 430., 431., 432., 433.,  434., 435., 436., 437., 438., 439., 440., 441., 442., 443., 444., 445., 446., 447.]
          ),
         ]

# create a new 'ELGA field To Surface'
eLGAfieldToSurface2 = ELGAfieldToSurface(registrationName='ELGAfieldToSurface2', Input=pG_3Dmed)
eLGAfieldToSurface2.UpdatePipeline()

for field_name, ref in fields:
    print("Testing PG_3D.med for field {}".format(field_name))
    test_VoroGauss(eLGAfieldToSurface2, np.array(ref), field_name)

###
# Test of VoroGauss1.med
###

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

# create a new 'MED Reader'
voroGauss1med = MEDReader(registrationName='VoroGauss1.med', FileName='VoroGauss1.med')
voroGauss1med.AllArrays = ['TS0/mesh/ComSup0/MyFieldCell@@][@@P0',
                           'TS0/mesh/ComSup0/MyFieldPG@@][@@GAUSS']
voroGauss1med.AllTimeSteps = ['0000']
voroGauss1med.UpdatePipeline()

# create a new 'ELGA field To Surface'
eLGAfieldToSurface3 = ELGAfieldToSurface(registrationName='ELGAfieldToSurface3', Input=voroGauss1med)
eLGAfieldToSurface3.UpdatePipeline()

fields = [('MyFieldCell',
           [[0., 0.], [0., 0.], [0., 0.], [1., 1.], [1., 1.], [1., 1.], [2., 2.], [2., 2.], [2., 2.], [3., 3.], [3., 3.], [3., 3.], [4., 4.], [4., 4.], [4., 4.], [5., 5.], [5., 5.], [5., 5.], [5., 5.], [6., 6.], [6., 6.], [6., 6.], [6., 6.]]
          ),
          ('MyFieldPG',
           [[ 0.,  0.], [ 1.,  1.], [ 2.,  2.], [ 3.,  3.], [ 4.,  4.], [ 5.,  5.], [ 6.,  6.], [ 7.,  7.], [ 8.,  8.], [ 9.,  9.], [10., 10.], [11., 11.], [12., 12.], [13., 13.], [14., 14.], [15., 15.], [16., 16.], [17., 17.], [18., 18.], [19., 19.], [20., 20.], [21., 21.], [22., 22.]]
          )
         ]

for field_name, ref in fields:
    print("Testing VoroGauss.med for field {}".format(field_name))
    test_VoroGauss(eLGAfieldToSurface3, np.array(ref), field_name)

###
# Test for hexa element
###

#[     1 ] :        185        189        205        201        186        190        206        202
file_name = "simple_mesh.med"
coords = [0.024000, 0.024000, 1.200000, #185:0
          0.024000, 0.048000, 1.200000, #186:1
          0.048000, 0.024000, 1.200000, #189:2
          0.048000, 0.048000, 1.200000, #190:3
          0.024000, 0.024000, 1.600000, #201:4
          0.024000, 0.048000, 1.600000, #202:5
          0.048000, 0.024000, 1.600000, #205:6
          0.048000, 0.048000, 1.600000, #206:7
          ]

conn = [0,2,6,4,1,3,7,5]


mesh=MEDCouplingUMesh("MESH", 3)

mesh.allocateCells(1)
mesh.insertNextCell(NORM_HEXA8, conn)

mesh.finishInsertingCells()

coords_array = DataArrayDouble(coords, 8, 3)
mesh.setCoords(coords_array)
mesh.checkConsistencyLight()
WriteMesh(file_name, mesh, True)

fieldGauss=MEDCouplingFieldDouble.New(ON_GAUSS_PT,ONE_TIME);
fieldGauss.setMesh(mesh);
fieldGauss.setName("RESU____EPSI_NOEU");
fieldGauss.setTimeUnit("s")
fieldGauss.setTime(0.0,1,-1)


hexa8CooGauss = [-0.577350, -0.577350, -0.577350,
               -0.577350, -0.577350, +0.577350,
               -0.577350, +0.577350, -0.577350,
               -0.577350, +0.577350, +0.577350,
               +0.577350, -0.577350, -0.577350,
               +0.577350, -0.577350, +0.577350,
               +0.577350, +0.577350, -0.577350,
               +0.577350, +0.577350, +0.577350
              ]
hexa8CooRef = [ -1.000000, -1.000000, -1.000000,
                -1.000000, +1.000000, -1.000000,
                +1.000000, +1.000000, -1.000000,
                +1.000000, -1.000000, -1.000000,
                -1.000000, -1.000000, +1.000000,
                -1.000000, +1.000000, +1.000000,
                +1.000000, +1.000000, +1.000000,
                +1.000000, -1.000000, +1.000000
              ]
wg8 = [1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0];
fieldGauss.setGaussLocalizationOnType(NORM_HEXA8,hexa8CooRef,hexa8CooGauss,wg8);

nbTuples=8
array=DataArrayDouble.New();
values=[
       -5.65022e-05,-5.6733e-05 , 0.000179768,-3.56665e-07,-7.22919e-07,-0.00020561,
        5.65022e-05, 5.6733e-05 ,-0.000179768,-3.56665e-07, 7.22919e-07,-0.00020561,
       -5.63787e-05,-5.65226e-05, 0.000179703,-1.43488e-07,-2.4061e-07 ,-0.000207459,
        5.63787e-05, 5.65226e-05,-0.000179703,-1.43488e-07, 2.4061e-07 ,-0.000207459,
       -2.49762e-05,-2.40288e-05, 9.05803e-05, 2.53861e-06, 2.20988e-07,-0.000207793,
        2.49762e-05, 2.40288e-05,-9.05803e-05, 2.53861e-06,-2.20988e-07,-0.000207793,
       -2.49587e-05,-2.41105e-05, 9.05832e-05, 9.76774e-07, 3.27621e-08,-0.000210835,
        2.49587e-05, 2.41105e-05,-9.05832e-05, 9.76774e-07,-3.27621e-08,-0.000210835
       ]
array.setValues(values, nbTuples, 6);
array.setInfoOnComponents(["EPXX", "EPYY", "EPZZ", "EPXY", "EPXZ", "EPYZ"])
fieldGauss.setArray(array);
fieldGauss.checkConsistencyLight();

WriteFieldUsingAlreadyWrittenMesh(file_name, fieldGauss)


fieldGauss2=MEDCouplingFieldDouble.New(ON_GAUSS_PT,ONE_TIME);
fieldGauss2.setMesh(mesh);
fieldGauss2.setName("RESU____SIGM_NOEU");
fieldGauss2.setTimeUnit("s")
fieldGauss2.setTime(0.0,1,-1)

fieldGauss2.setGaussLocalizationOnType(NORM_HEXA8,hexa8CooRef,hexa8CooGauss,wg8);

nbTuples=8
array=DataArrayDouble.New();
values=[
        -1.06652e+06, -1.1038e+06 , 3.71003e+07 , -57615.2, -116779 , -3.3214e+07,
        1.06652e+06 , 1.1038e+06  , -3.71003e+07, -57615.2, 116779  , -3.3214e+07,
        -1.01401e+06, -1.03727e+06, 3.71223e+07 , -23178.9, -38867.8, -3.35127e+07,
        1.01401e+06 , 1.03727e+06 , -3.71223e+07, -23178.9, 38867.8 , -3.35127e+07,
        1.00241e+06 , 1.15545e+06 , 1.96692e+07 , 410084  , 35698.1 , -3.35666e+07,
        -1.00241e+06, -1.15545e+06, -1.96692e+07, 410084  , -35698.1, -3.35666e+07,
        997783      , 1.1348e+06  , 1.96622e+07 , 157787  , 5292.34 , -3.40579e+07,
        -997783     , -1.1348e+06 , -1.96622e+07, 157787  , -5292.34, -3.40579e+07
       ]
array.setValues(values, nbTuples, 6);
array.setInfoOnComponents(["SIXX", "SIYY", "SIZZ", "SIXY", "SIXZ", "SIYZ"])
fieldGauss2.setArray(array);
fieldGauss2.checkConsistencyLight();

WriteFieldUsingAlreadyWrittenMesh(file_name, fieldGauss2)

# create a new 'MED Reader'
simple_meshmed = MEDReader(registrationName='simple_mesh.med', FileName='simple_mesh.med')
simple_meshmed.AllArrays = ['TS0/MESH/ComSup0/RESU____EPSI_NOEU@@][@@GAUSS',
                            'TS0/MESH/ComSup0/RESU____SIGM_NOEU@@][@@GAUSS']
simple_meshmed.AllTimeSteps = ['0000']
simple_meshmed.UpdatePipeline()

# create a new 'ELGA field To Surface'
eLGAfieldToSurface4 = ELGAfieldToSurface(registrationName='ELGAfieldToSurface4', Input=simple_meshmed)
eLGAfieldToSurface4.UpdatePipeline()

fields = [('RESU____EPSI_NOEU',
           [
            [-5.65022e-05,-5.6733e-05 , 0.000179768 ,  -3.56665e-07,   -7.22919e-07, -0.00020561],
            [ 5.65022e-05, 5.6733e-05 , -0.000179768,  -3.56665e-07,   7.22919e-07 , -0.00020561],
            [-5.63787e-05,-5.65226e-05, 0.000179703 ,  -1.43488e-07,   -2.4061e-07 , -0.000207459],
            [ 5.63787e-05, 5.65226e-05, -0.000179703,  -1.43488e-07,   2.4061e-07  , -0.000207459],
            [-2.49762e-05,-2.40288e-05, 9.05803e-05 ,  2.53861e-06 ,   2.20988e-07 , -0.000207793],
            [ 2.49762e-05, 2.40288e-05, -9.05803e-05,  2.53861e-06 ,   -2.20988e-07, -0.000207793],
            [-2.49587e-05,-2.41105e-05, 9.05832e-05 ,  9.76774e-07 ,   3.27621e-08 , -0.000210835],
            [ 2.49587e-05, 2.41105e-05, -9.05832e-05,  9.76774e-07 ,   -3.27621e-08, -0.000210835],
           ]),
          ('RESU____SIGM_NOEU',
           [
            [-1.06652e+06, -1.1038e+06 , 3.71003e+07 ,  -57615.2, -116779 , -3.3214e+07],
            [1.06652e+06 , 1.1038e+06  , -3.71003e+07,  -57615.2, 116779  , -3.3214e+07],
            [-1.01401e+06, -1.03727e+06, 3.71223e+07 ,  -23178.9, -38867.8, -3.35127e+07],
            [1.01401e+06 , 1.03727e+06 , -3.71223e+07,  -23178.9, 38867.8 , -3.35127e+07],
            [1.00241e+06 , 1.15545e+06 , 1.96692e+07 ,  410084  , 35698.1 , -3.35666e+07],
            [-1.00241e+06, -1.15545e+06, -1.96692e+07,  410084  , -35698.1, -3.35666e+07],
            [997783      , 1.1348e+06  , 1.96622e+07 ,  157787  , 5292.34 , -3.40579e+07],
            [-997783     , -1.1348e+06 , -1.96622e+07,  157787  , -5292.34, -3.40579e+07],

           ])
         ]

for field_name, ref in fields:
    print("Testing simple_mesh.med for field {}".format(field_name))
    test_VoroGauss(eLGAfieldToSurface4, np.array(ref), field_name)
