// Copyright (C) 2010-2015  CEA/DEN, EDF R&D
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

#include "vtkParaSMESHCorbaSource.h"

#include <SALOME_LifeCycleCORBA.hxx>
#include <SalomeApp_Application.h>

#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGridReader.h"

#include "SMDS_UnstructuredGrid.hxx"
#include "SMESH_Mesh.hh"

vtkStandardNewMacro(vtkParaSMESHCorbaSource);

void *vtkParaSMESHCorbaSource::Orb=0;

//----------------------------------------------
vtkParaSMESHCorbaSource::vtkParaSMESHCorbaSource() {
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
vtkParaSMESHCorbaSource::~vtkParaSMESHCorbaSource() {
  
}

//----------------------------------------------
const char* vtkParaSMESHCorbaSource::GetIORCorba()
{
  return &IOR[0];
}

//----------------------------------------------
void vtkParaSMESHCorbaSource::SetIORCorba(char *ior) {
  if(!ior)
    return;
  if(ior[0]=='\0')
    return;
  int length=strlen(ior);
  IOR.resize(length+1);
  vtksys_stl::copy(ior,ior+length+1,&IOR[0]);
  this->Modified();
}

//----------------------------------------------
int vtkParaSMESHCorbaSource::ProcessRequest(vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) {
  // generate the data
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA())) {
    return this->RequestData(request, inputVector, outputVector);
  }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------
int vtkParaSMESHCorbaSource::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info) { 
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  return 1;
}

//----------------------------------------------
int vtkParaSMESHCorbaSource::RequestData(vtkInformation* request, vtkInformationVector** inInfo, vtkInformationVector* outputVector) {
  vtkInformation *outInfo=outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *ret0=vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  double reqTS = 0;
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    reqTS = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());

  try {
    CORBA::ORB_var *OrbC=(CORBA::ORB_var *)this->Orb;
    CORBA::Object_var obj=(*OrbC)->string_to_object(&IOR[0]);
    SMESH::SMESH_Mesh_var meshObj = SMESH::SMESH_Mesh::_narrow(obj);
    if(!CORBA::is_nil(meshObj)) {
      SALOMEDS::TMPFile*  SeqFile = meshObj->GetVtkUgStream();
      int aSize = SeqFile->length();
      char* buf = (char*)SeqFile->NP_data();
      vtkUnstructuredGridReader* aReader = vtkUnstructuredGridReader::New();
      aReader->ReadFromInputStringOn();
      aReader->SetBinaryInputString(buf, aSize);
      aReader->Update();
      vtkUnstructuredGrid* ret = vtkUnstructuredGrid::New();
      ret->ShallowCopy(aReader->GetOutput());
      aReader->Delete();
      if(!ret) {
	vtkErrorMacro("On smesh object CORBA fetching an error occurs !");
	return 0;
      }

      ret0->SetBlock(0, ret);
      ret->Delete();
      return 1;
    }
    
    vtkErrorMacro("Unrecognized CORBA reference!");
  }
  catch(CORBA::Exception&) {
    vtkErrorMacro("On fetching object error occurs");
  }
  return 0;
}



