// Copyright (C) 2016-2020  CEA/DEN, EDF R&D
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

#include "visu.hxx"
#include "MEDFileField.hxx"
#include "MEDCouplingUMesh.hxx"
#include "MEDCouplingCMesh.hxx"
#include "MEDCouplingCurveLinearMesh.hxx"
#include "MEDFileMesh.hxx"
#include "MEDFileFieldRepresentationTree.hxx"
#include <iostream>
#include "mpi.h" 

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkCPProcessor.h>
#include "vtkDataSet.h"
#include "vtkCellData.h"

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>

using namespace MEDCoupling;


void Visualization::CatalystInitialize(const std::string& script)
{

  if(Processor == NULL)
    {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
    }
  else
    {
    Processor->RemoveAllPipelines();
    }
  // Add in the Python script
  vtkCPPythonScriptPipeline* pipeline = vtkCPPythonScriptPipeline::New();
  char *c = const_cast<char*>(script.c_str());
  pipeline->Initialize(c);
  Processor->AddPipeline(pipeline);
  pipeline->Delete();
  return;
}

void Visualization::CatalystFinalize()
{
  if(Processor)
    {
    Processor->Delete();
    Processor = NULL;
    }

  return;
}

void Visualization::CatalystCoProcess(vtkDataSet *VTKGrid, double time,
                                      unsigned int timeStep)
{
  vtkCPDataDescription* dataDescription = vtkCPDataDescription::New();
  // specify the simulation time and time step for Catalyst
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);

  if(Processor->RequestDataDescription(dataDescription) != 0)
    {
    // Make a map from input to our VTK grid so that
    // Catalyst gets the proper input dataset for the pipeline.
    dataDescription->GetInputDescriptionByName("input")->SetGrid(VTKGrid);
    // Call Catalyst to execute the desired pipelines.
    Processor->CoProcess(dataDescription);
    }
  dataDescription->Delete();
}

void Visualization::ConvertToVTK(MEDCoupling::MEDCouplingFieldDouble* field, vtkDataSet *&VTKGrid)
//vtkDataSet * Visualization::ConvertToVTK(MEDCoupling::MEDCouplingFieldDouble* field)
{
  MEDCoupling::MEDCouplingFieldDouble *f = field;
  MEDCoupling::MCAuto<MEDFileField1TS> ff(MEDFileField1TS::New());
  ff->setFieldNoProfileSBT(f);
  MCAuto<MEDFileFieldMultiTS> fmts(MEDFileFieldMultiTS::New());
  fmts->pushBackTimeStep(ff);
  MCAuto<MEDFileFields> fs(MEDFileFields::New());
  fs->pushField(fmts);
  
  MEDCouplingMesh *m(f->getMesh());
  MCAuto<MEDFileMesh> mm;
  if(dynamic_cast<MEDCouplingUMesh *>(m))
    {
      MCAuto<MEDFileUMesh> mmu(MEDFileUMesh::New()); 
      mmu->setMeshAtLevel(0,dynamic_cast<MEDCouplingUMesh *>(m));
      mmu->forceComputationOfParts();
      mm=DynamicCast<MEDFileUMesh,MEDFileMesh>(mmu);
    }
  else if(dynamic_cast<MEDCouplingCMesh *>(m))
    {
      MCAuto<MEDFileCMesh> mmc(MEDFileCMesh::New()); 
      mmc->setMesh(dynamic_cast<MEDCouplingCMesh *>(m));
      mm=DynamicCast<MEDFileCMesh,MEDFileMesh>(mmc);
    }
  else if(dynamic_cast<MEDCouplingCurveLinearMesh *>(m))
    {
      // MCAuto<MEDFileCLMesh> mmc(MEDFileCLMesh::New()); 
      MCAuto<MEDFileCurveLinearMesh> mmc(MEDFileCurveLinearMesh::New()); 
      mmc->setMesh(dynamic_cast<MEDCouplingCurveLinearMesh *>(m));
      //mm=DynamicCast<MEDCouplingCurveLinearMesh,MEDFileMesh>(mmc);
      mm=0;
    }
  else
    {
    throw ;
    }
  MCAuto<MEDFileMeshes> ms(MEDFileMeshes::New());
  ms->pushMesh(mm);
  ms->getMeshesNames();
  //
  int proc_id;
  MPI_Comm_rank(MPI_COMM_WORLD,&proc_id);
  //
  MEDFileFieldRepresentationTree Tree;
  Tree.loadInMemory(fs,ms);
  
  Tree.changeStatusOfAndUpdateToHaveCoherentVTKDataSet(0,true);
  Tree.changeStatusOfAndUpdateToHaveCoherentVTKDataSet(1,false);
  Tree.activateTheFirst();//"TS0/Fluid domain/ComSup0/TempC@@][@@P0"
  std::string meshName;
  TimeKeeper TK(0);
  int tmp1,tmp2;
  double tmp3(f->getTime(tmp1,tmp2));
  vtkDataSet *ret(Tree.buildVTKInstance(false,tmp3,meshName,TK));
  VTKGrid = ret;
}

Visualization::Visualization()
{
Processor = NULL;
}


// ajouter en parametre le fichier python qui contient le pipeline
// enlever le const s'il gene
void Visualization::run(MEDCoupling::MEDCouplingFieldDouble* field, const std::string& pathPipeline)
{
  int proc_id;
  MPI_Comm_rank(MPI_COMM_WORLD,&proc_id);

  vtkDataSet *VTKGrid = 0;
  ConvertToVTK(field, VTKGrid);

  const char *fileName = pathPipeline.c_str();
  CatalystInitialize(fileName);
  CatalystCoProcess(VTKGrid, 0.0, 0);
  CatalystFinalize();
  
  return ;
}

