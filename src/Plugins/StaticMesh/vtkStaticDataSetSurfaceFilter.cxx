/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticDataSetSurfaceFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStaticDataSetSurfaceFilter.h"

#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

vtkStandardNewMacro(vtkStaticDataSetSurfaceFilter);

//----------------------------------------------------------------------------
vtkStaticDataSetSurfaceFilter::vtkStaticDataSetSurfaceFilter()
{
  this->InputMeshTime = 0;
  this->FilterMTime = 0;
}

//----------------------------------------------------------------------------
vtkStaticDataSetSurfaceFilter::~vtkStaticDataSetSurfaceFilter()
{
}

//-----------------------------------------------------------------------------
int vtkStaticDataSetSurfaceFilter::UnstructuredGridExecute(vtkDataSet* input, vtkPolyData* output)
{
  vtkUnstructuredGrid* inputUG = vtkUnstructuredGrid::SafeDownCast(input);
  if (!inputUG)
  {
    // Rely on superclass for any input which is not a vtkUnstructuredGrid
    return this->Superclass::UnstructuredGridExecute(input, output);
  }

  // Check is cache is still valid
  if (this->InputMeshTime == inputUG->GetMeshMTime() && this->FilterMTime == this->GetMTime())
  {
    // Use cache as base
    output->ShallowCopy(this->Cache.Get());

    // Recover original ids
    vtkPointData* outPD = output->GetPointData();
    vtkCellData* outCD = output->GetCellData();
    vtkIdTypeArray* origPointArray =
      vtkIdTypeArray::SafeDownCast(outPD->GetArray(this->GetOriginalPointIdsName()));
    vtkIdTypeArray* origCellArray =
      vtkIdTypeArray::SafeDownCast(outCD->GetArray(this->GetOriginalCellIdsName()));
    if (!origPointArray || !origCellArray)
    {
      vtkErrorMacro(
        "OriginalPointIds or OriginalCellIds are missing, cannot use static mesh cache");
      return this->Superclass::UnstructuredGridExecute(input, output);
    }

    // Recover input point and cell data
    vtkPointData* inPD = input->GetPointData();
    vtkCellData* inCD = input->GetCellData();

    // Update output point data
    vtkIdType* tmpIds = new vtkIdType[origPointArray->GetNumberOfTuples()];
    memcpy(tmpIds, reinterpret_cast<vtkIdType*>(origPointArray->GetVoidPointer(0)),
      sizeof(vtkIdType) * origPointArray->GetNumberOfTuples());
    vtkNew<vtkIdList> pointIds;
    pointIds->SetArray(tmpIds, origPointArray->GetNumberOfTuples());

    // Remove array that have disappeared from input
    for (int iArr = outPD->GetNumberOfArrays() - 1; iArr >= 0; iArr--)
    {
      vtkAbstractArray* inArr = inPD->GetAbstractArray(outPD->GetArrayName(iArr));
      if (!inArr)
      {
        outPD->RemoveArray(iArr);
      }
    }

    // Update or create arrays present in input
    for (int iArr = 0; iArr < inPD->GetNumberOfArrays(); iArr++)
    {
      vtkAbstractArray* outArr = outPD->GetAbstractArray(inPD->GetArrayName(iArr));
      if (outArr)
      {
        inPD->GetAbstractArray(iArr)->GetTuples(pointIds.Get(), outArr);
      }
      else
      {
        // New array in input, create it in output
        vtkAbstractArray* inArr = inPD->GetAbstractArray(iArr);
        outArr = inArr->NewInstance();
        outArr->SetName(inArr->GetName());
        outArr->SetNumberOfTuples(output->GetNumberOfPoints());
        outArr->SetNumberOfComponents(inArr->GetNumberOfComponents());
        inArr->GetTuples(pointIds.Get(), outArr);
        outPD->AddArray(outArr);
      }
    }

    // Update output cell data
    tmpIds = new vtkIdType[origCellArray->GetNumberOfTuples()];
    memcpy(tmpIds, reinterpret_cast<vtkIdType*>(origCellArray->GetVoidPointer(0)),
      sizeof(vtkIdType) * origCellArray->GetNumberOfTuples());
    vtkNew<vtkIdList> cellIds;
    cellIds->SetArray(tmpIds, origCellArray->GetNumberOfTuples());

    // Remove array that have disappeared from input
    for (int iArr = outCD->GetNumberOfArrays() - 1; iArr >= 0; iArr--)
    {
      vtkAbstractArray* inArr = inCD->GetAbstractArray(outCD->GetArrayName(iArr));
      if (!inArr)
      {
        outCD->RemoveArray(iArr);
      }
    }

    for (int iArr = 0; iArr < inCD->GetNumberOfArrays(); iArr++)
    {
      vtkAbstractArray* outArr = outCD->GetAbstractArray(inCD->GetArrayName(iArr));
      if (outArr)
      {
        inCD->GetAbstractArray(iArr)->GetTuples(cellIds.Get(), outArr);
      }
      else
      {
        // New array in input, create it in output
        vtkAbstractArray* inArr = inCD->GetAbstractArray(iArr);
        outArr = inArr->NewInstance();
        outArr->SetName(inArr->GetName());
        outArr->SetNumberOfTuples(output->GetNumberOfCells());
        outArr->SetNumberOfComponents(inArr->GetNumberOfComponents());
        inArr->GetTuples(cellIds.Get(), outArr);
        outCD->AddArray(outArr);
      }

    }

    // Update output field data
    output->GetFieldData()->ShallowCopy(input->GetFieldData());
    return 1;
  }
  else
  {
    // Cache is not valid, Execute supercall algorithm
    int ret = this->Superclass::UnstructuredGridExecute(input, output);

    // Update the cache with superclass output
    this->Cache->ShallowCopy(output);
    this->InputMeshTime = inputUG->GetMeshMTime();
    this->FilterMTime = this->GetMTime();
    return ret;
  }
}

//----------------------------------------------------------------------------
void vtkStaticDataSetSurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Cache: " << this->Cache << endl;
  os << indent << "Input Mesh Time: " << this->InputMeshTime << endl;
  os << indent << "Filter mTime: " << this->FilterMTime << endl;
}
