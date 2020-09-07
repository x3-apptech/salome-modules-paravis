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

#include "vtkGroupsNames.h"
#include "ExtractGroupHelper.h"
#include "vtkMEDReader.h"
#include "vtkUgSelectCellIds.h"
#include "MEDFileFieldRepresentationTree.hxx"
#include "vtkLongArray.h"
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
#include "vtkTable.h"
#include "vtkStringArray.h"

class vtkGroupsNamesInternal : public ExtractGroupInternal
{
};

vtkStandardNewMacro(vtkGroupsNames);

vtkGroupsNames::vtkGroupsNames():Internal(new ExtractGroupInternal)
{
}

vtkGroupsNames::~vtkGroupsNames()
{
  delete this->Internal;
}

int vtkGroupsNames::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
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

int vtkGroupsNames::RequestData(vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  vtkInformation* inputInfo=inputVector[0]->GetInformationObject(0);
  vtkDataSet *input(nullptr);
  vtkMultiBlockDataSet *inputMB(vtkMultiBlockDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(inputMB)
  {
    if(inputMB->GetNumberOfBlocks()!=1)
    {
      vtkErrorMacro("vtkGroupsNames::RequestData : input has not the right number of parts ! Expected 1 !");
      return 0;
    }
    input = vtkDataSet::SafeDownCast(inputMB->GetBlock(0));
  }
  else
  {
    input = vtkDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT()));
  }
  if(!input)
  {
    vtkErrorMacro("vtkGroupsNames::RequestData : input is neither a DataSet nor a MultiblockDataSet !");
    return 0;
  }
  vtkInformation *info(input->GetInformation());
  vtkInformation *outInfo(outputVector->GetInformationObject(0));
  vtkTable *output(vtkTable::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(!output)
  {
    vtkErrorMacro("vtkGroupsNames::RequestData : something wrong !");
    return 0; 
  }
  //
  vtkUnstructuredGrid *inputc(vtkUnstructuredGrid::SafeDownCast(input));
  if(!inputc)
  {
    vtkErrorMacro("vtkGroupsNames::RequestData : Only implemented for vtkUnstructuredGrid !");
    return 0; 
  }
  //
  vtkDataArray *famIdsGen(inputc->GetCellData()->GetScalars(MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME));
  if(!famIdsGen)
  {
    vtkErrorMacro(<< "vtkGroupsNames::RequestData : Fail to locate " << MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME << " array !");
    return 0;
  }
  using vtkMCIdTypeArray = MEDFileVTKTraits<mcIdType>::VtkType;
  vtkMCIdTypeArray *famIdsArr(vtkMCIdTypeArray::SafeDownCast(famIdsGen));
  if(!famIdsArr)
  {
    vtkErrorMacro(<< "vtkGroupsNames::RequestData : cell array " << MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME << " is exepected to be IdType !");
    return 0; 
  }
  // Let's go !
  std::vector< std::pair<std::string,std::vector<int> > > allGroups(this->Internal->getAllGroups());
  vtkNew<vtkIntArray> blockIdArr;
  blockIdArr->SetName("Block ID");
  blockIdArr->SetNumberOfTuples(allGroups.size());
  vtkNew<vtkStringArray> groupNameArr;
  groupNameArr->SetName("Group Name");
  groupNameArr->SetNumberOfTuples(allGroups.size());
  int blockId(0);
  for(const auto& grp : allGroups)
  {
    blockIdArr->SetValue(blockId,blockId);
    groupNameArr->SetValue(blockId,vtkStdString(grp.first));
    blockId++;
  }
  output->AddColumn(blockIdArr);
  output->AddColumn(groupNameArr);
  return 1;
}
