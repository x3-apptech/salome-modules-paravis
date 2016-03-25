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


#ifndef __vtkParaGEOMCorbaSource_h
#define __vtkParaGEOMCorbaSource_h

#include "vtkUnstructuredGridAlgorithm.h"
#include <vector>

class vtkParaGEOMCorbaSource : public vtkAlgorithm {
 public:
  static vtkParaGEOMCorbaSource* New();
  vtkTypeMacro(vtkParaGEOMCorbaSource, vtkAlgorithm);
  virtual void PrintSelf( ostream& os, vtkIndent indent );
  const char *GetIORCorba();
  void SetIORCorba(char *ior);
  
  vtkSetMacro(Deflection,double);
  
 protected:
  vtkParaGEOMCorbaSource();
  virtual ~vtkParaGEOMCorbaSource();
  int FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info);
  int ProcessRequest(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector);
  virtual int RequestData( vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outInfo );

  std::vector<char> IOR;
  static void *Orb;
  double Deflection;
  
 private:
  vtkParaGEOMCorbaSource( const vtkParaGEOMCorbaSource& ); // Not implemented.
  void operator = ( const vtkParaGEOMCorbaSource& ); // Not implemented.
};

#endif // __vtkParaGEOMCorbaSource_h

