// Copyright (C) 2014-2021  CEA/DEN, EDF R&D
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
// Author : Roman NIKOLAEV

#ifndef __ArrayRenamerFilter_h_
#define __ArrayRenamerFilter_h_

#include <vtkDataSetAlgorithm.h>

/**
 * Description of class:
 * Class allows to rename data arrays and array's components.
 */
class VTK_EXPORT vtkArrayRenamerFilter : public vtkDataSetAlgorithm
{
public:
  /// Returns pointer on a new instance of the class
  static vtkArrayRenamerFilter* New();

  vtkTypeMacro(vtkArrayRenamerFilter, vtkDataSetAlgorithm)

  void SetArrayInfo(const char* originarrayname, const char* newarrayname, bool copy);
  void ClearArrayInfo();

  void SetComponentInfo(const char* arrayname, const int compid, const char* newarrayname);
  void ClearComponentsInfo();

protected:
  /// Constructor & destructor
  vtkArrayRenamerFilter();
  ~vtkArrayRenamerFilter() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkArrayRenamerFilter(const vtkArrayRenamerFilter&) = delete;
  void operator=(const vtkArrayRenamerFilter&) = delete;

  void FillListOfArrays(vtkDataObject*);

  class vtkInternals;
  friend class vtkInternals;
  vtkInternals* Internals;
};

#endif // __ArrayRenamerFilter_h_
