// Copyright (C) 2020  CEA/DEN, EDF R&D
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
// Author : Anthony Geay (EDF R&D)

#include "vtkGroupAsMultiBlock.h"
#include "ExtractGroupHelper.h"
#include "vtkMEDReader.h"
#include "vtkUgSelectCellIds.h"
#include "MEDFileFieldRepresentationTree.hxx"
#include "VTKMEDTraits.hxx"

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include "vtkMutableDirectedGraph.h"
#include "vtkInformationDataObjectMetaDataKey.h"
#include <vtkCompositeDataToUnstructuredGridFilter.h>
#include <vtkMultiBlockDataGroupFilter.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkPVGlyphFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkCellCenters.h>
#include <vtkGlyphSource2D.h>

class vtkGroupAsMultiBlockInternal : public ExtractGroupInternal
{
};

vtkStandardNewMacro(vtkGroupAsMultiBlock);

vtkGroupAsMultiBlock::vtkGroupAsMultiBlock():Internal(new ExtractGroupInternal)
{
}

vtkGroupAsMultiBlock::~vtkGroupAsMultiBlock()
{
  delete this->Internal;
}

int vtkGroupAsMultiBlock::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  vtkInformation *outInfo(outputVector->GetInformationObject(0));
  vtkInformation *inputInfo(inputVector[0]->GetInformationObject(0));
  if(!ExtractGroupInternal::IndependantIsInformationOK(vtkMEDReader::META_DATA(),inputInfo))
  {
    vtkErrorMacro("No SIL Data available ! The source of this filter must be MEDReader !");
    return 0;
  }
  vtkMutableDirectedGraph *mdg(vtkMutableDirectedGraph::SafeDownCast(inputInfo->Get(vtkMEDReader::META_DATA())));
  this->Internal->loadFrom(mdg);
  return 1;
}

int vtkGroupAsMultiBlock::RequestData(vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  vtkInformation* inputInfo=inputVector[0]->GetInformationObject(0);
  vtkMultiBlockDataSet *inputMB(vtkMultiBlockDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(inputMB->GetNumberOfBlocks()!=1)
  {
    vtkErrorMacro("vtkGroupAsMultiBlock::RequestData : input has not the right number of parts ! Expected 1 !");
    return 0;
  }
  vtkDataSet *input(vtkDataSet::SafeDownCast(inputMB->GetBlock(0)));
  vtkInformation *info(input->GetInformation());
  vtkInformation *outInfo(outputVector->GetInformationObject(0));
  vtkMultiBlockDataSet *output(vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(!output)
  {
    vtkErrorMacro("vtkGroupAsMultiBlock::RequestData : something wrong !");
    return 0; 
  }
  //
  vtkUnstructuredGrid *inputc(vtkUnstructuredGrid::SafeDownCast(input));
  if(!inputc)
  {
    vtkErrorMacro("vtkGroupAsMultiBlock::RequestData : Only implemented for vtkUnstructuredGrid !");
    return 0; 
  }
  //
  vtkDataArray *famIdsGen(inputc->GetCellData()->GetScalars(MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME));
  if(!famIdsGen)
  {
    vtkErrorMacro(<< "vtkGroupAsMultiBlock::RequestData : Fail to locate " << MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME << " array !");
    return 0;
  }
  using vtkMCIdTypeArray = MEDFileVTKTraits<mcIdType>::VtkType;
  vtkMCIdTypeArray *famIdsArr(vtkMCIdTypeArray::SafeDownCast(famIdsGen));
  if(!famIdsArr)
  {
    vtkErrorMacro(<< "vtkGroupAsMultiBlock::RequestData : cell array " << MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME << " is exepected to be IdType !");
    return 0; 
  }
  // Let's go !
  vtkIdType inputNbCell(famIdsArr->GetNumberOfTuples());
  std::vector< std::pair<std::string,std::vector<int> > > allGroups(this->Internal->getAllGroups());
  output->SetNumberOfBlocks(allGroups.size());
  int blockId(0);
  for(const auto& grp : allGroups)
  {
    std::vector<vtkIdType> ids;
    // iterate on all families on current group grp
    for(const auto& famId : grp.second)
    {// for each family of group grp locate cells in input lying on that group
      vtkIdType pos(0);
      std::for_each(famIdsArr->GetPointer(0),famIdsArr->GetPointer(inputNbCell),[&pos,&ids,famId](vtkIdType curFamId) { if(curFamId == famId) { ids.push_back(pos); } pos++; });
    }
    std::sort(ids.begin(),ids.end());
    vtkNew<vtkIdTypeArray> idsVTK;
    idsVTK->SetNumberOfComponents(1); idsVTK->SetNumberOfTuples(ids.size());
    std::copy(ids.begin(),ids.end(),idsVTK->GetPointer(0));
    vtkNew<vtkUgSelectCellIds> cellSelector;
    cellSelector->SetIds(idsVTK);
    cellSelector->SetInputData(inputc);
    cellSelector->Update();
    output->SetBlock(blockId++,cellSelector->GetOutput());
  }
  return 1;
}
