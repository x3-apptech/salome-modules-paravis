/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRTAnalyticSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkTemporalUGWavelet.h"

#include "vtkCellData.h"
#include "vtkDataSetTriangleFilter.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiProcessController.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkRTAnalyticSource.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkTemporalUGWavelet);

// ----------------------------------------------------------------------------
vtkTemporalUGWavelet::vtkTemporalUGWavelet()
{
  this->Cache = vtkUnstructuredGrid::New();
  this->NumberOfTimeSteps = 10;
}

// ----------------------------------------------------------------------------
vtkTemporalUGWavelet::~vtkTemporalUGWavelet()
{
  this->Cache->Delete();
}

//----------------------------------------------------------------------------
int vtkTemporalUGWavelet::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkUnstructuredGrid");
  return 1;
}

// ----------------------------------------------------------------------------
int vtkTemporalUGWavelet::RequestInformation(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  double range[2] = { 0, static_cast<double>(this->NumberOfTimeSteps - 1) };
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), range, 2);
  double* outTimes = new double[this->NumberOfTimeSteps];
  for (int i = 0; i < this->NumberOfTimeSteps; i++)
  {
    outTimes[i] = i;
  }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), outTimes, this->NumberOfTimeSteps);
  outInfo->Set(CAN_HANDLE_PIECE_REQUEST(), 1);
  return Superclass::RequestInformation(request, inputVector, outputVector);
}

// ----------------------------------------------------------------------------
int vtkTemporalUGWavelet::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* data = vtkUnstructuredGrid::GetData(outInfo);
  if (this->CacheMTime < this->GetMTime())
  {
    // Create an output vector to recover the output image data
    // RequestData method
    vtkNew<vtkInformationVector> tmpOutputVec;
    vtkNew<vtkImageData> image;
    tmpOutputVec->Copy(outputVector, 1);
    vtkInformation* tmpOutInfo = tmpOutputVec->GetInformationObject(0);
    tmpOutInfo->Set(vtkDataObject::DATA_OBJECT(), image.Get());

    // Generate wavelet
    int ret = this->Superclass::RequestData(request, inputVector, tmpOutputVec.Get());
    if (ret == 0)
    {
      return ret;
    }

    // Transform it to unstructured grid
    vtkNew<vtkDataSetTriangleFilter> tetra;
    tetra->SetInputData(image.Get());
    tetra->Update();

    // Create the cache
    this->Cache->ShallowCopy(tetra->GetOutput());
    this->CacheMTime.Modified();
  }

  // Use the cache
  data->ShallowCopy(this->Cache);

  double t = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());

  // Generate tPoint array
  vtkIdType nbPoints = data->GetNumberOfPoints();
  vtkNew<vtkFloatArray> pointArray;
  pointArray->SetName("tPoint");
  pointArray->SetNumberOfValues(nbPoints);
  data->GetPointData()->AddArray(pointArray.Get());
  for (vtkIdType i = 0; i < nbPoints; i++)
  {
    pointArray->SetValue(
      i, static_cast<int>(i + t * (nbPoints / this->NumberOfTimeSteps)) % nbPoints);
  }

  // Generate tCell array
  vtkIdType nbCells = data->GetNumberOfCells();
  vtkNew<vtkFloatArray> cellArray;
  cellArray->SetName("tCell");
  cellArray->SetNumberOfValues(nbCells);
  data->GetCellData()->AddArray(cellArray.Get());

  for (vtkIdType i = 0; i < nbCells; i++)
  {
    cellArray->SetValue(i, static_cast<int>(i + t * (nbCells / this->NumberOfTimeSteps)) % nbCells);
  }

  return 1;
}

// ----------------------------------------------------------------------------
void vtkTemporalUGWavelet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfTimeSteps: " << this->NumberOfTimeSteps << endl;
}
