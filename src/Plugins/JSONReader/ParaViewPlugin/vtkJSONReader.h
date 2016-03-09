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

#include "vtk_jsoncpp.h" // For json parser
#include "vtkTableAlgorithm.h"


//---------------------------------------------------
class VTK_EXPORT vtkJSONException : public std::exception {
 public:
    vtkJSONException(const char *reason);
    ~vtkJSONException() throw ();
    const char* what() const throw();
 protected:
  std::string Reason;
};

class vtkStringArray;

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
  // Request Data
  virtual int RequestData(vtkInformation*, 
			  vtkInformationVector**,
			  vtkInformationVector*);

protected:
  // Decription:
  // Parse the Json Value corresponding to the root data from the file
  virtual void Parse(Json::Value& root, vtkTable *theTable);

  // Decription:
  // Verify if file exists and can be read by the parser
  // If exists, parse into Jsoncpp data structure
  int CanParseFile(const char *fname, Json::Value &root);

protected:
  vtkJSONReader();
  ~vtkJSONReader();  
  // name of the file to read from
  char*            FileName;

private:
  vtkJSONReader(const vtkJSONReader&); // Not implemented.
  void operator=(const vtkJSONReader&); // Not implemented.
};

#endif //__vtkJSONReader_h_
