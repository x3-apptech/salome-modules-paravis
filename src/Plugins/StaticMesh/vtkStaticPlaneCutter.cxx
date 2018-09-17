/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticPlaneCutter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStaticPlaneCutter.h"

#include <vtkCellData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkIdFilter.h>
#include <vtkIdList.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiPieceDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>

vtkStandardNewMacro(vtkStaticPlaneCutter);

//----------------------------------------------------------------------------
vtkStaticPlaneCutter::vtkStaticPlaneCutter()
{
  this->InputMeshTime = 0;
  this->FilterMTime = 0;
}

//----------------------------------------------------------------------------
vtkStaticPlaneCutter::~vtkStaticPlaneCutter()
{
}

//-----------------------------------------------------------------------------
int vtkStaticPlaneCutter::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the inputs and outputs
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* input = vtkUnstructuredGrid::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet* inputMB = vtkMultiBlockDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet* mb =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!mb)
  {
    vtkErrorMacro("Ouput information does not contain expected type of data object");
    return 0;
  }

  // Recover the fisrt and only block so this works with single block mb
  if (inputMB && inputMB->GetNumberOfBlocks() == 1)
  {
    input = vtkUnstructuredGrid::SafeDownCast(inputMB->GetBlock(0));
  }

  // Recover the static unstructured grid
  if (!input)
  {
    // For any other type of input, fall back to superclass implementation
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  // Check cache validity
  if (this->InputMeshTime == input->GetMeshMTime() && this->FilterMTime == this->GetMTime())
  {
    // Cache mesh is up to date, use it to generate data
    if (this->InterpolateAttributes)
    {
      // Update the cache data
      this->UpdateCacheData(input);
    }

    // Copy the updated cache into the output
    mb->SetBlock(0, this->Cache.Get());
    return 1;
  }
  else
  {
    // Cache is invalid
    // Add needed Arrays
    vtkNew<vtkUnstructuredGrid> tmpInput;
    this->AddIdsArray(input, tmpInput.Get());

    // Create an input vector to pass the completed input to the superclass
    // RequestData method
    vtkNew<vtkInformationVector> tmpInputVec;
    tmpInputVec->Copy(inputVector[0], 1);
    vtkInformation* tmpInInfo = tmpInputVec->GetInformationObject(0);
    tmpInInfo->Set(vtkDataObject::DATA_OBJECT(), tmpInput.Get());
    vtkInformationVector* tmpInputVecPt = tmpInputVec.Get();
    int ret = this->Superclass::RequestData(request, &tmpInputVecPt, outputVector);

    // Update the cache with superclass output
    vtkMultiPieceDataSet* output = vtkMultiPieceDataSet::SafeDownCast(mb->GetBlock(0));
    if (!output)
    {
      vtkErrorMacro("Output is not of expected type");
      return 0;
    }
    this->RemovePointData(output);

    this->Cache->ShallowCopy(output);
    this->InputMeshTime = input->GetMeshMTime();
    this->FilterMTime = this->GetMTime();

    // Compute the cell ids to be passed from the input to the cache
    this->ComputeCellIds();

    return ret;
  }
}

//-----------------------------------------------------------------------------
void vtkStaticPlaneCutter::AddIdsArray(vtkDataSet* input, vtkDataSet* output)
{
  vtkDataSet* tmpInput;
  tmpInput = input;
  vtkNew<vtkIdFilter> generateIdScalars;

  // Check for Ids array
  vtkAbstractArray* cellIdsTmp = input->GetCellData()->GetAbstractArray("Ids");
  if (!cellIdsTmp)
  {
    // Create Ids array
    generateIdScalars->SetInputData(tmpInput);
    generateIdScalars->SetIdsArrayName("Ids");
    generateIdScalars->Update();
    tmpInput = generateIdScalars->GetOutput();
  }
  output->ShallowCopy(tmpInput);
}

//-----------------------------------------------------------------------------
void vtkStaticPlaneCutter::RemovePointData(vtkMultiPieceDataSet* dataset)
{
  // Iterate over each piece of the multipiece output
  vtkCompositeDataIterator* iter = dataset->NewIterator();
  iter->SkipEmptyNodesOn();
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkPolyData* slice = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
    if (slice)
    {
      // Remove all point data array on each slice
      vtkPointData* pd = slice->GetPointData();
      for (int i = pd->GetNumberOfArrays() - 1; i >= 0; i--)
      {
        pd->RemoveArray(i);
      }
    }
  }
  iter->Delete();
}

//-----------------------------------------------------------------------------
void vtkStaticPlaneCutter::ComputeCellIds()
{
  this->CellToCopyFrom.clear();
  this->CellToCopyTo.clear();

  // Iterate over each piece of the multipiece output
  vtkCompositeDataIterator* iter = this->Cache->NewIterator();
  iter->SkipEmptyNodesOn();
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkPolyData* slice = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
    if (slice)
    {
      // For each piece, recover the Ids of the cells sliced and the corresponding
      // cellId in the slice
      vtkSmartPointer<vtkIdList> cellIdsFrom = vtkSmartPointer<vtkIdList>::New();
      vtkSmartPointer<vtkIdList> cellIdsTo = vtkSmartPointer<vtkIdList>::New();
      this->CellToCopyFrom.push_back(cellIdsFrom);
      this->CellToCopyTo.push_back(cellIdsTo);

      vtkIdTypeArray* ids = vtkIdTypeArray::SafeDownCast(slice->GetCellData()->GetArray("Ids"));
      cellIdsFrom->SetNumberOfIds(ids->GetNumberOfValues());
      cellIdsTo->SetNumberOfIds(ids->GetNumberOfValues());
      for (vtkIdType i = 0; i < ids->GetNumberOfValues(); i++)
      {
        cellIdsFrom->SetId(i, ids->GetValue(i));
        cellIdsTo->SetId(i, i);
      }
    }
  }
  iter->Delete();
}

//-----------------------------------------------------------------------------
void vtkStaticPlaneCutter::UpdateCacheData(vtkDataSet* input)
{
  // Remove useless FieldData Array from multipiece
  // Created by automatic pass data in pipeline
  vtkFieldData* mpFieldData = this->Cache->GetFieldData();
  for (int i = mpFieldData->GetNumberOfArrays() - 1; i >= 0; i--)
  {
    mpFieldData->RemoveArray(i);
  }

  // Recover cell data
  // We may want to improve this and support PointData at some point
  vtkCellData* inCD = input->GetCellData();

  vtkCompositeDataIterator* iter = this->Cache->NewIterator();
  iter->SkipEmptyNodesOn();
  int i = 0;
  for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    vtkPolyData* slice = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
    if (slice)
    {
      vtkCellData* sliceCD = slice->GetCellData();
      for (int iArr = 0; iArr < inCD->GetNumberOfArrays(); iArr++)
      {
        // For each array of the cell data of each slice
        vtkAbstractArray* arrayToCopyIn = sliceCD->GetAbstractArray(inCD->GetArrayName(iArr));
        if (arrayToCopyIn)
        {
          // Copy the tuples from the input cell ids to the slice cell ids
          arrayToCopyIn->InsertTuples(this->CellToCopyTo[i].Get(), this->CellToCopyFrom[i].Get(),
            inCD->GetAbstractArray(iArr));
        }
      }
      // Update field data
      slice->GetFieldData()->PassData(input->GetFieldData());
    }
    i++;
  }
  iter->Delete();
}

//----------------------------------------------------------------------------
void vtkStaticPlaneCutter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Cache: " << this->Cache << endl;
  os << indent << "Input Mesh Time: " << this->InputMeshTime << endl;
  os << indent << "Filter mTime: " << this->FilterMTime << endl;
}
