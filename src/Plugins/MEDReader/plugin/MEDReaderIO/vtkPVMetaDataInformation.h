// Copyright (C) 2010-2020  CEA/DEN, EDF R&D
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

#ifndef __vtkPVMetaDataInformation_h
#define __vtkPVMetaDataInformation_h

#include "vtkPVInformation.h"

class vtkDataObject;
class vtkInformationDataObjectKey;

class vtkPVMetaDataInformation : public vtkPVInformation
{
public:
  static vtkPVMetaDataInformation* New();
  vtkTypeMacro(vtkPVMetaDataInformation, vtkPVInformation)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  //BTX
  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);
  virtual void AddInformation(vtkPVInformation*);
  //ETX

  // Description:
  // Returns the Information Data.
  vtkGetObjectMacro(InformationData, vtkDataObject);

//BTX
protected:
  vtkPVMetaDataInformation();
  ~vtkPVMetaDataInformation();
  void SetInformationData(vtkDataObject*);
  vtkDataObject* InformationData;

private:
  vtkPVMetaDataInformation(const vtkPVMetaDataInformation&); // Not implemented
  void operator=(const vtkPVMetaDataInformation&); // Not implemented
//ETX
};

#endif
