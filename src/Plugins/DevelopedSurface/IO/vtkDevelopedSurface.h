// Copyright (C) 2017  EDF R&D
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

#ifndef vtkDevelopedSurface_h__
#define vtkDevelopedSurface_h__

#include "vtkDataSetAlgorithm.h"

class vtkMutableDirectedGraph;
class vtkImplicitFunction;
class vtkCylinder;

class VTK_EXPORT vtkDevelopedSurface : public vtkDataSetAlgorithm
{
public:
  static vtkDevelopedSurface* New();
  vtkTypeMacro(vtkDevelopedSurface, vtkDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);
  void SetCutFunction(vtkImplicitFunction* func);
  vtkMTimeType GetMTime();
  void SetInvertWay(bool invertStatus);
  void SetThetaOffset(double offsetInDegrees);
protected:
  vtkDevelopedSurface();
  ~vtkDevelopedSurface();
  int FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info);
  int RequestInformation(vtkInformation *request,
      vtkInformationVector **inputVector, vtkInformationVector *outputVector);

  int RequestData(vtkInformation *request, vtkInformationVector **inputVector,
      vtkInformationVector *outputVector);
private:
  vtkDevelopedSurface(const vtkDevelopedSurface&);
  void operator=(const vtkDevelopedSurface&); // Not implemented.
 private:
  //BTX
  vtkCylinder *_cyl;
  //ETX
  
  class vtkInternals;
  vtkInternals *Internal;
  bool InvertStatus;
  double OffsetInRad;
};

#endif
