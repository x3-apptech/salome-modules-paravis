// Copyright (C) 2018  EDF R&D
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

#ifndef vtkGaussToCell_h__
#define vtkGaussToCell_h__

#include "vtkUnstructuredGridAlgorithm.h"

class vtkMutableDirectedGraph;

class VTK_EXPORT vtkGaussToCell : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGaussToCell* New();
  vtkTypeMacro(vtkGaussToCell, vtkUnstructuredGridAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetAvgFlag(bool avgStatus);

  void SetMaxFlag(bool maxStatus);

  void SetMinFlag(bool minStatus);

protected:
  vtkGaussToCell();
  ~vtkGaussToCell();

  int RequestInformation(vtkInformation *request,
      vtkInformationVector **inputVector, vtkInformationVector *outputVector);

  int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
      vtkInformationVector *outputVector);
  
private:
  vtkGaussToCell(const vtkGaussToCell&);
  void operator=(const vtkGaussToCell&); // Not implemented.
 private:
  //BTX
  //ETX
  bool avgStatus;
  bool maxStatus;
  bool minStatus;
};

#endif
