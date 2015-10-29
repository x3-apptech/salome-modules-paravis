//  Copyright (C) 2015 CEA/DEN, EDF R&D, OPEN CASCADE
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
// Autor: Roman NIKOLAEV (roman.nikolaev@opencascade.com)

#ifndef __vtkJSONReader_h_
#define __vtkJSONReader_h_

#include "vtkTableAlgorithm.h"

class vtkStringArray;
class vtkJSONParser;

class VTK_EXPORT vtkJSONReader: public vtkTableAlgorithm
{
public:
  static vtkJSONReader* New();
  vtkTypeMacro(vtkJSONReader, vtkTableAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specifies the name of the file
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  // Description:
  // Determine whether the given file can be read
  virtual int CanReadFile(const char* fname);

  // Description:
  // Request Data
  virtual int RequestData(vtkInformation*, 
			  vtkInformationVector**,
			  vtkInformationVector*);


protected:
  vtkJSONReader();
  ~vtkJSONReader();  
  // name of the file to read from
  char*            FileName;

  vtkJSONParser*   Parser;

private:
  vtkJSONReader(const vtkJSONReader&); // Not implemented.
  void operator=(const vtkJSONReader&); // Not implemented.
};

#endif //__vtkJSONReader_h_
