/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticPUnstructuredGridGhostCellsGenerator.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkStaticPUnstructuredGridGhostCellsGenerator.h"

#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkIdFilter.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkMPIController.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkProcessIdScalars.h>
#include <vtkTable.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>

static const int SUGGCG_SIZE_EXCHANGE_TAG = 9002;
static const int SUGGCG_DATA_EXCHANGE_TAG = 9003;

vtkStandardNewMacro(vtkStaticPUnstructuredGridGhostCellsGenerator);

//----------------------------------------------------------------------------
vtkStaticPUnstructuredGridGhostCellsGenerator::vtkStaticPUnstructuredGridGhostCellsGenerator()
{
  this->InputMeshTime = 0;
  this->FilterMTime = 0;

  vtkMPIController* controller =
    vtkMPIController::SafeDownCast(vtkMultiProcessController::GetGlobalController());
  if (controller)
  {
    // Initialise vtkIdList vectors
    this->GhostPointsToReceive.resize(controller->GetNumberOfProcesses());
    this->GhostPointsToSend.resize(controller->GetNumberOfProcesses());
    this->GhostCellsToReceive.resize(controller->GetNumberOfProcesses());
    this->GhostCellsToSend.resize(controller->GetNumberOfProcesses());

    int nProc = controller->GetNumberOfProcesses();

    for (int i = 0; i < nProc; i++)
    {
      this->GhostCellsToReceive[i] = vtkSmartPointer<vtkIdList>::New();
      this->GhostCellsToSend[i] = vtkSmartPointer<vtkIdList>::New();
      this->GhostPointsToReceive[i] = vtkSmartPointer<vtkIdList>::New();
      this->GhostPointsToSend[i] = vtkSmartPointer<vtkIdList>::New();
    }
  }
}

//----------------------------------------------------------------------------
vtkStaticPUnstructuredGridGhostCellsGenerator::~vtkStaticPUnstructuredGridGhostCellsGenerator()
{
}

//-----------------------------------------------------------------------------
int vtkStaticPUnstructuredGridGhostCellsGenerator::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the inputs and outputs
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkUnstructuredGridBase* input =
    vtkUnstructuredGridBase::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkUnstructuredGrid* output =
    vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Recover the static unstructured grid
  vtkUnstructuredGrid* inputUG = vtkUnstructuredGrid::SafeDownCast(input);
  if (!inputUG)
  {
    // For any other types of input, fall back to superclass implementation
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  // Check cache validity
  if (this->InputMeshTime == inputUG->GetMeshMTime() && this->FilterMTime == this->GetMTime())
  {
    // Cache mesh is up to date, use it to generate data
    // Update the cache data
    this->UpdateCacheData(input);

    // Copy the updated cache into the output
    output->ShallowCopy(this->Cache.Get());
    return 1;
  }
  else
  {
    // Add Arrays Ids needed
    vtkNew<vtkUnstructuredGrid> tmpInput;
    this->AddIdsArrays(input, tmpInput.Get());

    // Create an input vector to pass the completed input to the superclass
    // RequestData method
    vtkNew<vtkInformationVector> tmpInputVec;
    tmpInputVec->Copy(inputVector[0], 1);
    vtkInformation* tmpInInfo = tmpInputVec->GetInformationObject(0);
    tmpInInfo->Set(vtkDataObject::DATA_OBJECT(), tmpInput.Get());
    vtkInformationVector* tmpInputVecPt = tmpInputVec.Get();
    int ret = this->Superclass::RequestData(request, &tmpInputVecPt, outputVector);

    // Update the cache with superclass output
    this->Cache->ShallowCopy(output);
    this->InputMeshTime = inputUG->GetMeshMTime();
    this->FilterMTime = this->GetMTime();

    this->ProcessGhostIds();

    return ret;
  }
}

//-----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::ProcessGhostIds()
{
  vtkMPIController* controller =
    vtkMPIController::SafeDownCast(vtkMultiProcessController::GetGlobalController());
  if (controller)
  {
    int nProc = controller->GetNumberOfProcesses();
    int rank = controller->GetLocalProcessId();

    // Clear ghost vectors
    for (int i = 0; i < nProc; i++)
    {
      this->GhostCellsToReceive[i]->SetNumberOfIds(0);
      this->GhostCellsToSend[i]->SetNumberOfIds(0);
      this->GhostPointsToReceive[i]->SetNumberOfIds(0);
      this->GhostPointsToSend[i]->SetNumberOfIds(0);
    }

    // Recover all needed data arrays
    vtkPointData* cachePD = this->Cache->GetPointData();
    vtkCellData* cacheCD = this->Cache->GetCellData();
    vtkUnsignedCharArray* pointGhostArray = this->Cache->GetPointGhostArray();
    vtkUnsignedCharArray* cellGhostArray = this->Cache->GetCellGhostArray();
    vtkIdTypeArray* pointIds = vtkIdTypeArray::SafeDownCast(cachePD->GetAbstractArray("Ids"));
    vtkIdTypeArray* cellIds = vtkIdTypeArray::SafeDownCast(cacheCD->GetAbstractArray("Ids"));
    vtkIntArray* pointProcIds = vtkIntArray::SafeDownCast(cachePD->GetAbstractArray("ProcessId"));
    vtkIntArray* cellProcIds = vtkIntArray::SafeDownCast(cacheCD->GetAbstractArray("ProcessId"));
    if (!pointGhostArray || !pointIds || !pointProcIds || !cellGhostArray || !cellIds ||
      !cellProcIds)
    {
      // Sanity check
      vtkWarningMacro("Arrays are missing from cache, cache is discarded");
      this->InputMeshTime = 0;
      this->FilterMTime = 0;
    }
    else
    {
      // Compute list of remote ghost point ids
      // and corresponding local point ids.
      std::vector<std::vector<vtkIdType> > remoteGhostPoints;
      remoteGhostPoints.resize(nProc);
      for (vtkIdType i = 0; i < pointGhostArray->GetNumberOfTuples(); i++)
      {
        if (pointGhostArray->GetValue(i) != 0)
        {
          this->GhostPointsToReceive[pointProcIds->GetValue(i)]->InsertNextId(i);
          remoteGhostPoints[pointProcIds->GetValue(i)].push_back(pointIds->GetValue(i));
        }
      }

      // Compute list of remote ghost cell ids
      // and corresponding local cell ids.
      std::vector<std::vector<vtkIdType> > remoteGhostCells;
      remoteGhostCells.resize(nProc);
      for (vtkIdType i = 0; i < cellGhostArray->GetNumberOfTuples(); i++)
      {
        if (cellGhostArray->GetValue(i) != 0)
        {
          this->GhostCellsToReceive[cellProcIds->GetValue(i)]->InsertNextId(i);
          remoteGhostCells[cellProcIds->GetValue(i)].push_back(cellIds->GetValue(i));
        }
      }

      // Send requested ghost point ids to their own rank
      vtkMPICommunicator::Request pointSizeReqs[nProc];
      vtkMPICommunicator::Request pointIdsReqs[nProc];
      vtkIdType lengths[nProc];
      for (int i = 0; i < nProc; i++)
      {
        if (i != rank)
        {
          lengths[i] = remoteGhostPoints[i].size();
          controller->NoBlockSend(&lengths[i], 1, i, SUGGCG_SIZE_EXCHANGE_TAG, pointSizeReqs[i]);
          controller->NoBlockSend(&remoteGhostPoints[i][0], remoteGhostPoints[i].size(), i,
            SUGGCG_DATA_EXCHANGE_TAG, pointIdsReqs[i]);
        }
      }

      // Receive and store requested ghost point ids.
      for (int i = 0; i < nProc; i++)
      {
        if (i != rank)
        {
          vtkIdType length;
          controller->Receive(&length, 1, i, SUGGCG_SIZE_EXCHANGE_TAG);
          this->GhostPointsToSend[i]->SetNumberOfIds(length);
          controller->Receive(
            this->GhostPointsToSend[i]->GetPointer(0), length, i, SUGGCG_DATA_EXCHANGE_TAG);
        }
      }
      controller->Barrier();

      // Send requested ghost cell ids to their own rank
      vtkMPICommunicator::Request cellSizeReqs[nProc];
      vtkMPICommunicator::Request cellIdsReqs[nProc];
      for (int i = 0; i < nProc; i++)
      {
        if (i != rank)
        {
          lengths[i] = remoteGhostCells[i].size();
          controller->NoBlockSend(&lengths[i], 1, i, SUGGCG_SIZE_EXCHANGE_TAG, cellSizeReqs[i]);
          controller->NoBlockSend(
            &remoteGhostCells[i][0], lengths[i], i, SUGGCG_DATA_EXCHANGE_TAG, cellIdsReqs[i]);
        }
      }
      // Receive and store requested ghost cell ids.
      for (int i = 0; i < nProc; i++)
      {
        if (i != rank)
        {
          vtkIdType length;
          controller->Receive(&length, 1, i, SUGGCG_SIZE_EXCHANGE_TAG);
          this->GhostCellsToSend[i]->SetNumberOfIds(length);
          controller->Receive(
            this->GhostCellsToSend[i]->GetPointer(0), length, i, SUGGCG_DATA_EXCHANGE_TAG);
        }
      }
      controller->Barrier();
    }
  }
}

//-----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::AddIdsArrays(
  vtkDataSet* input, vtkDataSet* output)
{
  vtkDataSet* tmpInput;
  tmpInput = input;
  vtkNew<vtkIdFilter> generateIdScalars;
  vtkNew<vtkProcessIdScalars> processPointIdScalars;
  vtkNew<vtkProcessIdScalars> processCellIdScalars;

  // Check for Ids array
  vtkAbstractArray* pointIdsTmp = input->GetPointData()->GetAbstractArray("Ids");
  vtkAbstractArray* cellIdsTmp = input->GetCellData()->GetAbstractArray("Ids");
  if (!pointIdsTmp || !cellIdsTmp)
  {
    // Create Ids array
    generateIdScalars->SetInputData(tmpInput);
    generateIdScalars->SetIdsArrayName("Ids");
    generateIdScalars->Update();
    tmpInput = generateIdScalars->GetOutput();
  }

  // Check for ProcessId point array
  vtkAbstractArray* procIdsTmp = input->GetPointData()->GetAbstractArray("ProcessId");
  if (!procIdsTmp)
  {
    // Create ProcessId point Array
    processPointIdScalars->SetInputData(tmpInput);
    processPointIdScalars->SetScalarModeToPointData();
    processPointIdScalars->Update();
    tmpInput = processPointIdScalars->GetOutput();
  }

  // Check for ProcessId Cell Array
  procIdsTmp = input->GetCellData()->GetAbstractArray("ProcessId");
  if (!procIdsTmp)
  {
    // Create ProcessId Cell array
    vtkNew<vtkProcessIdScalars> processIdScalars;
    processCellIdScalars->SetInputData(tmpInput);
    processCellIdScalars->SetScalarModeToCellData();
    processCellIdScalars->Update();
    tmpInput = processCellIdScalars->GetOutput();
  }
  output->ShallowCopy(tmpInput);
}

//-----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::UpdateCacheData(vtkDataSet* input)
{
  this->UpdateCacheDataWithInput(input);
  this->UpdateCacheGhostCellAndPointData(input);
}

//-----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::UpdateCacheDataWithInput(vtkDataSet* input)
{
  // Recover point and cell data
  vtkPointData* cachePD = this->Cache->GetPointData();
  vtkCellData* cacheCD = this->Cache->GetCellData();
  vtkPointData* inPD = input->GetPointData();
  vtkCellData* inCD = input->GetCellData();

  // Update cache point data using input point data
  // Of course this concerns only non-ghost points
  for (int i = 0; i < inPD->GetNumberOfArrays(); i++)
  {
    vtkAbstractArray* cacheArray = cachePD->GetAbstractArray(inPD->GetArrayName(i));
    if (cacheArray)
    {
      cacheArray->InsertTuples(0, input->GetNumberOfPoints(), 0, inPD->GetAbstractArray(i));
    }
  }

  // Update cache cell data using input cell data
  // Of course this concerns only non-ghost cells
  for (int i = 0; i < inCD->GetNumberOfArrays(); i++)
  {
    vtkAbstractArray* cacheArray = cacheCD->GetAbstractArray(inCD->GetArrayName(i));
    if (cacheArray)
    {
      cacheArray->InsertTuples(0, input->GetNumberOfCells(), 0, inCD->GetAbstractArray(i));
    }
  }

  // Update field data
  this->Cache->GetFieldData()->ShallowCopy(input->GetFieldData());
}

//-----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::UpdateCacheGhostCellAndPointData(
  vtkDataSet* input)
{
  vtkMPIController* controller =
    vtkMPIController::SafeDownCast(vtkMultiProcessController::GetGlobalController());
  if (controller)
  {
    vtkPointData* cachePD = this->Cache->GetPointData();
    vtkCellData* cacheCD = this->Cache->GetCellData();
    vtkPointData* inPD = input->GetPointData();
    vtkCellData* inCD = input->GetCellData();

    int nProc = controller->GetNumberOfProcesses();
    int rank = controller->GetLocalProcessId();

    vtkNew<vtkCharArray> buffers[nProc];
    vtkIdType lengths[nProc];
    vtkMPICommunicator::Request sizeReqs[nProc];
    vtkMPICommunicator::Request dataReqs[nProc];

    // For each rank
    for (int i = 0; i < nProc; i++)
    {
      if (i != rank && this->GhostPointsToSend[i]->GetNumberOfIds() > 0)
      {
        // Prepare ghost points point data to send it as a table
        vtkNew<vtkPointData> ghostPointData;
        ghostPointData->CopyAllocate(inPD);

        // Prepare a list of iterating ids and copy all ghost point ids
        // for this rank into the ghostPointData
        vtkNew<vtkIdList> dumStaticPointIds;
        dumStaticPointIds->SetNumberOfIds(this->GhostPointsToSend[i]->GetNumberOfIds());
        for (vtkIdType id = 0; id < this->GhostPointsToSend[i]->GetNumberOfIds(); id++)
        {
          dumStaticPointIds->SetId(id, id);
        }
        ghostPointData->CopyData(inPD, this->GhostPointsToSend[i].Get(), dumStaticPointIds.Get());

        // Add each point data array to a dumStatic table
        vtkNew<vtkTable> pointDataTable;
        for (int iArr = 0; iArr < ghostPointData->GetNumberOfArrays(); iArr++)
        {
          pointDataTable->AddColumn(ghostPointData->GetArray(iArr));
        }

        // Marshall the table and transfer it to rank
        vtkCommunicator::MarshalDataObject(pointDataTable.Get(), buffers[i].Get());
        lengths[i] = buffers[i]->GetNumberOfTuples();
        controller->NoBlockSend(&lengths[i], 1, i, SUGGCG_SIZE_EXCHANGE_TAG, sizeReqs[i]);
        controller->NoBlockSend((char*)(buffers[i]->GetVoidPointer(0)), lengths[i], i,
          SUGGCG_DATA_EXCHANGE_TAG, dataReqs[i]);
      }
    }
    // Foe each rank
    for (int i = 0; i < nProc; i++)
    {
      if (i != rank && this->GhostPointsToReceive[i]->GetNumberOfIds() > 0)
      {
        // Receive dumStatic table to unmarshall
        vtkIdType length;
        controller->Receive(&length, 1, i, SUGGCG_SIZE_EXCHANGE_TAG);

        vtkNew<vtkCharArray> recvBuffer;
        recvBuffer->SetNumberOfValues(length);
        controller->Receive(
          (char*)(recvBuffer->GetVoidPointer(0)), length, i, SUGGCG_DATA_EXCHANGE_TAG);
        vtkNew<vtkTable> pointDataTable;
        vtkCommunicator::UnMarshalDataObject(recvBuffer.Get(), pointDataTable.Get());

        // Create a dumStatic iterating point ids
        vtkNew<vtkIdList> dumStaticPointIds;
        dumStaticPointIds->SetNumberOfIds(this->GhostPointsToReceive[i]->GetNumberOfIds());
        for (vtkIdType id = 0; id < this->GhostPointsToReceive[i]->GetNumberOfIds(); id++)
        {
          dumStaticPointIds->SetId(id, id);
        }

        // Copy the tuples of each array from the dumStatic table
        // into the ghost point data
        for (int iArr = 0; iArr < pointDataTable->GetNumberOfColumns(); iArr++)
        {
          vtkAbstractArray* arrayToCopyIn =
            cachePD->GetAbstractArray(pointDataTable->GetColumnName(iArr));
          if (arrayToCopyIn)
          {
            arrayToCopyIn->InsertTuples(this->GhostPointsToReceive[i].Get(),
              dumStaticPointIds.Get(), pointDataTable->GetColumn(iArr));
          }
        }
      }
    }
    // Make sure all rank finished
    controller->Barrier();

    for (int i = 0; i < nProc; i++)
    {
      if (i != rank && this->GhostCellsToSend[i]->GetNumberOfIds() > 0)
      {
        // Prepare ghost cells data to send it as a table
        vtkNew<vtkCellData> ghostCellData;
        ghostCellData->CopyAllocate(inCD);

        // Prepare a list of iterating ids and copy all ghost point ids
        // for this rank into the ghostPointData
        vtkNew<vtkIdList> dumStaticCellIds;
        dumStaticCellIds->SetNumberOfIds(this->GhostCellsToSend[i]->GetNumberOfIds());
        for (vtkIdType id = 0; id < this->GhostCellsToSend[i]->GetNumberOfIds(); id++)
        {
          dumStaticCellIds->SetId(id, id);
        }
        ghostCellData->CopyData(inCD, this->GhostCellsToSend[i].Get(), dumStaticCellIds.Get());

        // Add each point data array to a dumStatic table
        vtkNew<vtkTable> cellDataTable;
        for (int iArr = 0; iArr < ghostCellData->GetNumberOfArrays(); iArr++)
        {
          cellDataTable->AddColumn(ghostCellData->GetArray(iArr));
        }

        // Marshall the table and transfer it to rank
        vtkCommunicator::MarshalDataObject(cellDataTable.Get(), buffers[i].Get());
        lengths[i] = buffers[i]->GetNumberOfTuples();
        controller->NoBlockSend(&lengths[i], 1, i, SUGGCG_SIZE_EXCHANGE_TAG, sizeReqs[i]);
        controller->NoBlockSend((char*)(buffers[i]->GetVoidPointer(0)), lengths[i], i,
          SUGGCG_DATA_EXCHANGE_TAG, dataReqs[i]);
      }
    }
    for (int i = 0; i < nProc; i++)
    {
      if (i != rank && this->GhostCellsToReceive[i]->GetNumberOfIds() > 0)
      {
        // Receive dumStatic table to unmarshall
        vtkIdType length;
        controller->Receive(&length, 1, i, SUGGCG_SIZE_EXCHANGE_TAG);

        vtkNew<vtkCharArray> recvBuffer;
        recvBuffer->SetNumberOfValues(length);
        controller->Receive(
          (char*)(recvBuffer->GetVoidPointer(0)), length, i, SUGGCG_DATA_EXCHANGE_TAG);
        vtkNew<vtkTable> cellDataTable;
        vtkCommunicator::UnMarshalDataObject(recvBuffer.Get(), cellDataTable.Get());

        // Create a dumStatic iterating point ids
        vtkNew<vtkIdList> dumStaticCellIds;
        dumStaticCellIds->SetNumberOfIds(this->GhostCellsToReceive[i]->GetNumberOfIds());
        for (vtkIdType id = 0; id < this->GhostCellsToReceive[i]->GetNumberOfIds(); id++)
        {
          dumStaticCellIds->SetId(id, id);
        }

        // Copy the tuples of each array from the dumStatic table
        // into the ghost point data
        for (int iArr = 0; iArr < cellDataTable->GetNumberOfColumns(); iArr++)
        {
          vtkAbstractArray* arrayToCopyIn =
            cacheCD->GetAbstractArray(cellDataTable->GetColumnName(iArr));
          if (arrayToCopyIn)
          {
            arrayToCopyIn->InsertTuples(this->GhostCellsToReceive[i].Get(), dumStaticCellIds.Get(),
              cellDataTable->GetColumn(iArr));
          }
        }
      }
    }
    // Make sure all rank finished
    controller->Barrier();
  }
}

//----------------------------------------------------------------------------
void vtkStaticPUnstructuredGridGhostCellsGenerator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Cache: " << this->Cache << endl;
  os << indent << "Input Mesh Time: " << this->InputMeshTime << endl;
  os << indent << "Filter mTime: " << this->FilterMTime << endl;
}
