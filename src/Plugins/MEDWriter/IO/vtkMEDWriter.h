// Copyright (C) 2016-2019  CEA/DEN, EDF R&D
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

#ifndef vtkMEDWriter_h__
#define vtkMEDWriter_h__

#include "vtkDataObjectAlgorithm.h"

class vtkMutableDirectedGraph;

class VTK_EXPORT vtkMEDWriter : public vtkDataObjectAlgorithm
{
public:
  static vtkMEDWriter* New();
  vtkTypeMacro(vtkMEDWriter, vtkDataObjectAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  int Write();
  vtkGetMacro(WriteAllTimeSteps, int);
  vtkSetMacro(WriteAllTimeSteps, int);
  vtkBooleanMacro(WriteAllTimeSteps, int);
protected:
  vtkMEDWriter();
  ~vtkMEDWriter();

  int RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
  int RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector);
  int RequestUpdateExtent(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
private:
  vtkMEDWriter(const vtkMEDWriter&);
  void operator=(const vtkMEDWriter&); // Not implemented.
 private:
  int WriteAllTimeSteps;
  int NumberOfTimeSteps;
  int CurrentTimeIndex;
  char *FileName;
  //BTX
  //ETX
};

#endif
