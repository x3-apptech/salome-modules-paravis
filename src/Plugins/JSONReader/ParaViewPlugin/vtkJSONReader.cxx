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
// Author: Roman NIKOLAEV

#include "vtkJSONReader.h"
#include "vtkJSONParser.h"

#include <vtkObjectFactory.h>
#include <vtkTable.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVariantArray.h>
#include <vtkStringArray.h>
#include <vtkStringToNumeric.h>

#include <stdexcept>
#include <sstream>

vtkStandardNewMacro(vtkJSONReader);

//---------------------------------------------------
vtkJSONReader::vtkJSONReader()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->FileName = NULL;
  this->Parser = vtkJSONParser::New();
}

//---------------------------------------------------
vtkJSONReader::~vtkJSONReader()
{
  this->SetFileName(NULL);
  this->Parser->Delete();
}

//---------------------------------------------------
int vtkJSONReader::CanReadFile(const char* fname)
{
  return 1;
}

//---------------------------------------------------
void vtkJSONReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " 
      << (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "Parser: "<<this->Parser << endl;
}

//---------------------------------------------------
int vtkJSONReader::RequestData(vtkInformation*, 
    vtkInformationVector**,
    vtkInformationVector* outputVector)
{
  vtkTable* const output_table = vtkTable::GetData(outputVector);

  vtkInformation* const outInfo = outputVector->GetInformationObject(0);
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) &&
  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
  {
   return 1;
  }

 // If the filename is not defined
 if (!this->FileName && !this->Parser)
   {
     return 1;
   }

 this->Parser->SetFileName(this->FileName);
 try{
   return this->Parser->Parse(output_table);
 } catch(vtkJSONException e) {
   std::cout<<e.what()<<std::endl;
   return 1;
 }
}
