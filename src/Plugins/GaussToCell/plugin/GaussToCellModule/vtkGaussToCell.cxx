// Copyright (C) 2018-2020  CEA/DEN, EDF R&D
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

#include "vtkGaussToCell.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkIntArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCellType.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"

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
#include "vtkInformationDataObjectMetaDataKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkExecutive.h"
#include "vtkVariantArray.h"
#include "vtkStringArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkCharArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkWarpScalar.h"
#include "vtkQuadratureSchemeDefinition.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkCompositeDataToUnstructuredGridFilter.h"
#include "vtkMultiBlockDataGroupFilter.h"

#include "MEDCouplingMemArray.hxx"
#include "MEDCouplingUMesh.hxx"
#include "MEDCouplingFieldDouble.hxx"
#include "InterpKernelAutoPtr.hxx"
#include "InterpKernelGaussCoords.hxx"

#include <map>
#include <set>
#include <deque>
#include <sstream>

using MEDCoupling::DataArray;
using MEDCoupling::DataArrayInt32;
using MEDCoupling::DataArrayInt64;
using MEDCoupling::DataArrayDouble;
using MEDCoupling::MEDCouplingMesh;
using MEDCoupling::MEDCouplingUMesh;
using MEDCoupling::DynamicCastSafe;
using MEDCoupling::MEDCouplingFieldDouble;
using MEDCoupling::ON_GAUSS_PT;
using MEDCoupling::MCAuto;

vtkStandardNewMacro(vtkGaussToCell);

vtkInformationDoubleVectorKey *GetMEDReaderMetaDataIfAny()
{
  static const char ZE_KEY[]="vtkMEDReader::GAUSS_DATA";
  MEDCoupling::GlobalDict *gd(MEDCoupling::GlobalDict::GetInstance());
  if(!gd->hasKey(ZE_KEY))
    return 0;
  std::string ptSt(gd->value(ZE_KEY));
  void *pt(0);
  std::istringstream iss(ptSt); iss >> pt;
  return reinterpret_cast<vtkInformationDoubleVectorKey *>(pt);
}

bool IsInformationOK(vtkInformation *info, std::vector<double>& data)
{
  vtkInformationDoubleVectorKey *key(GetMEDReaderMetaDataIfAny());
  if(!key)
    return false;
  // Check the information contain meta data key
  if(!info->Has(key))
    return false;
  int lgth(key->Length(info));
  const double *data2(info->Get(key));
  data.insert(data.end(),data2,data2+lgth);
  return true;
}

void ExtractInfo(vtkInformationVector *inputVector, vtkUnstructuredGrid *& usgIn)
{
  vtkInformation *inputInfo(inputVector->GetInformationObject(0));
  vtkDataSet *input(0);
  vtkDataSet *input0(vtkDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  vtkMultiBlockDataSet *input1(vtkMultiBlockDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(input0)
    input=input0;
  else
    {
      if(!input1)
        throw INTERP_KERNEL::Exception("Input dataSet must be a DataSet or single elt multi block dataset expected !");
      if(input1->GetNumberOfBlocks()!=1)
        throw INTERP_KERNEL::Exception("Input dataSet is a multiblock dataset with not exactly one block ! Use MergeBlocks or ExtractBlocks filter before calling this filter !");
      vtkDataObject *input2(input1->GetBlock(0));
      if(!input2)
        throw INTERP_KERNEL::Exception("Input dataSet is a multiblock dataset with exactly one block but this single element is NULL !");
      vtkDataSet *input2c(vtkDataSet::SafeDownCast(input2));
      if(!input2c)
        throw INTERP_KERNEL::Exception("Input dataSet is a multiblock dataset with exactly one block but this single element is not a dataset ! Use MergeBlocks or ExtractBlocks filter before calling this filter !");
      input=input2c;
    }
  if(!input)
    throw INTERP_KERNEL::Exception("Input data set is NULL !");
  usgIn=vtkUnstructuredGrid::SafeDownCast(input);
  if(!usgIn)
    throw INTERP_KERNEL::Exception("Input data set is not an unstructured mesh ! This filter works only on unstructured meshes !");
}

vtkGaussToCell::vtkGaussToCell():avgStatus(true),maxStatus(false),minStatus(false)
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkGaussToCell::~vtkGaussToCell()
{
}

void vtkGaussToCell::SetAvgFlag(bool avgStatus)
{
  if(this->avgStatus!=avgStatus)
    {
      this->avgStatus=avgStatus;
      this->Modified();
    }
}

void vtkGaussToCell::SetMaxFlag(bool maxStatus)
{
  if(this->maxStatus!=maxStatus)
    {
      this->maxStatus=maxStatus;
      this->Modified();
    }
}

void vtkGaussToCell::SetMinFlag(bool minStatus)
{
  if(this->minStatus!=minStatus)
    {
      this->minStatus=minStatus;
      this->Modified();
    }
}

int vtkGaussToCell::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{ 
  //std::cerr << "########################################## vtkGaussToCell::RequestInformation ##########################################" << std::endl;
  try
    {
      vtkUnstructuredGrid *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkGaussToCell::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

typedef void (*DataComputer) (const double *inData, const vtkIdType *offData, const std::vector<int> *nbgPerCell, int zeNbCompo, vtkIdType outNbCells, double *outData);

void ComputeAvg(const double *inData, const vtkIdType *offData, const std::vector<int> *nbgPerCell, int zeNbCompo, vtkIdType outNbCells, double *outData)
{
  std::fill(outData,outData+outNbCells*zeNbCompo,0.);
  for(auto i=0;i<outNbCells;i++,outData+=zeNbCompo)
    {
      auto NbGaussPt((*nbgPerCell)[i]);
      for(auto j=0;j<NbGaussPt;j++)
        std::transform(inData+(offData[i]+j)*zeNbCompo,inData+(offData[i]+(j+1))*zeNbCompo,outData,outData,[](const double& v, const double& w) { return v+w; });
      double deno(1./(double)NbGaussPt);
      std::transform(outData,outData+zeNbCompo,outData,[deno](const double& v) { return v*deno; });
    }
}

void ComputeMax(const double *inData, const vtkIdType *offData, const std::vector<int> *nbgPerCell, int zeNbCompo, vtkIdType outNbCells, double *outData)
{
  std::fill(outData,outData+outNbCells*zeNbCompo,-std::numeric_limits<double>::max());
  for(auto i=0;i<outNbCells;i++,outData+=zeNbCompo)
    {
      auto NbGaussPt((*nbgPerCell)[i]);
      for(auto j=0;j<NbGaussPt;j++)
        std::transform(inData+(offData[i]+j)*zeNbCompo,inData+(offData[i]+(j+1))*zeNbCompo,outData,outData,[](const double& v, const double& w) { return std::max(v,w); });
    }
}

void ComputeMin(const double *inData, const vtkIdType *offData, const std::vector<int> *nbgPerCell, int zeNbCompo, vtkIdType outNbCells, double *outData)
{
  std::fill(outData,outData+outNbCells*zeNbCompo,std::numeric_limits<double>::max());
  for(auto i=0;i<outNbCells;i++,outData+=zeNbCompo)
    {
      auto NbGaussPt((*nbgPerCell)[i]);
      for(auto j=0;j<NbGaussPt;j++)
        std::transform(inData+(offData[i]+j)*zeNbCompo,inData+(offData[i]+(j+1))*zeNbCompo,outData,outData,[](const double& v, const double& w) { return std::min(v,w); });
    }
}

void DealWith(const char *postName, vtkDoubleArray *zearray, vtkIdTypeArray *offsets, const std::vector<int> *nbgPerCell, vtkIdType outNbCells, vtkDoubleArray *zeOutArray, DataComputer dc)
{
  std::ostringstream oss;
  oss << zearray->GetName() << '_' << postName;
  int zeNbCompo(zearray->GetNumberOfComponents());
  {
    std::string st(oss.str());
    zeOutArray->SetName(st.c_str());
  }
  zeOutArray->SetNumberOfComponents(zeNbCompo);
  zeOutArray->SetNumberOfTuples(outNbCells);
  double *outData(zeOutArray->GetPointer(0));
  const double *inData(zearray->GetPointer(0));
  const auto *offData(offsets->GetPointer(0));
  for(auto i=0;i<zeNbCompo;i++)
    {
      const char *comp(zearray->GetComponentName(i));
      if(comp)
        zeOutArray->SetComponentName(i,comp);
    }
  dc(inData,offData,nbgPerCell,zeNbCompo,outNbCells,outData);
}

int vtkGaussToCell::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkGaussToCell::RequestData        ##########################################" << std::endl;
  try
    {
      std::vector<double> GaussAdvData;
      bool isOK(IsInformationOK(inputVector[0]->GetInformationObject(0),GaussAdvData));
      if(!isOK)
        throw INTERP_KERNEL::Exception("Sorry but no advanced gauss info found ! Expect to be called right after a MEDReader containing Gauss Points !");
      vtkUnstructuredGrid *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkUnstructuredGrid *output(vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      output->ShallowCopy(usgIn);
      //
      std::string zeArrOffset;
      int nArrays(usgIn->GetFieldData()->GetNumberOfArrays());
      std::map<vtkIdTypeArray *,std::vector<int> > offsetKeyMap;//Map storing for each offsets array the corresponding nb of Gauss Points per cell
      for(int i=0;i<nArrays;i++)
        {
          vtkDataArray *array(usgIn->GetFieldData()->GetArray(i));
          if(!array)
            continue;
          const char* arrayOffsetName(array->GetInformation()->Get(vtkQuadratureSchemeDefinition::QUADRATURE_OFFSET_ARRAY_NAME()));
          if(!arrayOffsetName)
            continue;
          std::string arrOffsetNameCpp(arrayOffsetName);
          if(arrOffsetNameCpp.find("ELGA@")==std::string::npos)
            continue;
          if(zeArrOffset.empty())
            zeArrOffset=arrOffsetNameCpp;
          else
            if(zeArrOffset!=arrOffsetNameCpp)
              {
                throw INTERP_KERNEL::Exception("ComputeGaussToCell : error in QUADRATURE_OFFSET_ARRAY_NAME for Gauss fields array !");
              }
          vtkDataArray *offTmp(usgIn->GetCellData()->GetArray(zeArrOffset.c_str()));
          if(!offTmp)
            {
              std::ostringstream oss; oss << "ComputeGaussToCell : cell field " << zeArrOffset << " not found !";
              throw INTERP_KERNEL::Exception(oss.str());
            }
          vtkIdTypeArray *offsets(vtkIdTypeArray::SafeDownCast(offTmp));
          if(!offsets)
            {
              std::ostringstream oss; oss << "ComputeGaussToCell : cell field " << zeArrOffset << " exists but not with the right type of data !";
              throw INTERP_KERNEL::Exception(oss.str());
            }
          vtkDoubleArray *zearray(vtkDoubleArray::SafeDownCast(array));
          if(!zearray)
            continue ;
          //
          std::map<vtkIdTypeArray *,std::vector<int> >::iterator nbgPerCellPt(offsetKeyMap.find(offsets));
          const std::vector<int> *nbgPerCell(nullptr);
          if(nbgPerCellPt==offsetKeyMap.end())
            {
              // fini la parlote
              vtkInformation *info(offsets->GetInformation());
              if(!info)
                throw INTERP_KERNEL::Exception("info is null ! Internal error ! Looks bad !");
              vtkInformationQuadratureSchemeDefinitionVectorKey *key(vtkQuadratureSchemeDefinition::DICTIONARY());
              if(!key->Has(info))
                throw INTERP_KERNEL::Exception("No quadrature key in info included in offets array ! Internal error ! Looks bad !");
              int dictSize(key->Size(info));
              INTERP_KERNEL::AutoPtr<vtkQuadratureSchemeDefinition *> dict(new vtkQuadratureSchemeDefinition *[dictSize]);
              key->GetRange(info,dict,0,0,dictSize);
              auto nbOfCells(output->GetNumberOfCells());
              std::vector<int> nbg(nbOfCells);
              for(auto cellId=0;cellId<nbOfCells;cellId++)
                {
                  int ct(output->GetCellType(cellId));
                  vtkQuadratureSchemeDefinition *gaussLoc(dict[ct]);
                  if(!gaussLoc)
                    {
                      std::ostringstream oss; oss << "For cell " << cellId << " no Gauss info attached !";
                      throw INTERP_KERNEL::Exception(oss.str());
                    }
                  int np(gaussLoc->GetNumberOfQuadraturePoints());
                  nbg[cellId]=np;
                }
              nbgPerCell=&((*(offsetKeyMap.emplace(offsets,std::move(nbg)).first)).second);
            }
          else
            {
              nbgPerCell=&((*nbgPerCellPt).second);
            }
          auto outNbCells(nbgPerCell->size());
          if(this->avgStatus)
            {
              vtkNew<vtkDoubleArray> zeOutArray;
              DealWith("avg",zearray,offsets,nbgPerCell,outNbCells,zeOutArray,ComputeAvg);
              output->GetCellData()->AddArray(zeOutArray);
            }
          if(this->maxStatus)
            {
              vtkNew<vtkDoubleArray> zeOutArray;
              DealWith("max",zearray,offsets,nbgPerCell,outNbCells,zeOutArray,ComputeMax);
              output->GetCellData()->AddArray(zeOutArray);
            }
          if(this->minStatus)
            {
              vtkNew<vtkDoubleArray> zeOutArray;
              DealWith("min",zearray,offsets,nbgPerCell,outNbCells,zeOutArray,ComputeMin);
              output->GetCellData()->AddArray(zeOutArray);
            }
        }
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkGaussToCell::RequestData : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

void vtkGaussToCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
