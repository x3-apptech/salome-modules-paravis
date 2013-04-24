// Copyright (C) 2010-2013  CEA/DEN, EDF R&D
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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

#ifndef __TableParser_h_
#define __TableParser_h_

#include <string>
#include <vector>

struct Table2D 
{
  typedef std::string Value;
  typedef std::vector<Value> Values;
  
  struct Row
  {
    std::string myTitle;
    std::string myUnit;
    Values myValues;
  };
  
  std::string myTitle;
  std::vector<std::string> myColumnUnits;
  std::vector<std::string> myColumnTitles;
  
  typedef std::vector<Row> Rows;
  Rows myRows;
  
  bool Check();
};

std::vector<std::string> GetTableNames(const char* fname, const char* separator,
				       const bool firstStringAsTitles);
Table2D GetTable(const char* fname, const char* separator, const int tableNb,
		 const bool firstStringAsTitles);

#endif //__TableParser_h_