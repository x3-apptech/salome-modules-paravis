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

#include "vtkParaGEOMCorbaSource.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkUnstructuredGrid.h"
//
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkInformation.h"

#include <SALOME_LifeCycleCORBA.hxx>
#include <SALOME_NamingService.hxx>
#include <TopoDS_Shape.hxx>
#include "vtkPolyData.h"
#include "GEOM_Gen.hh"
#include "GEOM_Client.hxx"
#include "OCC2VTK_Tools.h"

#include <algorithm>

//----------------------------------------------
vtkStandardNewMacro(vtkParaGEOMCorbaSource);

void *vtkParaGEOMCorbaSource::Orb=0;

//----------------------------------------------
vtkParaGEOMCorbaSource::vtkParaGEOMCorbaSource():Deflection(0.0)
{
  if(!Orb) {
    CORBA::ORB_var *OrbC=new CORBA::ORB_var;
    int argc=0;
    *OrbC=CORBA::ORB_init(argc,0);
    this->Orb=OrbC;
  }
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------
vtkParaGEOMCorbaSource::~vtkParaGEOMCorbaSource() {
}

//----------------------------------------------
const char* vtkParaGEOMCorbaSource::GetIORCorba()
{
  return &IOR[0];
}

//----------------------------------------------
void vtkParaGEOMCorbaSource::SetIORCorba(char *ior) {
  if(!ior)
    return;
  if(ior[0]=='\0')
    return;
  int length=strlen(ior);
  IOR.resize(length+1);
  std::copy(ior,ior+length+1,&IOR[0]);
  this->Modified();
}

//----------------------------------------------
int vtkParaGEOMCorbaSource::ProcessRequest(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) {
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA())) {
    return this->RequestData(request, inputVector, outputVector);
  }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------
int vtkParaGEOMCorbaSource::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info) {
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------
int vtkParaGEOMCorbaSource::RequestData(vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outputVector) {
  vtkInformation *outInfo=outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *ret0=vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  double reqTS = 0;
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    reqTS = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
  try {
    //Client request on ORB.
    CORBA::ORB_var *OrbC=(CORBA::ORB_var *)this->Orb;
    CORBA::Object_var obj=(*OrbC)->string_to_object(&IOR[0]);
    GEOM::GEOM_Object_var geomObj = GEOM::GEOM_Object::_narrow(obj);
    
    if(!CORBA::is_nil(geomObj)) {
      SALOME_NamingService ns(*OrbC);
      SALOME_LifeCycleCORBA aLCC(&ns);
      Engines::EngineComponent_var aComponent = aLCC.FindOrLoad_Component("FactoryServer","GEOM");
      GEOM::GEOM_Gen_var geomGen = GEOM::GEOM_Gen::_narrow(aComponent);
      if ( !CORBA::is_nil( geomGen ) ) {
	TopoDS_Shape aTopoDSShape = GEOM_Client::get_client().GetShape( geomGen, geomObj );
	
	if ( !aTopoDSShape.IsNull() ) {
	  vtkPolyData *ret=GEOM::GetVTKData(aTopoDSShape, this->Deflection);
	  if(!ret) {
	    vtkErrorMacro("On geom object CORBA fetching an error occurs !");
	    return 0;
	  }
	  ret0->SetBlock(0,ret);
	  ret->Delete();
	  return 1;
	}
      }
    }
    vtkErrorMacro("Unrecognized CORBA reference!");
  }
  catch(CORBA::Exception& ex) {
    vtkErrorMacro("On fetching object error occurs");
  }
  return 0;
}

//----------------------------------------------
void vtkParaGEOMCorbaSource::PrintSelf(ostream& os, vtkIndent indent) {
  this->Superclass::PrintSelf( os, indent );
  os << "Deflection: " << this->Deflection << "\n";
}

