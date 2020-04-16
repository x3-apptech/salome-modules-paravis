// Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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
// Author : Anthony Geay (EDF R&D)

#ifndef __VTKMEDTRAITS_HXX__
#define __VTKMEDTRAITS_HXX__

class vtkIntArray;
class vtkLongArray;
#ifdef WIN32
class vtkLongLongArray;
#endif
class vtkFloatArray;
class vtkDoubleArray;

template<class T>
class MEDFileVTKTraits
{
public:
  typedef void VtkType;
  typedef void MCType;
};

template<>
class MEDFileVTKTraits<int>
{
public:
  typedef vtkIntArray VtkType;
  typedef MEDCoupling::DataArrayInt32 MCType;
};

template<>
#ifdef WIN32
class MEDFileVTKTraits<long long>
#else 
class MEDFileVTKTraits<long>
#endif
#
{
public:
#ifdef WIN32
  typedef vtkLongLongArray VtkType;
#else
  typedef vtkLongArray VtkType;
#endif
  typedef MEDCoupling::DataArrayInt64 MCType;
};

template<>
class MEDFileVTKTraits<float>
{
public:
  typedef vtkFloatArray VtkType;
  typedef MEDCoupling::DataArrayFloat MCType;
};

template<>
class MEDFileVTKTraits<double>
{
public:
  typedef vtkDoubleArray VtkType;
  typedef MEDCoupling::DataArrayDouble MCType;
};

#endif
