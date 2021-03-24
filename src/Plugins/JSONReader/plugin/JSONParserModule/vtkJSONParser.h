// Copyright (C) 2015-2021  CEA/DEN, EDF R&D
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
// Author: Roman NIKOLAEV (roman.nikolaev@opencascade.com)

#ifndef __vtkJSONParser_h_
#define __vtkJSONParser_h_

#include <exception>
#include <stdio.h>
#include <string>
#include <vector>
#include <vtkObject.h>

class vtkTable;
class vtkJSONNode;
class vtkJSONMetaNode;
class vtkJSONInfoNode;

//---------------------------------------------------
class VTK_EXPORT vtkJSONException : public std::exception
{
public:
  vtkJSONException(const char* reason);
  ~vtkJSONException() noexcept;
  const char* what() const noexcept;

protected:
  std::string Reason;
};

class VTK_EXPORT vtkJSONParser : public vtkObject
{
public:
  static vtkJSONParser* New();

  // Description:
  // Specifies the name of the file
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);

  virtual int Parse(vtkTable* theTable);

protected:
  // Struct to store cursor information
  //----------------------------------
  struct Info
  {
  public:
    char type;
    long ln;
    long cn;
    long pos;

    Info()
    {
      type = 0;
      ln = -1;
      cn = -1;
      pos = -1;
    }
  };

  vtkJSONParser();
  ~vtkJSONParser();

  // name of the file to read from
  //----------------------------------
  char* FileName;

  // current line and column
  //----------------------------------
  int LineNumber;
  int ColumnNumber;

  // markup information
  //----------------------------------
  std::vector<Info> CInfoVector;

  // file
  //----------------------------------
  FILE* File;

  // Nodes
  //----------------------------------
  std::vector<vtkJSONNode*> Nodes;
  vtkJSONNode* CurrentNode;
  vtkJSONMetaNode* MetaNode;

  // Nodes
  //----------------------------------
  std::vector<char> ExpectedCharacters;

  // Flags
  //----------------------------------
  bool InsideQuotes;
  bool ParseList;
  bool ParseObjectList;
  bool ShortNamesFilled;

  // Last parced string
  //----------------------------------
  char* LastString;
  std::vector<const char*> Strings;
  std::vector<const char*> CurrentList;
  std::vector<const char*> ShortNames;

  // Last parced values
  //----------------------------------
  double LastValue;

private:
  vtkJSONParser(const vtkJSONParser&) = delete;
  void operator=(const vtkJSONParser&) = delete;

  vtkJSONMetaNode* GetMetaNode();
  vtkJSONInfoNode* GetInfoNode();

  void processOCB();
  void processCCB();

  void processOSB();
  void processCSB();

  void processCOMMA();

  void processCOLON();

  void processENDL();

  void processQTS();

  void processCharacter(const char ch);

  void processMetaNode();
  void processInfoNode();

  void readDoubleValue();

  char* getString(long b, long e);

  void allowsDigits();

  bool isDigitsAllowed();

  void checkShortName(const char* unit);

  void finalize(vtkTable* t);

  void clean();

  void throwSimpleException(const char* message);

  void throwException(const char* message);

  void throwException(const char* message, int ln, int cn);

  void throwException(const char* message, int ln);
};
#endif //__vtkJSONParser_h_
