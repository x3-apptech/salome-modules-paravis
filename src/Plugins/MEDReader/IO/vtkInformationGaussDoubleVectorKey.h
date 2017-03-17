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

#ifndef __vtkInformationGaussDoubleVectorKey_h_
#define __vtkInformationGaussDoubleVectorKey_h_

#include "vtkInformationDoubleVectorKey.h"

class VTK_EXPORT vtkInformationGaussDoubleVectorKey : public vtkInformationDoubleVectorKey
{
public:
  vtkTypeMacro(vtkInformationGaussDoubleVectorKey, vtkInformationDoubleVectorKey);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE{}

  vtkInformationGaussDoubleVectorKey(const char* name, const char* location,
    int length = -1) : vtkInformationDoubleVectorKey(name, location, length) { }

  /**
  * This method simply returns a new vtkInformationDoubleVectorKey, given a
  * name, a location and a required length. This method is provided for
  * wrappers. Use the constructor directly from C++ instead.
  */
  static vtkInformationGaussDoubleVectorKey* MakeKey(const char* name, const char* location,
    int length = -1)
  {
    return new vtkInformationGaussDoubleVectorKey(name, location, length);
  }

  /**
  * Simply shallow copies the key from fromInfo to toInfo.
  * This is used by the pipeline to propagate this key downstream.
  */
  void CopyDefaultInformation(vtkInformation* request,
    vtkInformation* fromInfo,
    vtkInformation* toInfo) VTK_OVERRIDE
  {
    this->ShallowCopy(fromInfo, toInfo);
  }

  /*private:
  vtkInformationGaussDoubleVectorKey(const vtkInformationGaussDoubleVectorKey&) VTK_DELETE_FUNCTION;
  void operator=(const vtkInformationGaussDoubleVectorKey&) VTK_DELETE_FUNCTION;*/
};

#endif
