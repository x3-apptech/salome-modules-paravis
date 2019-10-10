# -*- coding: iso-8859-1 -*-
# Script de visualisation rosette et contrainte normale/tangeantielle

# Donnees utilisateur
#
# MEDFILE : Fichier de resultat MED
# MAILLAGE : Nom du maillage
# GROUPE_FACE : Liste  groupes de face de post-traitement
# COEFF_T1T2 : facteur scale rosette
# COEFF_TN : facteur scale normale/cisaillement
#
# T1 : Nom du champ SIRO_ELEM_T1
# T2 : Nom du champ SIRO_ELEM_T2
# T  : Nom du champ SIRO_ELEM_SIGT
# N  : Nom du champ SIRO_ELEM_SIGN
#
# ROSETTE : True/False

from paraview.simple import *

import os
dataRoot = os.getenv('PARAVIEW_DATA_ROOT', "")
MEDFILE = dataRoot+'/rosette_am.med'
MAILLAGE='mail'
GROUPE_FACE = ['SURF_AM',]
COEFF_T1T2 = 0.00003
COEFF_TN = 0.003

T='RESUNL__SIRO_ELEM_TANGENT_Vector'
N='RESUNL__SIRO_ELEM_NORMAL_Vector'
T1 = 'RESUNL__SIRO_ELEM_T1_Vector'
T2 = 'RESUNL__SIRO_ELEM_T2_Vector'

ROSETTE = True

# Fin données utilisateurs



#%==================== Importing MED file====================%

print ("**** Importing MED file")

myResult0 = MEDReader(FileName=MEDFILE)
if myResult0 is None : raise "Erreur de fichier MED"

# Imposition GenerateVectors à faire
myResult0.GenerateVectors=1

NB_ORDRE = myResult0.GetPropertyValue('TimestepValues')

#extraction groupes et visualisation du maillage
listeg=[]
for g in GROUPE_FACE :
    listeg.append('GRP_'+g)

myResult = ExtractGroup()
myResult.AllGroups=listeg
#myResult.Groups = listeg


CellCenters1 = CellCenters()
if len(NB_ORDRE) > 1 :
    animation = GetAnimationScene()
    #animation.AnimationTime = NB_ORDRE[2]
view2 = GetRenderView()


#%====================Displaying rosette====================%

if ROSETTE : 
     CMAX = 0.0
     T_MAX = 0.0
     print ("****  Displaying rosette")

     view2.Background=[1.0,1.0,1.0]
     SetActiveSource(myResult)
     Display0 = Show()
     Display0.Texture=[]
     Display0.Representation = 'Wireframe'
     Display0.AmbientColor=[0.0,0.0,0.0]
 
     SetActiveSource(CellCenters1)
     Glyph1 = Glyph()
     Glyph1.OrientationArray = ['POINTS', T1]
     Glyph1.GlyphType ='Line'
     Glyph1.GlyphMode = 'All Points'

     pd1 = Glyph1.PointData

     for i in range(len(pd1)):
           if pd1.values()[i-1].GetName()== T1 :
                    NUM = i-1
                    NB_CMP = pd1.values()[i-1].GetNumberOfComponents()
                    NOM_CMP = pd1.values()[i-1].GetComponentName(NB_CMP)
                    RANGE_CMP = pd1.values()[i-1].GetRange(NB_CMP-1)
                    MAX_CMP = max(abs(RANGE_CMP[0]),abs(RANGE_CMP[1]))
                    if RANGE_CMP[0] < 0.0 :
                         C_MAX = -RANGE_CMP[0]
                    else :
                         C_MAX = 0.
                    if RANGE_CMP[1] > 0.0 :
                         T_MAX = RANGE_CMP[1]
                    else :
                         T_MAX = 0.
 
     Render()

     Glyph1.ScaleArray = ['POINTS', T1]
     Glyph1.VectorScaleMode = 'Scale by Magnitude'
     Glyph1.ScaleFactor = COEFF_T1T2

     Display1=Show()

     PVLookupTable1 = GetLookupTableForArray( T1, NB_CMP, VectorMode = 'Component',VectorComponent=NB_CMP-1, NumberOfTableValues=2, RGBPoints=[-MAX_CMP, 0.0, 0.0, 1.0, MAX_CMP, 1.0, 0.0, 0.0], AutomaticRescaleRangeMode=-1, ScalarRangeInitialized = 1.0)
     
     scalarbar1 = CreateScalarBar(Title =T1, ComponentTitle = NOM_CMP, LookupTable=PVLookupTable1, TitleFontSize=12 , LabelFontSize=12, TitleColor=[0.0,0.0,0.0],LabelColor=[0.0,0.0,0.0])

     view2.Representations.append(scalarbar1)
     Display1.ColorArrayName = T1
     Display1.LookupTable = PVLookupTable1
     Display1.Representation = 'Surface'

     SetActiveSource(CellCenters1)
     Glyph2 = Glyph()
     Glyph2.OrientationArray = ['POINTS', T2]
     Glyph2.GlyphType ='Line'
     Glyph2.ScaleArray = ['POINTS', T2]
     Glyph2.VectorScaleMode = 'Scale by Magnitude'
     Glyph2.ScaleFactor = COEFF_T1T2

     Glyph2.GlyphMode = 'All Points'

     pd2 = Glyph2.PointData

     for i in range(len(pd2)):
            if pd2.values()[i-1].GetName()== T2 :
                    NB_CMP = pd2.values()[i-1].GetNumberOfComponents()
                    NOM_CMP = pd2.values()[i-1].GetComponentName(NB_CMP-1)
                    RANGE_CMP = pd2.values()[i-1].GetRange(NB_CMP-1)
                    MAX_CMP = max(abs(RANGE_CMP[0]),abs(RANGE_CMP[1]))
                    if RANGE_CMP[0] < 0.0 :
                         C_MAX = max(C_MAX,-RANGE_CMP[0])
                    if RANGE_CMP[1] > 0.0 : 
                         T_MAX = max(T_MAX,RANGE_CMP[1])

     Display2=Show()

     PVLookupTable2 = GetLookupTableForArray( T2, NB_CMP, VectorMode = 'Component',VectorComponent=NB_CMP-1, NumberOfTableValues=2, RGBPoints=[-MAX_CMP, 0.0, 0.0, 1.0, MAX_CMP, 1.0, 0.0, 0.0], AutomaticRescaleRangeMode=-1)
     scalarbar2 = CreateScalarBar(Title =T2, ComponentTitle = NOM_CMP, LookupTable=PVLookupTable2, TitleFontSize=12 , LabelFontSize=12,TitleColor=[0.0,0.0,0.0],LabelColor=[0.0,0.0,0.0])
     #view2.Representations.append(scalarbar2)
     Display2.ColorArrayName = T2
     Display2.LookupTable = PVLookupTable2
     Display2.Representation = 'Surface'

     Text2 = Text()
     Texte = 'COMPRESSION MAX = '+str(C_MAX)+' MPa \nTRACTION MAX = '+str(T_MAX)+' Mpa'
     Text2.Text = Texte
     DisplayT = Show()
     DisplayT.FontSize = 12
     DisplayT.Color=[0.0, 0.0, 0.0]

     Render()
     view2.CameraPosition = [-524.450475200065, -34.2470779418945, 572.968383789062]
     view2.CameraFocalPoint = [9.7492516040802, -34.2470779418945, 572.968383789062]
     view2.CameraViewUp = [0.0, 0.0, 1.0]
     ResetCamera()

     import sys
     try:
       baselineIndex = sys.argv.index('-B')+1
       baselinePath = sys.argv[baselineIndex]
     except:
       print ("Could not get baseline directory. Test failed.")
     baseline_file = os.path.join(baselinePath, "TestDataAnalysis.png")
     from paraview.vtk.test import Testing
     from paraview.vtk.util.misc import vtkGetTempDir
     Testing.VTK_TEMP_DIR = vtkGetTempDir()
     Testing.compareImage(view2.GetRenderWindow(), baseline_file, threshold=10)
     Testing.interact()
