// Copyright (C) 2014-2016  CEA/DEN, EDF R&D
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
// Author : Roman NIKOLAEV

#include "vtkArrayRenamerFilter.h"

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

#include <map>
#include <string>
#include <vector>

//For debug 
//#include <iostream>

class vtkArrayRenamerFilter::vtkInternals 
{
public:
  //Vector which contains information about array's components : origin_component_name <-> new_name
  typedef std::vector< std::pair<int, std::string> > ComponentsInfo;
  
  struct ArrayInfo
  {
  std::string   NewName;                               //  New name of the array
  bool          CopyArray;                             //  Make copy of the array or keep origin array, but change the name
  ComponentsInfo ComponentVector;                      //  Components of the array
    ArrayInfo():
      NewName( "" ),
      CopyArray( false )
    {
    }
  };
  typedef std::map<std::string, ArrayInfo> ArraysType;  //  Map : origin_aray_name <-> ArrayInfo struct
  ArraysType Arrays;
};


//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkArrayRenamerFilter);
//--------------------------------------------------------------------------------------------------
vtkArrayRenamerFilter::vtkArrayRenamerFilter()
{
  this->Internals = new vtkInternals();
}

//--------------------------------------------------------------------------------------------------
vtkArrayRenamerFilter::~vtkArrayRenamerFilter()
{
  delete this->Internals;
}



//--------------------------------------------------------------------------------------------------
int vtkArrayRenamerFilter::RequestData( vtkInformation*        theRequest,
					vtkInformationVector** theInputVector,
					vtkInformationVector*  theOutputVector )
{

  //std::cout<<"vtkArrayRenamerFilter::RequestData  !!! "<<std::endl;
  
  // Get the information
  vtkInformation *anInputInfo  = theInputVector[0]->GetInformationObject( 0 );
  vtkInformation *anOutputInfo = theOutputVector->GetInformationObject( 0 );

  vtkDataSet* anInput = vtkDataSet::GetData(theInputVector[0], 0);
  vtkDataSet* anOutput = vtkDataSet::GetData(theOutputVector, 0);
  anOutput->DeepCopy(anInput);
  vtkFieldData* data = 0;
  
  vtkInternals::ArraysType::iterator it = this->Internals->Arrays.begin();
  for( ; it !=  this->Internals->Arrays.end(); it ++ ) {
    vtkDataArray* array = anOutput->GetPointData()->GetArray( it->first.c_str() );
    data =  anOutput->GetPointData();
    if( !array ) {
      array = anOutput->GetCellData()->GetArray( it->first.c_str() );
      data =  anOutput->GetCellData();
    }
    
    if( array && !it->second.NewName.empty() ) {
      
      if ( it->second.CopyArray ) {
	vtkDataArray* new_array = array->NewInstance();
	new_array->DeepCopy(array);
	data->AddArray(new_array);
	array = new_array;
      } 
      array->SetName(it->second.NewName.c_str());
    }
    
    if ( array ) {
      vtkInternals::ComponentsInfo::iterator vect_it = it->second.ComponentVector.begin();
      for( ; vect_it != it->second.ComponentVector.end(); vect_it++ ) {
	array->SetComponentName( vect_it->first, vect_it->second.c_str() );
      }
    }
  }
  
  return Superclass::RequestData( theRequest, theInputVector, theOutputVector );
}


void vtkArrayRenamerFilter::SetComponentInfo( const char* arrayname, const int compid, const char* newcompname ) {
  //std::cout<<"vtkArrayRenamerFilter::SetComponentArrayInfo : "<<arrayname<<" "<<"id : "<<compid<<" , newcompname = "<<newcompname<<std::endl;

  vtkInternals::ArraysType::iterator it = this->Internals->Arrays.find(arrayname);

  if ( it == this->Internals->Arrays.end() ) {
    vtkInternals::ArraysType::iterator it1 = this->Internals->Arrays.begin();
    for( ; it1 != this->Internals->Arrays.end(); it1 ++ ) {
      if( it1->second.NewName.compare(arrayname) == 0 ) {
	it = it1;
	break;
      }      
    }
  }

  if( it == this->Internals->Arrays.end() ) {
    std::pair<vtkInternals::ArraysType::iterator,bool> ret;
    ret = this->Internals->Arrays.insert ( std::pair<std::string, vtkInternals::ArrayInfo>( arrayname, vtkInternals::ArrayInfo() ) );
    it = ret.first;
  }
  
  if( it != this->Internals->Arrays.end() ) {
    vtkInternals::ComponentsInfo::iterator vect_it = it->second.ComponentVector.begin();
    for( ; vect_it != it->second.ComponentVector.end(); vect_it++ ) {
      if ( vect_it->first ==  compid )
	break;
    }

    if ( vect_it != it->second.ComponentVector.end() ) {
      vect_it->second = newcompname;
    } else {
      it->second.ComponentVector.push_back( std::pair<int,std::string>( compid,newcompname ) );
    }
  }
  this->Modified();
}

void vtkArrayRenamerFilter::SetArrayInfo( const char* originarrayname, const char* newarrayname, bool copy ) {
  
  //std::cout<<"vtkArrayRenamerFilter::SetArrayInfo : "<<originarrayname<<" "<<"new name :"<<newarrayname<<" , copy = "<<copy<<std::endl;
  
  vtkInternals::ArraysType::iterator it = this->Internals->Arrays.find(originarrayname);
    
  if ( it == this->Internals->Arrays.end() )
    this->Internals->Arrays.insert ( std::pair<std::string, vtkInternals::ArrayInfo>( originarrayname, vtkInternals::ArrayInfo() ) );

  this->Internals->Arrays[originarrayname].NewName = newarrayname;
  this->Internals->Arrays[originarrayname].CopyArray = copy;
  this->Modified();
}

void vtkArrayRenamerFilter::ClearArrayInfo() {
  //std::cout<<"vtkArrayRenamerFilter::ClearArrayInfo"<<std::endl;
  this->Internals->Arrays.clear();
  this->Modified();
}

void vtkArrayRenamerFilter::ClearComponentsInfo() {
  //std::cout<<"vtkArrayRenamerFilter::ClearComponentsInfo"<<std::endl;
  vtkInternals::ArraysType::iterator it = this->Internals->Arrays.begin();
  for( ; it != this->Internals->Arrays.end(); it ++ ){
    it->second.ComponentVector.clear();
  }
  this->Modified();
}
