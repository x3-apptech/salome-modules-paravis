// Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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

#include "MEDUtilities.hxx"

#include "vtkInformationIntegerKey.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"

#include <algorithm>

vtkInformationKeyMacro(MEDUtilities,ELGA,Integer);
vtkInformationKeyMacro(MEDUtilities,ELNO,Integer);

void ExportedTinyInfo::pushGaussAdditionnalInfo(int ct, int dim, const std::vector<double>& refCoo, const std::vector<double>& posInRefCoo)
{
  prepareForAppend();
  std::vector<double> tmp(1,(double)ct);
  tmp.push_back((double)dim);
  tmp.insert(tmp.end(),refCoo.begin(),refCoo.end());
  tmp.insert(tmp.end(),posInRefCoo.begin(),posInRefCoo.end());
  _data.push_back((double)tmp.size());
  _data.insert(_data.end(),tmp.begin(),tmp.end());
}

void ExportedTinyInfo::prepareForAppend()
{
  if(_data.empty())
    _data.push_back(1.);
  else
    {
      double val(_data[0]);
      int val2((int) val);
      _data[0]=++val2;
    }
}
