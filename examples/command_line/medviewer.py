# Copyright (C) 2014-2020  CEA/DEN, EDF R&D
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
class MEDViewer():

  def __init__(self, createRenderView=True, interactive=False):
    self.interactive = interactive
    import pvsimple

    if createRenderView:
      self.renderView = pvsimple.CreateRenderView()
    else:
      self.renderView = pvsimple.GetRenderView()
  #

  def __displayInteractive(self, element):
    from PyQt4 import QtCore, QtGui
    from vtk.qt4 import QVTKRenderWindowInteractor

    class __InteractiveViewer(QtCore.QObject):
      def __init__(self, viewer):
        self.renderView = viewer.renderView
        self.widget = QVTKRenderWindowInteractor.QVTKRenderWindowInteractor(\
          rw=self.renderView.GetRenderWindow(),
          iren=self.renderView.GetInteractor())
        self.widget.Initialize()
        self.widget.Start()
        self.widget.show()
        import pvsimple
        pvsimple.ResetCamera()
        pvsimple.Render(view=self.renderView)
      #
      def render(self):
        pvsimple.Render(self.renderView)
      #
    #
    app = QtGui.QApplication(['ParaView Python App'])
    iv = __InteractiveViewer(self)
    app.exec_()
  #

  def __display(self, element, pause=False):
    import pvsimple
    pvsimple.SetActiveSource(element)
    pvsimple.Show(view=self.renderView)

    if self.interactive:
      self.__displayInteractive(element)
    else:
      pvsimple.Render(view=self.renderView)
      pvsimple.ResetCamera()
      pvsimple.SetActiveSource(element)
      pvsimple.Render(view=self.renderView)
      if pause:
        input("Press Enter key to continue")
  #

  def display_mesh(self, element, pause=False):
    self.__display(element, pause)
  #

  def display_field(self, element, pause=False):
    import pvsimple
    pvsimple.SetActiveSource(element)
    representation = pvsimple.Show(view=self.renderView)

    from medio import get_element_type, get_element_name
    etype = get_element_type(element)
    ename = get_element_name(element)

    if etype == 'P0':
      representation.ColorArrayName = ("CELLS", ename)
      data = element.CellData
    elif etype == 'P1':
      representation.ColorArrayName = ("POINTS", ename)
      data = element.PointData

    # :TODO: Determine nb components
    nb_cmp = 1
    mode = "Magnitude" # "Component" if nb_cmp > 1
    scalarbar = self.__build_scalar_bar(data, ename, nb_cmp, mode)

    pvsimple.SetActiveSource(element)
    self.renderView.Representations.append(scalarbar)
    self.__display(element, pause)
  #

  def __build_scalar_bar(self, data, fieldname, nb_components, vector_mode):
    import pvsimple
    # Get data range (mini/maxi)
    for n in range(data.GetNumberOfArrays()):
        if data.GetArray(n).GetName() == fieldname:
            mini,maxi = data.GetArray(n).GetRange()

    stepvalue = (maxi-mini)/100. # 100 steps

    # Build Lookup table
    RGBPoints = [mini, 0.0, 0.0, 1.0, maxi, 1.0, 0.0, 0.0]
    nb = int((maxi-mini)/stepvalue)-1
    Table = pvsimple.GetLookupTableForArray("", nb_components, VectorMode=vector_mode, ColorSpace='HSV')
    Table.Discretize = 1
    Table.NumberOfTableValues = nb
    Table.RGBPoints = RGBPoints

    representation = pvsimple.Show(view=self.renderView)
    representation.Representation = 'Surface'
    representation.LookupTable = Table

    # Build scalar bar
    scalarbar = pvsimple.CreateScalarBar(LabelFormat = '%.1f',Title= "",LabelFontSize=12,Enabled=1,LookupTable=Table,TitleFontSize=12,LabelColor=[0.0, 0.0, 0.0],TitleColor=[0.0, 0.0, 0.0],)
    return scalarbar
  #

#
