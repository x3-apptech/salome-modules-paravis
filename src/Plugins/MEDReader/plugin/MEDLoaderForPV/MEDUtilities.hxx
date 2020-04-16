// Copyright (C) 2010-2020  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//
// Author : Anthony Geay

#ifndef __MEDUTILITIES_HXX__
#define __MEDUTILITIES_HXX__

#include "MEDLoaderForPV.h"
#include "vtkCellType.h"

#include <vector>

class vtkInformationIntegerKey;

class MEDLOADERFORPV_EXPORT MEDUtilities
{
public:
  static vtkInformationIntegerKey *ELGA();
  static vtkInformationIntegerKey *ELNO();
};

class ExportedTinyInfo
{
public:
  void pushGaussAdditionnalInfo(int ct, int dim, const std::vector<double>& refCoo, const std::vector<double>& posInRefCoo);
  const std::vector<double>& getData() const { return _data; }
  bool empty() const { return _data.empty(); }
private:
  void prepareForAppend();
private:
  // first place is nb of ct
  // 2nd place is the size of first ct def (this 2nd place included)
  // 3rd place is the VTK cell type of first ct def
  // 4th place is the dimension of first ct def
  // 5th->n th : ref Coo
  // nth -> n+p th : posInRefCoo
  // n+p+1 -> size of second ct def (this n+p+1 place included)
  // n+p+2 -> VTK cell type of second ct def
  // ...
  std::vector<double> _data;
};

#endif
