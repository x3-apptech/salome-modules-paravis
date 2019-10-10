// Copyright (C) 2017-2019  EDF R&D
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

#include <vtkDataSetAlgorithm.h>

class vtkMutableDirectedGraph;
class vtkImplicitFunction;
class vtkCylinder;

class vtkDevelopedSurface : public vtkDataSetAlgorithm
{
public:
  static vtkDevelopedSurface* New();
  vtkTypeMacro(vtkDevelopedSurface, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetCutFunction(vtkImplicitFunction* func);

  void SetInvertWay(bool invertStatus);

  void SetThetaOffset(double offsetInDegrees);

  vtkMTimeType GetMTime();

protected:
  vtkDevelopedSurface();
  ~vtkDevelopedSurface();

  int FillOutputPortInformation(int, vtkInformation*) override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkCylinder* _cyl;
  class vtkInternals;
  vtkInternals* Internal;
  bool InvertStatus;
  double OffsetInRad;

private:
  vtkDevelopedSurface(const vtkDevelopedSurface&) = delete;
  void operator=(const vtkDevelopedSurface&) = delete;
};

#endif
