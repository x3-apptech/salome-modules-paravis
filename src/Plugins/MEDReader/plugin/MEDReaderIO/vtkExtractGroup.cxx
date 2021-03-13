// Copyright (C) 2010-2020  CEA/DEN, EDF R&D
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
// Author : Anthony Geay

#include "vtkExtractGroup.h"
#include "MEDFileFieldRepresentationTree.hxx"
#include "ExtractGroupHelper.h"
#include "vtkMEDReader.h"
#include "VTKMEDTraits.hxx"

#include "vtkAdjacentVertexIterator.h"
#include "vtkAOSDataArrayTemplate.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#ifdef WIN32
#include "vtkLongLongArray.h"
#endif
#include "vtkCellData.h"
#include "vtkPointData.h"

#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"
#include  "vtkMultiBlockDataSet.h"

#include "vtkInformationStringKey.h"
#include "vtkAlgorithmOutput.h"
#include "vtkObjectFactory.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkDataSet.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataArraySelection.h"
#include "vtkTimeStamp.h"
#include "vtkInEdgeIterator.h"
#include "vtkInformationDataObjectKey.h"
#include "vtkExecutive.h"
#include "vtkVariantArray.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkThreshold.h"
#include "vtkMultiBlockDataGroupFilter.h"
#include "vtkCompositeDataToUnstructuredGridFilter.h"
#include "vtkInformationDataObjectMetaDataKey.h"

#include <map>
#include <deque>

vtkStandardNewMacro(vtkExtractGroup)

class vtkExtractGroup::vtkExtractGroupInternal : public ExtractGroupInternal
{
};

////////////////////

vtkExtractGroup::vtkExtractGroup():SIL(NULL),Internal(new vtkExtractGroupInternal),InsideOut(0)
{
}

vtkExtractGroup::~vtkExtractGroup()
{
  delete this->Internal;
}

void vtkExtractGroup::SetInsideOut(int val)
{
  if(this->InsideOut!=val)
    {
      this->InsideOut=val;
      this->Modified();
    }
}

int vtkExtractGroup::RequestInformation(vtkInformation * /*request*/, vtkInformationVector **inputVector, vtkInformationVector */*outputVector*/)
{
//  vtkUnstructuredGridAlgorithm::RequestInformation(request,inputVector,outputVector);
  try
    {
//      std::cerr << "########################################## vtkExtractGroup::RequestInformation ##########################################" << std::endl;
//      request->Print(cout);
      //vtkInformation *outInfo(outputVector->GetInformationObject(0)); // todo: unused
      vtkInformation *inputInfo(inputVector[0]->GetInformationObject(0));
      if(!ExtractGroupInternal::IndependantIsInformationOK(vtkMEDReader::META_DATA(),inputInfo))
        {
        vtkErrorMacro("No SIL Data available ! The source of this filter must be MEDReader !");
        return 0;
        }

      this->SetSIL(vtkMutableDirectedGraph::SafeDownCast(inputInfo->Get(vtkMEDReader::META_DATA())));
      this->Internal->loadFrom(this->SIL);
      //this->Internal->printMySelf(std::cerr);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::cerr << "Exception has been thrown in vtkExtractGroup::RequestInformation : " << e.what() << std::endl;
      return 0;
    }
  return 1;
}

/*!
 * Do not use vtkCxxSetObjectMacro macro because input mdg comes from an already managed in the pipeline just a ref on it.
 */
void vtkExtractGroup::SetSIL(vtkMutableDirectedGraph *mdg)
{
  if(this->SIL==mdg)
    return ;
  this->SIL=mdg;
}

template<class CellPointExtractor>
vtkDataSet *FilterFamilies(vtkSmartPointer<vtkThreshold>& thres,
                           vtkDataSet *input, const std::set<int>& idsToKeep, bool insideOut, const char *arrNameOfFamilyField,
                           const char *associationForThreshold, bool& catchAll, bool& catchSmth)
{
  const int VTK_DATA_ARRAY_DELETE=vtkAOSDataArrayTemplate<double>::VTK_DATA_ARRAY_DELETE;
  const char ZE_SELECTION_ARR_NAME[]="@@ZeSelection@@";
  vtkDataSet *output(input->NewInstance());
  output->ShallowCopy(input);
  thres->SetInputData(output);
  //vtkDataSetAttributes *dscIn(input->GetCellData()),*dscIn2(input->GetPointData()); // todo: unused
  //vtkDataSetAttributes *dscOut(output->GetCellData()),*dscOut2(output->GetPointData()); // todo: unused
  //
  double vMin(insideOut==0?1.:0.),vMax(insideOut==0?2.:1.);
  thres->ThresholdBetween(vMin,vMax);
  // OK for the output
  //
  CellPointExtractor cpe2(input);
  vtkDataArray *da(cpe2.Get()->GetScalars(arrNameOfFamilyField));
  if(!da)
    return 0;
  std::string daName(da->GetName());
  typedef MEDFileVTKTraits<mcIdType>::VtkType vtkMCIdTypeArray;
  vtkMCIdTypeArray *dai(vtkMCIdTypeArray::SafeDownCast(da));
  if(daName!=arrNameOfFamilyField || !dai)
    return 0;
  //
  int nbOfTuples(dai->GetNumberOfTuples());
  vtkCharArray *zeSelection(vtkCharArray::New());
  zeSelection->SetName(ZE_SELECTION_ARR_NAME);
  zeSelection->SetNumberOfComponents(1);
  char *pt(new char[nbOfTuples]);
  zeSelection->SetArray(pt,nbOfTuples,0,VTK_DATA_ARRAY_DELETE);
  const mcIdType *inPtr(dai->GetPointer(0));
  std::fill(pt,pt+nbOfTuples,0);
  catchAll=true; catchSmth=false;
  std::vector<bool> pt2(nbOfTuples,false);
  for(std::set<int>::const_iterator it=idsToKeep.begin();it!=idsToKeep.end();it++)
    {
      bool catchFid(false);
      for(int i=0;i<nbOfTuples;i++)
        if(inPtr[i]==*it)
          { pt2[i]=true; catchFid=true; }
      if(!catchFid)
        catchAll=false;
      else
        catchSmth=true;
    }
  for(int ii=0;ii<nbOfTuples;ii++)
    if(pt2[ii])
      pt[ii]=2;
  CellPointExtractor cpe3(output);
  int idx(cpe3.Get()->AddArray(zeSelection));
  cpe3.Get()->SetActiveAttribute(idx,vtkDataSetAttributes::SCALARS);
  cpe3.Get()->CopyScalarsOff();
  zeSelection->Delete();
  //
  thres->SetInputArrayToProcess(idx,0,0,associationForThreshold,ZE_SELECTION_ARR_NAME);
  thres->Update();
  vtkUnstructuredGrid *zeComputedOutput(thres->GetOutput());
  CellPointExtractor cpe(zeComputedOutput);
  cpe.Get()->RemoveArray(idx);
  output->Delete();
  zeComputedOutput->Register(0);
  return zeComputedOutput;
}

class CellExtractor
{
public:
  CellExtractor(vtkDataSet *ds):_ds(ds) { }
  vtkDataSetAttributes *Get() { return _ds->GetCellData(); }
private:
  vtkDataSet *_ds;
};

class PointExtractor
{
public:
  PointExtractor(vtkDataSet *ds):_ds(ds) { }
  vtkDataSetAttributes *Get() { return _ds->GetPointData(); }
private:
  vtkDataSet *_ds;
};
int vtkExtractGroup::RequestData(vtkInformation * /*request*/, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  try
    {
      // std::cerr << "########################################## vtkExtractGroup::RequestData        ##########################################" << std::endl;
      // request->Print(cout);
      vtkInformation* inputInfo=inputVector[0]->GetInformationObject(0);
      vtkMultiBlockDataSet *inputMB(vtkMultiBlockDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
      if(inputMB->GetNumberOfBlocks()!=1)
        {
          std::ostringstream oss; oss << "vtkExtractGroup::RequestData : input has not the right number of parts ! Expected 1 !";
          if(this->HasObserver("ErrorEvent") )
            this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
          else
            vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
          vtkObject::BreakOnError();
          return 0;
        }
      vtkDataSet *input(vtkDataSet::SafeDownCast(inputMB->GetBlock(0)));
      //vtkInformation *info(input->GetInformation()); // todo: unused
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkMultiBlockDataSet *output(vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      std::set<int> idsToKeep(this->Internal->getIdsToKeep());
      this->Internal->clearSelection();
      // first shrink the input
      bool catchAll,catchSmth;
      vtkSmartPointer<vtkThreshold> thres1(vtkSmartPointer<vtkThreshold>::New()),thres2(vtkSmartPointer<vtkThreshold>::New());
      vtkDataSet *tryOnCell(FilterFamilies<CellExtractor>(thres1,input,idsToKeep,this->InsideOut,
                                                          MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME,"vtkDataObject::FIELD_ASSOCIATION_CELLS",catchAll,catchSmth));
      if(tryOnCell)
        {
          if(catchAll)
            {
              output->SetBlock(0,tryOnCell);
              tryOnCell->Delete();//
              return 1;
            }
          else
            {
              if(catchSmth)
                {
                  vtkDataSet *tryOnNode(FilterFamilies<PointExtractor>(thres2,input,idsToKeep,this->InsideOut,
                                                                       MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_NODE_NAME,"vtkDataObject::FIELD_ASSOCIATION_POINTS",catchAll,catchSmth));
                  if(tryOnNode && catchSmth)
                    {
                      output->SetBlock(0,tryOnCell);
                      output->SetBlock(1,tryOnNode);
                      tryOnCell->Delete();
                      tryOnNode->Delete();
                      return 1;
                    }
                  else
                    {
                      if(tryOnNode)
                        tryOnNode->Delete();
                      output->SetBlock(0,tryOnCell);
                      tryOnCell->Delete();
                      return 1;
                    }
                }
              else
                {
                  vtkDataSet *tryOnNode(FilterFamilies<PointExtractor>(thres1,input,idsToKeep,this->InsideOut,
                                                                       MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_NODE_NAME,"vtkDataObject::FIELD_ASSOCIATION_POINTS",catchAll,catchSmth));
                  if(tryOnNode)
                    {
                      tryOnCell->Delete();
                      output->SetBlock(0,tryOnNode);
                      tryOnNode->Delete();
                      return 1;
                    }
                  else
                    {
                      output->SetBlock(0,tryOnNode);
                      tryOnCell->Delete();
                      return 0;
                    }
                }
            }
        }
      else
        {
          vtkDataSet *tryOnNode(FilterFamilies<PointExtractor>(thres1,input,idsToKeep,this->InsideOut,
                                                               MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_NODE_NAME,"vtkDataObject::FIELD_ASSOCIATION_POINTS",catchAll,catchSmth));
          if(tryOnNode)
            {
              output->ShallowCopy(tryOnNode);
              tryOnNode->Delete();//
              return 1;
            }
          else
            {
              std::ostringstream oss; oss << "vtkExtractGroup::RequestData : The integer array with name \""<< MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME;
              oss << "\" or \"" << MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_NODE_NAME << "\" does not exist ! The extraction of group and/or family is not possible !";
              if(this->HasObserver("ErrorEvent") )
                this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
              else
                vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
              vtkObject::BreakOnError();
              return 0;
            }
        }
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::cerr << "Exception has been thrown in vtkExtractGroup::RequestData : " << e.what() << std::endl;
      return 0;
    }
}

int vtkExtractGroup::GetSILUpdateStamp()
{
  return (int)this->SILTime;
}

const char* vtkExtractGroup::GetGrpStart()
{
  return ExtractGroupGrp::START;
}

const char* vtkExtractGroup::GetFamStart()
{
  return ExtractGroupFam::START;
}

void vtkExtractGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkExtractGroup::GetNumberOfGroupsFlagsArrays()
{
  int ret(this->Internal->getNumberOfEntries());
  //std::cerr << "vtkExtractGroup::GetNumberOfFieldsTreeArrays() -> " << ret << std::endl;
  return ret;
}

const char *vtkExtractGroup::GetGroupsFlagsArrayName(int index)
{
  const char *ret(this->Internal->getKeyOfEntry(index));
//  std::cerr << "vtkExtractGroup::GetFieldsTreeArrayName(" << index << ") -> " << ret << std::endl;
  return ret;
}

int vtkExtractGroup::GetGroupsFlagsArrayStatus(const char *name)
{
  int ret((int)this->Internal->getStatusOfEntryStr(name));
//  std::cerr << "vtkExtractGroup::GetGroupsFlagsArrayStatus(" << name << ") -> " << ret << std::endl;
  return ret;
}

void vtkExtractGroup::SetGroupsFlagsStatus(const char *name, int status)
{
  //std::cerr << "vtkExtractGroup::SetFieldsStatus(" << name << "," << status << ")" << std::endl;
  this->Internal->setStatusOfEntryStr(name,(bool)status);
  this->Modified();
  //this->Internal->printMySelf(std::cerr);
}

const char *vtkExtractGroup::GetMeshName()
{
  return this->Internal->getMeshName();
}
