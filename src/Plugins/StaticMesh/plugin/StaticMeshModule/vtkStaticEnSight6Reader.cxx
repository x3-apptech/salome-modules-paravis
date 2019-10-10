/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticEnSight6Reader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStaticEnSight6Reader.h"

#include <vtkDataArray.h>
#include <vtkDataArrayCollection.h>
#include <vtkIdList.h>
#include <vtkIdListCollection.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkStaticEnSight6Reader);

//----------------------------------------------------------------------------
int vtkStaticEnSight6Reader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkDebugMacro("In execute ");

  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int tsLength =
    outInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  double* steps =
    outInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  this->ActualTimeValue = this->TimeValue;

  // Check if a particular time was requested by the pipeline.
  // This overrides the ivar.
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) && tsLength>0)
  {
    // Get the requested time step. We only support requests of a single time
    // step in this reader right now
    double requestedTimeStep =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());

    // find the first time value larger than requested time value
    // this logic could be improved
    int cnt = 0;
    while (cnt < tsLength-1 && steps[cnt] < requestedTimeStep)
    {
      cnt++;
    }
    this->ActualTimeValue = steps[cnt];
  }

  vtkDebugMacro("Executing with: " << this->ActualTimeValue);

  if (this->CacheMTime < this->GetMTime())
  {
    int i, timeSet, fileSet, timeStep, timeStepInFile, fileNum;
    vtkDataArray *times;
    vtkIdList *numStepsList, *filenameNumbers;
    float newTime;
    int numSteps;
    char* fileName;
    int filenameNum;

    if ( ! this->CaseFileRead)
    {
      vtkErrorMacro("error reading case file");
      return 0;
    }

    this->NumberOfNewOutputs = 0;
    this->NumberOfGeometryParts = 0;
    if (this->GeometryFileName)
    {
      timeStep = timeStepInFile = 1;
      fileNum = 1;
      fileName = new char[strlen(this->GeometryFileName) + 10];
      strcpy(fileName, this->GeometryFileName);

      if (this->UseTimeSets)
      {
        timeSet = this->TimeSetIds->IsId(this->GeometryTimeSet);
        if (timeSet >= 0)
        {
          times = this->TimeSets->GetItem(timeSet);
          this->GeometryTimeValue = times->GetComponent(0, 0);
          for (i = 1; i < times->GetNumberOfTuples(); i++)
          {
            newTime = times->GetComponent(i, 0);
            if (newTime <= this->ActualTimeValue &&
                newTime > this->GeometryTimeValue)
            {
              this->GeometryTimeValue = newTime;
              timeStep++;
              timeStepInFile++;
            }
          }
          if (this->TimeSetFileNameNumbers->GetNumberOfItems() > 0)
          {
            int collectionNum = this->TimeSetsWithFilenameNumbers->
              IsId(this->GeometryTimeSet);
            if (collectionNum > -1)
            {
              filenameNumbers =
                this->TimeSetFileNameNumbers->GetItem(collectionNum);
              filenameNum = filenameNumbers->GetId(timeStep-1);
              if (! this->UseFileSets)
              {
                this->ReplaceWildcards(fileName, filenameNum);
              }
            }
          }

          // There can only be file sets if there are also time sets.
          if (this->UseFileSets)
          {
            fileSet = this->FileSets->IsId(this->GeometryFileSet);
            numStepsList = static_cast<vtkIdList*>(this->FileSetNumberOfSteps->
                                                   GetItemAsObject(fileSet));

            if (timeStep > numStepsList->GetId(0))
            {
              numSteps = numStepsList->GetId(0);
              timeStepInFile -= numSteps;
              fileNum = 2;
              for (i = 1; i < numStepsList->GetNumberOfIds(); i++)
              {
                numSteps += numStepsList->GetId(i);
                if (timeStep > numSteps)
                {
                  fileNum++;
                  timeStepInFile -= numStepsList->GetId(i);
                }
              }
            }
            if (this->FileSetFileNameNumbers->GetNumberOfItems() > 0)
            {
              int collectionNum = this->FileSetsWithFilenameNumbers->
                IsId(this->GeometryFileSet);
              if (collectionNum > -1)
              {
                filenameNumbers = this->FileSetFileNameNumbers->
                  GetItem(collectionNum);
                filenameNum = filenameNumbers->GetId(fileNum-1);
                this->ReplaceWildcards(fileName, filenameNum);
              }
            }
          }
        }
      }

      if (!this->ReadGeometryFile(fileName, timeStepInFile, this->Cache))
      {
        vtkErrorMacro("error reading geometry file");
        delete [] fileName;
        return 0;
      }

      delete [] fileName;
    }
    if (this->MeasuredFileName)
    {
      timeStep = timeStepInFile = 1;
      fileNum = 1;
      fileName = new char[strlen(this->MeasuredFileName) + 10];
      strcpy(fileName, this->MeasuredFileName);

      if (this->UseTimeSets)
      {
        timeSet = this->TimeSetIds->IsId(this->MeasuredTimeSet);
        if (timeSet >= 0)
        {
          times = this->TimeSets->GetItem(timeSet);
          this->MeasuredTimeValue = times->GetComponent(0, 0);
          for (i = 1; i < times->GetNumberOfTuples(); i++)
          {
            newTime = times->GetComponent(i, 0);
            if (newTime <= this->ActualTimeValue &&
                newTime > this->MeasuredTimeValue)
            {
              this->MeasuredTimeValue = newTime;
              timeStep++;
              timeStepInFile++;
            }
          }
          if (this->TimeSetFileNameNumbers->GetNumberOfItems() > 0)
          {
            int collectionNum = this->TimeSetsWithFilenameNumbers->
              IsId(this->MeasuredTimeSet);
            if (collectionNum > -1)
            {
              filenameNumbers = this->TimeSetFileNameNumbers->
                GetItem(collectionNum);
              filenameNum = filenameNumbers->GetId(timeStep-1);
              if (! this->UseFileSets)
              {
                this->ReplaceWildcards(fileName, filenameNum);
              }
            }
          }

          // There can only be file sets if there are also time sets.
          if (this->UseFileSets)
          {
            fileSet = this->FileSets->IsId(this->MeasuredFileSet);
            numStepsList = static_cast<vtkIdList*>(this->FileSetNumberOfSteps->
                                                   GetItemAsObject(fileSet));

            if (timeStep > numStepsList->GetId(0))
            {
              numSteps = numStepsList->GetId(0);
              timeStepInFile -= numSteps;
              fileNum = 2;
              for (i = 1; i < numStepsList->GetNumberOfIds(); i++)
              {
                numSteps += numStepsList->GetId(i);
                if (timeStep > numSteps)
                {
                  fileNum++;
                  timeStepInFile -= numStepsList->GetId(i);
                }
              }
            }
            if (this->FileSetFileNameNumbers->GetNumberOfItems() > 0)
            {
              int collectionNum = this->FileSetsWithFilenameNumbers->
                IsId(this->MeasuredFileSet);
              if (collectionNum > -1)
              {
                filenameNumbers = this->FileSetFileNameNumbers->
                  GetItem(fileSet);
                filenameNum = filenameNumbers->GetId(fileNum-1);
                this->ReplaceWildcards(fileName, filenameNum);
              }
            }
          }
        }
      }
      if (!this->ReadMeasuredGeometryFile(fileName, timeStepInFile, this->Cache))
      {
        vtkErrorMacro("error reading measured geometry file");
        delete [] fileName;
        return 0;
      }
      delete [] fileName;
    }
    this->CacheMTime.Modified();
  }
  output->ShallowCopy(this->Cache);

  if ((this->NumberOfVariables + this->NumberOfComplexVariables) > 0)
  {
    if (!this->ReadVariableFiles(output))
    {
      vtkErrorMacro("error reading variable files");
      return 0;
    }
  }

  return 1;
}
