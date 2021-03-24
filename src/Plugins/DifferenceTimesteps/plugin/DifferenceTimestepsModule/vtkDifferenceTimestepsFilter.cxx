// Copyright (C) 2014-2021  CEA/DEN, EDF R&D
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
// Author : Maxim Glibin

#include "vtkDifferenceTimestepsFilter.h"

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

// Temporal difference of data array
vtkDataArray* DataTempDiffArray(
  vtkDataArray* theDataArray, vtkIdType theNumComp, vtkIdType theNumTuple, const char* thePrefix)
{
  // Create the new array
  vtkAbstractArray* anAbstractArray = theDataArray->CreateArray(theDataArray->GetDataType());
  vtkDataArray* anOutput = vtkDataArray::SafeDownCast(anAbstractArray);

  // Initialize and appoint a new name
  anOutput->SetNumberOfComponents(theNumComp);
  anOutput->SetNumberOfTuples(theNumTuple);
  std::string aNewName = std::string(thePrefix) + theDataArray->GetName();
  anOutput->SetName(aNewName.c_str());

  return anOutput;
}

// Templated difference function
template <class T>
void vtkTemporalDataDifference(vtkDifferenceTimestepsFilter* /*theDTF*/, vtkDataArray* theOutput,
  vtkDataArray** theArrays, vtkIdType theNumComp, T*)
{
  T* anOutputData = static_cast<T*>(theOutput->GetVoidPointer(0));
  T* anInputData0 = static_cast<T*>(theArrays[0]->GetVoidPointer(0));
  T* anInputData1 = static_cast<T*>(theArrays[1]->GetVoidPointer(0));

  vtkIdType N = theArrays[0]->GetNumberOfTuples();
  for (vtkIdType t = 0; t < N; ++t)
  {
    T* x0 = &anInputData0[t * theNumComp];
    T* x1 = &anInputData1[t * theNumComp];
    for (int c = 0; c < theNumComp; ++c)
    {
      // Compute the difference
      *anOutputData++ = static_cast<T>(x1[c] - x0[c]);
    }
  }
  // Copy component name
  for (int c = 0; c < theNumComp; ++c)
  {
    theOutput->SetComponentName(c, theArrays[0]->GetComponentName(c));
  }
  theOutput->SetNumberOfTuples(N);
}

vtkStandardNewMacro(vtkDifferenceTimestepsFilter)

//--------------------------------------------------------------------------------------------------
vtkDifferenceTimestepsFilter::vtkDifferenceTimestepsFilter()
{
  this->NumberTimeSteps = 0;
  this->RangeIndicesTimeSteps[0] = 0;
  this->RangeIndicesTimeSteps[1] = 0;
  this->FirstTimeStepIndex = 0.0;
  this->SecondTimeStepIndex = 0.0;
  this->TimeStepValues.clear();
  this->ArrayNamePrefix = nullptr;

  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);

  // Set the input data array that the algorithm will process
  this->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
}

//--------------------------------------------------------------------------------------------------
vtkDifferenceTimestepsFilter::~vtkDifferenceTimestepsFilter()
{
  this->TimeStepValues.clear();
  this->SetArrayNamePrefix(nullptr);
}

//--------------------------------------------------------------------------------------------------
void vtkDifferenceTimestepsFilter::PrintSelf(ostream& theOS, vtkIndent theIndent)
{
  this->Superclass::PrintSelf(theOS, theIndent);
  theOS << theIndent << "Number of time steps : " << this->NumberTimeSteps << endl;
  theOS << theIndent << "First time step : " << this->FirstTimeStepIndex << endl;
  theOS << theIndent << "Second time step : " << this->SecondTimeStepIndex << endl;
  theOS << theIndent << "Field association : "
        << vtkDataObject::GetAssociationTypeAsString(this->GetInputFieldAssociation()) << endl;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::FillInputPortInformation(int thePort, vtkInformation* theInfo)
{
  if (thePort == 0)
    theInfo->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");

  return 1;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::FillOutputPortInformation(
  int vtkNotUsed(thePort), vtkInformation* theInfo)
{
  theInfo->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataObject");
  return 1;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::RequestDataObject(vtkInformation* vtkNotUsed(theRequest),
  vtkInformationVector** theInputVector, vtkInformationVector* theOutputVector)
{
  if (this->GetNumberOfInputPorts() == 0 || this->GetNumberOfOutputPorts() == 0)
    return 1;

  vtkInformation* anInputInfo = theInputVector[0]->GetInformationObject(0);
  if (anInputInfo == nullptr)
  {
    vtkErrorMacro(<< "Input information vector is missed.");
    return 0;
  }

  vtkDataObject* anInputObj = anInputInfo->Get(vtkDataObject::DATA_OBJECT());
  if (anInputObj != nullptr)
  {
    // For each output
    for (int i = 0; i < this->GetNumberOfOutputPorts(); ++i)
    {
      vtkInformation* anOutputInfo = theOutputVector->GetInformationObject(i);
      vtkDataObject* anOutputObj = anOutputInfo->Get(vtkDataObject::DATA_OBJECT());
      if (!anOutputObj || !anOutputObj->IsA(anInputObj->GetClassName()))
      {
        vtkDataObject* aNewOutput = anInputObj->NewInstance();
        anOutputInfo->Set(vtkDataObject::DATA_OBJECT(), aNewOutput);
        aNewOutput->Delete();
      }
    }
    return 1;
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::RequestInformation(vtkInformation* vtkNotUsed(theRequest),
  vtkInformationVector** theInputVector, vtkInformationVector* /*theOutputVector*/)
{
  // Get input and output information objects
  vtkInformation* anInInfo = theInputVector[0]->GetInformationObject(0);
  //vtkInformation* anOutInfo = theOutputVector->GetInformationObject(0); // todo: unused

  // Check for presence more than one time step
  if (anInInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
  {
    // Find time on input
    this->NumberTimeSteps = anInInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    if (this->NumberTimeSteps < 2)
    {
      vtkErrorMacro(<< "Not enough numbers of time steps: " << this->NumberTimeSteps);
      return 0;
    }
    // Get time step values
    this->TimeStepValues.resize(this->NumberTimeSteps);
    anInInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &this->TimeStepValues[0]);
    if (this->TimeStepValues.size() == 0)
    {
      vtkErrorMacro(<< "Array of time steps is empty.");
      return 0;
    }
  }
  else
  {
    vtkErrorMacro(<< "No time steps in input data.");
    return 0;
  }

  // Update range of indices of the time steps
  this->RangeIndicesTimeSteps[0] = 0;
  this->RangeIndicesTimeSteps[1] = this->NumberTimeSteps - 1;

  /*
   * RNV: Temporary commented:
   *      This piece of the code removes all time steps from the output object,
   *      but this leads to the strange side effect in the ParaView: time steps also disappears
   *      from the animation scene of the input (parent) object of this filter.
   *      Seems it is a bug of the ParaView, to be investigated ...
   *
  // The output data of this filter has no time associated with it.
  // It is the result of computation difference between two time steps.
  // Unset the time steps
  if (anOutInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    anOutInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_STEPS());

  // Unset the time range
  if(anOutInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()))
    anOutInfo->Remove(vtkStreamingDemandDrivenPipeline::TIME_RANGE());
  */
  return 1;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::RequestUpdateExtent(vtkInformation* /*theRequest*/,
  vtkInformationVector** theInputVector, vtkInformationVector* theOutputVector)
{
  // Get the information objects
  vtkInformation* anInputInfo = theInputVector[0]->GetInformationObject(0);
  vtkInformation* anOutputInfo = theOutputVector->GetInformationObject(0);

  // Indices must not go beyond the range of indices of the time steps
  if (this->FirstTimeStepIndex >= this->NumberTimeSteps || this->FirstTimeStepIndex < 0)
  {
    vtkErrorMacro(<< "Specified index of the first time step [" << this->FirstTimeStepIndex
                  << "] is outside the range of indices.");
    return 0;
  }
  if (this->SecondTimeStepIndex >= this->NumberTimeSteps || this->SecondTimeStepIndex < 0)
  {
    vtkErrorMacro(<< "Specified index of the second time step [" << this->SecondTimeStepIndex
                  << "] is outside the range of indices.");
    return 0;
  }

  // Warn if the selected time steps are equal
  if (this->FirstTimeStepIndex == this->SecondTimeStepIndex)
  {
    vtkWarningMacro(<< "First and second indices [" << this->FirstTimeStepIndex << " = "
                    << this->SecondTimeStepIndex << "] are the same.");
  }

  // Find the required input time steps and request them
  if (anOutputInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
  {
    // Get the available input times
    double* anInputTimes = anInputInfo->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
    if (anInputTimes != nullptr)
    {
      // Compute the requested times
      double anInputUpdateTimes[2];
      int aNumberInputUpdateTimes(0);

      // For each the requested time mark the required input times
      anInputUpdateTimes[aNumberInputUpdateTimes++] = anInputTimes[this->FirstTimeStepIndex];
      anInputUpdateTimes[aNumberInputUpdateTimes++] = anInputTimes[this->SecondTimeStepIndex];

      // Make the multiple time requests upstream and use set of time-stamped data
      // objects are stored in time order in a vtkMultiBlockDataSet object
      anInputInfo->Set(vtkMultiTimeStepAlgorithm::UPDATE_TIME_STEPS(), anInputUpdateTimes,
        aNumberInputUpdateTimes);
    }
  }
  return 1;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::RequestData(vtkInformation* vtkNotUsed(theRequest),
  vtkInformationVector** theInputVector, vtkInformationVector* theOutputVector)
{
  // Get the information objects
  vtkInformation* anInputInfo = theInputVector[0]->GetInformationObject(0);
  vtkInformation* anOutputInfo = theOutputVector->GetInformationObject(0);

  vtkDataObject* anOutputDataObj = nullptr;

  vtkMultiBlockDataSet* anInputData =
    vtkMultiBlockDataSet::SafeDownCast(anInputInfo->Get(vtkDataObject::DATA_OBJECT()));

  int aNumberTimeSteps = anInputData->GetNumberOfBlocks();
  if (aNumberTimeSteps == 2)
  {
    // Get data objects
    vtkDataObject* aData0 = anInputData->GetBlock(0);
    vtkDataObject* aData1 = anInputData->GetBlock(1);
    if (aData0 == nullptr && aData1 == nullptr)
    {
      vtkErrorMacro(<< "Null data set.");
      return 0;
    }

    // Compute difference between two objects
    anOutputDataObj = this->DifferenceDataObject(aData0, aData1);
    anOutputInfo->Set(vtkDataObject::DATA_OBJECT(), anOutputDataObj);
    if (anOutputDataObj != nullptr)
      anOutputDataObj->Delete();
  }
  else
  {
    vtkErrorMacro(<< "The amount of time blocks is not correct: " << aNumberTimeSteps);
    return 0;
  }

  return 1;
}

//--------------------------------------------------------------------------------------------------
vtkDataObject* vtkDifferenceTimestepsFilter::DifferenceDataObject(
  vtkDataObject* theInput1, vtkDataObject* theInput2)
{
  // Determine the input object type
  if (theInput1->IsA("vtkDataSet"))
  {
    vtkDataSet* anInDataSet1 = vtkDataSet::SafeDownCast(theInput1);
    vtkDataSet* anInDataSet2 = vtkDataSet::SafeDownCast(theInput2);
    return this->DifferenceDataSet(anInDataSet1, anInDataSet2);
  }
  else if (theInput1->IsA("vtkCompositeDataSet"))
  {
    // It is essential that aMGDataSet[0] an aMGDataSet[1] has the same structure.
    vtkCompositeDataSet* aMGDataSet[2];
    aMGDataSet[0] = vtkCompositeDataSet::SafeDownCast(theInput1);
    aMGDataSet[1] = vtkCompositeDataSet::SafeDownCast(theInput2);

    vtkCompositeDataSet* anOutput = aMGDataSet[0]->NewInstance();
    anOutput->CopyStructure(aMGDataSet[0]);

    vtkSmartPointer<vtkCompositeDataIterator> anIter;
    anIter.TakeReference(aMGDataSet[0]->NewIterator());
    for (anIter->InitTraversal(); !anIter->IsDoneWithTraversal(); anIter->GoToNextItem())
    {
      vtkDataObject* aDataObj1 = anIter->GetCurrentDataObject();
      vtkDataObject* aDataObj2 = aMGDataSet[1]->GetDataSet(anIter);
      if (aDataObj1 == nullptr || aDataObj2 == nullptr)
      {
        vtkWarningMacro("The composite datasets were not identical in structure.");
        continue;
      }

      vtkDataObject* aResultDObj = this->DifferenceDataObject(aDataObj1, aDataObj2);
      if (aResultDObj != nullptr)
      {
        anOutput->SetDataSet(anIter, aResultDObj);
        aResultDObj->Delete();
      }
      else
      {
        vtkErrorMacro(<< "Unexpected error during computation of the difference.");
        return nullptr;
      }
    }
    return anOutput;
  }
  else
  {
    vtkErrorMacro("We cannot yet compute difference of this type of dataset.");
    return nullptr;
  }
}

//--------------------------------------------------------------------------------------------------
vtkDataSet* vtkDifferenceTimestepsFilter::DifferenceDataSet(
  vtkDataSet* theInput1, vtkDataSet* theInput2)
{
  vtkDataSet* anInput[2];
  anInput[0] = theInput1;
  anInput[1] = theInput2;

  // Copy input structure into output
  vtkDataSet* anOutput = anInput[0]->NewInstance();
  anOutput->CopyStructure(anInput[0]);

  std::vector<vtkDataArray*> anArrays;
  vtkDataArray* anOutputArray;

  // Compute the difference of the the specified point or cell data array
  vtkDataArray* aDataArray0 = this->GetInputArrayToProcess(0, anInput[0]);
  vtkDataArray* aDataArray1 = this->GetInputArrayToProcess(0, anInput[1]);
  if (aDataArray0 == nullptr || aDataArray1 == nullptr)
  {
    vtkErrorMacro(<< "Input array to process is empty.");
    return nullptr;
  }
  anArrays.push_back(aDataArray0);
  anArrays.push_back(aDataArray1);

  if (anArrays.size() > 1)
  {
    if (!this->VerifyArrays(&anArrays[0], 2))
    {
      vtkErrorMacro(<< "Verification of data arrays has failed.");
      return nullptr;
    }

    anOutputArray = this->DifferenceDataArray(&anArrays[0], anArrays[0]->GetNumberOfTuples());
    // Determine a field association
    int aTypeFieldAssociation = this->GetInputFieldAssociation();
    if (aTypeFieldAssociation == vtkDataObject::FIELD_ASSOCIATION_POINTS)
    {
      // For point data
      anOutput->GetPointData()->AddArray(anOutputArray);
    }
    else if (aTypeFieldAssociation == vtkDataObject::FIELD_ASSOCIATION_CELLS)
    {
      // For cell data
      anOutput->GetCellData()->AddArray(anOutputArray);
    }
    else
    {
      vtkErrorMacro(<< "Solution is not implemeted yet.");
      return nullptr;
    }
    anOutputArray->Delete();
    anArrays.clear();
  }

  return anOutput;
}

//--------------------------------------------------------------------------------------------------
vtkDataArray* vtkDifferenceTimestepsFilter::DifferenceDataArray(
  vtkDataArray** theArrays, vtkIdType theNumTuple)
{
  // Create the output array based on the number of tuple and components
  // with a new name containing the specified prefix
  int aNumComp = theArrays[0]->GetNumberOfComponents();
  vtkDataArray* anOutput =
    DataTempDiffArray(theArrays[0], aNumComp, theNumTuple, this->ArrayNamePrefix);

  // Now do the computation of the difference
  switch (theArrays[0]->GetDataType())
  {
    vtkTemplateMacro(
      vtkTemporalDataDifference(this, anOutput, theArrays, aNumComp, static_cast<VTK_TT*>(0)));
    default:
      vtkErrorMacro(<< "Execute: unknown scalar type.");
      return nullptr;
  }

  return anOutput;
}

//--------------------------------------------------------------------------------------------------
int vtkDifferenceTimestepsFilter::GetInputFieldAssociation()
{
  vtkInformationVector* anInputArrayVec = this->GetInformation()->Get(INPUT_ARRAYS_TO_PROCESS());
  vtkInformation* anInputArrayInfo = anInputArrayVec->GetInformationObject(0);
  return anInputArrayInfo->Get(vtkDataObject::FIELD_ASSOCIATION());
}

//--------------------------------------------------------------------------------------------------
bool vtkDifferenceTimestepsFilter::VerifyArrays(vtkDataArray** theArrays, int theNumArrays)
{
  // Get all required data to compare with other
  const char* anArrayName = theArrays[0]->GetName();
  vtkIdType aNumTuples = theArrays[0]->GetNumberOfTuples();
  vtkIdType aNumComponents = theArrays[0]->GetNumberOfComponents();

  for (int i = 1; i < theNumArrays; ++i)
  {
    if (strcmp(theArrays[i]->GetName(), anArrayName) != 0)
    {
      vtkWarningMacro(<< "Computation of difference aborted for dataset because "
                      << "the array name in each time step are different.");
      return false;
    }

    if (theArrays[i]->GetNumberOfTuples() != aNumTuples)
    {
      vtkWarningMacro(<< "Computation of difference aborted for dataset because "
                      << "the number of tuples in each time step are different.");
      return false;
    }

    if (theArrays[i]->GetNumberOfComponents() != aNumComponents)
    {
      vtkWarningMacro(<< "Computation of difference aborted for dataset because "
                      << "the number of components in each time step are different.");
      return false;
    }
  }

  return true;
}
