// Copyright (C) 2017  CEA/DEN, EDF R&D
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

#ifndef vtkSimpleMode_h__
#define vtkSimpleMode_h__

#include "vtkPolyDataAlgorithm.h"

class vtkMutableDirectedGraph;

class VTK_EXPORT vtkSimpleMode : public vtkPolyDataAlgorithm
{
public:
  static vtkSimpleMode* New();
  vtkTypeMacro(vtkSimpleMode, vtkPolyDataAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetInputArrayToProcess(int idx, int port, int connection, int fieldAssociation, const char *name);
  vtkGetMacro(Factor,double);
  vtkSetClampMacro(Factor,double,0.,VTK_DOUBLE_MAX);
  vtkGetMacro(AnimationTime,double);
  vtkSetClampMacro(AnimationTime,double,0.,1.);
protected:
  vtkSimpleMode();
  ~vtkSimpleMode();

  int RequestInformation(vtkInformation *request,
      vtkInformationVector **inputVector, vtkInformationVector *outputVector);

  int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
      vtkInformationVector *outputVector);
private:
  vtkSimpleMode(const vtkSimpleMode&);
  void operator=(const vtkSimpleMode&); // Not implemented.
 private:
  //BTX
  //ETX
  double Factor;
  double AnimationTime;
  class vtkSimpleModeInternal;
  vtkSimpleModeInternal* Internal;
};

#endif
