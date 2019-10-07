#  -*- coding: iso-8859-1 -*-
# Copyright (C) 2018-2019  CEA/DEN, EDF R&D
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

import os
import time
import unittest

class TestParavis(unittest.TestCase):

    def setUp(self):
        import salome
        salome.salome_init()

    def processGuiEvents(self):
        import salome
        if salome.sg.hasDesktop():
            salome.sg.updateObjBrowser();
            import SalomePyQt
            SalomePyQt.SalomePyQt().processEvents()

    def test_paravis(self):
        """Quick test for Paravis module"""

        import salome
        if salome.hasDesktop(): # can test paravis in gui mode only

            print()
            print('Testing Paravis module')

            # ---- initialize Paravis
            print('... Initialize Paravis module (this can take some time)')
            self.processGuiEvents()
            import pvsimple as pvs
            self.processGuiEvents()

            # ---- get data dir
            data_dir = os.getenv('DATA_DIR', '')
            self.assertNotEqual(data_dir, '', 'DATA_DIR should be defined to load the test data')

            # ---- import med File
            print('... Load MED file')
            file_path = os.path.join(os.getenv('DATA_DIR'), 'MedFiles', 'ResOK_0000.med')
            med_reader = pvs.MEDReader(FileName=file_path)
            self.assertIsNotNone(med_reader)
            times = med_reader.TimestepValues.GetData()
            self.assertEqual(len(times), 2)
            self.processGuiEvents()

            # ---- get view
            print('... Get view')
            view = pvs.GetRenderView()
            self.assertIsNotNone(view)
            view.ResetCamera()
            self.processGuiEvents()

            # ---- create presentations
            print("... Display presentations")

            # ---- show mesh
            print('...... Show mesh')
            mesh = pvs.Show(med_reader, view)
            self.assertIsNotNone(mesh)
            view.ResetCamera()
            mesh.Representation = 'Surface With Edges'
            pvs.Render(view)
            time.sleep(1)
            self.processGuiEvents()

            # ---- show scalar map
            print('...... Show scalar map')
            scalar_map = mesh
            self.assertIsNotNone(scalar_map)
            scalar_map.Representation = 'Surface'
            pvs.ColorBy(scalar_map, ('POINTS', 'vitesse', 'Magnitude'))
            view.ViewTime = times[-1]
            pvs.Render(view)
            time.sleep(1)
            scalar_map.Visibility = 0
            pvs.Render(view)
            self.processGuiEvents()

            # ---- show vectors
            print('...... Show vectors')
            calc = pvs.Calculator(Input=med_reader)
            self.assertIsNotNone(calc)
            calc.ResultArrayName = 'vitesse_3c'
            calc.Function = 'iHat * vitesse_X + jHat * vitesse_Y + kHat * 0'
            glyph = pvs.Glyph(Input=calc, GlyphType='Arrow')
            self.assertIsNotNone(glyph)
            glyph.OrientationArray = ['POINTS', 'vitesse_3c']
            glyph.ScaleArray = ['POINTS', 'No scale array']
            glyph.ScaleFactor = 0.01
            vectors = pvs.Show(glyph, view)
            self.assertIsNotNone(vectors)
            vectors.Representation = 'Surface'
            pvs.Render(view)
            time.sleep(1)
            vectors.Visibility = 0
            pvs.Render(view)
            self.processGuiEvents()

            # ---- show iso surfaces
            print('...... Show iso surfaces')
            merge_blocks = pvs.MergeBlocks(Input=med_reader)
            self.assertIsNotNone(merge_blocks)
            calc = pvs.Calculator(Input=merge_blocks)
            self.assertIsNotNone(calc)
            calc.ResultArrayName = 'vitesse_magnitude'
            calc.Function = 'sqrt(vitesse_X^2+vitesse_Y^2)'
            data_range = med_reader.GetPointDataInformation()['vitesse'].GetComponentRange(-1)
            nb_surfaces = 10
            surfaces = [data_range[0] + i*(data_range[1]-data_range[0])/(nb_surfaces-1) for i in range(nb_surfaces)]
            contour = pvs.Contour(Input=calc)
            self.assertIsNotNone(contour)
            contour.ComputeScalars = 1
            contour.ContourBy = ['POINTS', 'vitesse_magnitude']
            contour.Isosurfaces = surfaces
            iso_surfaces = pvs.Show(contour, view)
            self.assertIsNotNone(iso_surfaces)
            iso_surfaces.Representation = 'Surface'
            pvs.ColorBy(iso_surfaces, ('POINTS', 'vitesse', 'Magnitude'))
            pvs.Render(view)
            time.sleep(1)
            iso_surfaces.Visibility = 0
            pvs.Render(view)
            self.processGuiEvents()

            # ---- show cut planes
            print('...... Show cut planes')
            slice = pvs.Slice(Input=med_reader)
            self.assertIsNotNone(slice)
            slice.SliceType = "Plane"
            slice.SliceType.Normal = [1.0, 0.0, 0.0]
            bounds = med_reader.GetDataInformation().GetBounds()
            nb_planes = 30
            displacement = 0.5
            b_left = bounds[0] + (bounds[1]-bounds[0])*displacement/100
            b_right = bounds[1] - (bounds[1]-bounds[0])*displacement/100
            b_range = b_right - b_left
            positions = [b_left + i*b_range/(nb_planes-1) for i in range(nb_planes)]
            slice.SliceOffsetValues = positions
            pvs.Hide3DWidgets(proxy=slice.SliceType)
            cut_planes = pvs.Show(slice, view)
            self.assertIsNotNone(cut_planes)
            cut_planes.Representation = 'Surface'
            pvs.ColorBy(cut_planes, ('POINTS', 'vitesse', 'Magnitude'))
            pvs.Render(view)
            time.sleep(1)
            cut_planes.Visibility = 0
            pvs.Render(view)
            self.processGuiEvents()

            # ---- show deformed shape
            print('...... Show deformed shape')
            merge_blocks = pvs.MergeBlocks(Input=med_reader)
            self.assertIsNotNone(merge_blocks)
            calc = pvs.Calculator(Input=merge_blocks)
            self.assertIsNotNone(calc)
            calc.ResultArrayName = 'vitesse_3c'
            calc.Function = 'iHat * vitesse_X + jHat * vitesse_Y + kHat * 0'
            warp = pvs.WarpByVector(Input=calc)
            self.assertIsNotNone(warp)
            warp.Vectors = ['POINTS', 'vitesse_3c']
            warp.ScaleFactor = 0.5
            deformed_shape = pvs.Show(warp, view)
            self.assertIsNotNone(deformed_shape)
            deformed_shape.Representation = 'Surface'
            pvs.ColorBy(deformed_shape, ('CELLS', 'pression'))
            pvs.Render(view)
            time.sleep(1)
            deformed_shape.Visibility = 0
            pvs.Render(view)
            self.processGuiEvents()

        else: # not in gui mode, Paravis can not be tested
            print()
            print("PARAVIS module requires SALOME to be running in GUI mode.")
            print()
            print("Skipping test for PARAVIS...")
            self.processGuiEvents()

if __name__ == '__main__':
    unittest.main()
