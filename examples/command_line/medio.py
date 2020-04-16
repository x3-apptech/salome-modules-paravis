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
def __select_data(reader, dataname=None):
  if dataname:
    keys = reader.GetProperty("FieldsTreeInfo")[::2]
    # list all the names of arrays that can be seen (including their spatial discretization)
    arr_name_with_dis = [elt.split("/")[-1] for elt in keys]
    # list all the names of arrays (Equal to those in the MED File)
    separator = reader.GetProperty("Separator").GetData()
    arr_name = [elt.split(separator)[0] for elt in arr_name_with_dis]
    for idx in range(len(keys)):
      if arr_name[idx] == dataname:
        reader.AllArrays = keys[idx]
        break

  return reader
#

def get_element_type(reader):
  # Return 'P0', 'P1'...
  separator = reader.GetProperty("Separator").GetData()
  return reader.AllArrays[0].split(separator)[1]
#

def get_element_name(reader):
  separator = reader.GetProperty("Separator").GetData()
  return reader.AllArrays[0].split(separator)[0].split("/")[-1]
#

def load_mesh(med_filename, mesh_name=None):
  import pvsimple
  reader = pvsimple.MEDReader(FileName=med_filename)
  return __select_data(reader, mesh_name)
#

def load_field(med_filename, field_name=None):
  import pvsimple
  reader = pvsimple.MEDReader(FileName=med_filename)
  return __select_data(reader, field_name)
#
