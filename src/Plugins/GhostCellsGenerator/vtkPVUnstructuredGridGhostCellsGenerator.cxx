/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPVUnstructuredGridGhostCellsGenerator.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkPVUnstructuredGridGhostCellsGenerator.h"
#include "vtkPVConfig.h" // need ParaView defines before MPI stuff

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiProcessController.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"

#ifdef PARAVIEW_USE_MPI
#  include "vtkPUnstructuredGridGhostCellsGenerator.h"
#endif

static const char* PVUGGCG_GLOBAL_POINT_IDS = "GlobalNodeIds";

//----------------------------------------------------------------------------

vtkStandardNewMacro(vtkPVUnstructuredGridGhostCellsGenerator)
vtkSetObjectImplementationMacro(
  vtkPVUnstructuredGridGhostCellsGenerator, Controller, vtkMultiProcessController);

//----------------------------------------------------------------------------
vtkPVUnstructuredGridGhostCellsGenerator::vtkPVUnstructuredGridGhostCellsGenerator()
{
  this->Controller = NULL;
  this->SetController(vtkMultiProcessController::GetGlobalController());

  this->GlobalPointIdsArrayName = NULL;
  this->UseGlobalPointIds = true;
  this->SetGlobalPointIdsArrayName(PVUGGCG_GLOBAL_POINT_IDS);
}

//----------------------------------------------------------------------------
vtkPVUnstructuredGridGhostCellsGenerator::~vtkPVUnstructuredGridGhostCellsGenerator()
{
  this->SetController(NULL);
  this->SetGlobalPointIdsArrayName(NULL);
}

//-----------------------------------------------------------------------------
void vtkPVUnstructuredGridGhostCellsGenerator::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
int vtkPVUnstructuredGridGhostCellsGenerator::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output. Input may just have the UnstructuredGridBase
  // interface, but output should be an unstructured grid.
  vtkUnstructuredGridBase *input = vtkUnstructuredGridBase::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
#ifdef PARAVIEW_USE_MPI
  vtkNew<vtkPUnstructuredGridGhostCellsGenerator> ghostGenerator;
  ghostGenerator->SetInputData(input);
  ghostGenerator->SetController(this->Controller);
  ghostGenerator->SetUseGlobalPointIds(this->UseGlobalPointIds);
  ghostGenerator->SetGlobalPointIdsArrayName(this->GlobalPointIdsArrayName);
  ghostGenerator->Update();
  output->ShallowCopy(ghostGenerator->GetOutput());
#else
  output->ShallowCopy(input);
#endif
  return 1;
}
