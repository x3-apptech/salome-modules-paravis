// Copyright (C) 2016-2019  CEA/DEN, EDF R&D
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

#include "vtkMEDWriter.h"
#include "VTKToMEDMem.hxx"

#include "vtkAdjacentVertexIterator.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkCellArray.h"

#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationDataObjectMetaDataKey.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkRectilinearGrid.h"
#include "vtkInformationStringKey.h"
#include "vtkAlgorithmOutput.h"
#include "vtkObjectFactory.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
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
#include "vtkWarpScalar.h"

#include "MEDFileMesh.hxx"
#include "MEDFileField.hxx"
#include "MEDFileData.hxx"
#include "MEDCouplingMemArray.hxx"
#include "MEDCouplingFieldInt.hxx"
#include "MEDCouplingFieldFloat.hxx"
#include "MEDCouplingFieldDouble.hxx"
#include "MEDCouplingRefCountObject.hxx"
#include "MEDLoaderTraits.hxx"

#include <map>
#include <deque>
#include <sstream>

vtkStandardNewMacro(vtkMEDWriter);

using MEDCoupling::MEDFileData;
using MEDCoupling::MCAuto;
using VTKToMEDMem::Grp;
using VTKToMEDMem::Fam;

vtkInformationDataObjectMetaDataKey *GetMEDReaderMetaDataIfAny()
{
  static const char ZE_KEY[]="vtkMEDReader::META_DATA";
  MEDCoupling::GlobalDict *gd(MEDCoupling::GlobalDict::GetInstance());
  if(!gd->hasKey(ZE_KEY))
    return 0;
  std::string ptSt(gd->value(ZE_KEY));
  void *pt(0);
  std::istringstream iss(ptSt); iss >> pt;
  return reinterpret_cast<vtkInformationDataObjectMetaDataKey *>(pt);
}

bool IsInformationOK(vtkInformation *info)
{
  vtkInformationDataObjectMetaDataKey *key(GetMEDReaderMetaDataIfAny());
  if(!key)
    return false;
  // Check the information contain meta data key
  if(!info->Has(key))
    return false;
  // Recover Meta Data
  vtkMutableDirectedGraph *sil(vtkMutableDirectedGraph::SafeDownCast(info->Get(key)));
  if(!sil)
    return false;
  int idNames(0);
  vtkAbstractArray *verticesNames(sil->GetVertexData()->GetAbstractArray("Names",idNames));
  vtkStringArray *verticesNames2(vtkStringArray::SafeDownCast(verticesNames));
  if(!verticesNames2)
    return false;
  for(int i=0;i<verticesNames2->GetNumberOfValues();i++)
    {
      vtkStdString &st(verticesNames2->GetValue(i));
      if(st=="MeshesFamsGrps")
        return true;
    }
  return false;
}

void LoadFamGrpMapInfo(vtkMutableDirectedGraph *sil, std::string& meshName, std::vector<Grp>& groups, std::vector<Fam>& fams)
{
  if(!sil)
    throw MZCException("LoadFamGrpMapInfo : internal error !");
  int idNames(0);
  vtkAbstractArray *verticesNames(sil->GetVertexData()->GetAbstractArray("Names",idNames));
  vtkStringArray *verticesNames2(vtkStringArray::SafeDownCast(verticesNames));
  vtkIdType id0;
  bool found(false);
  for(int i=0;i<verticesNames2->GetNumberOfValues();i++)
    {
      vtkStdString &st(verticesNames2->GetValue(i));
      if(st=="MeshesFamsGrps")
        {
          id0=i;
          found=true;
        }
    }
  if(!found)
    throw INTERP_KERNEL::Exception("There is an internal error ! The tree on server side has not the expected look !");
  vtkAdjacentVertexIterator *it0(vtkAdjacentVertexIterator::New());
  sil->GetAdjacentVertices(id0,it0);
  int kk(0),ll(0);
  while(it0->HasNext())
    {
      vtkIdType id1(it0->Next());
      std::string mName((const char *)verticesNames2->GetValue(id1));
      meshName=mName;
      vtkAdjacentVertexIterator *it1(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(id1,it1);
      vtkIdType idZeGrps(it1->Next());//zeGroups
      vtkAdjacentVertexIterator *itGrps(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(idZeGrps,itGrps);
      while(itGrps->HasNext())
        {
          vtkIdType idg(itGrps->Next());
          Grp grp((const char *)verticesNames2->GetValue(idg));
          vtkAdjacentVertexIterator *itGrps2(vtkAdjacentVertexIterator::New());
          sil->GetAdjacentVertices(idg,itGrps2);
          std::vector<std::string> famsOnGroup;
          while(itGrps2->HasNext())
            {
              vtkIdType idgf(itGrps2->Next());
              famsOnGroup.push_back(std::string((const char *)verticesNames2->GetValue(idgf)));
            }
          grp.setFamilies(famsOnGroup);
          itGrps2->Delete();
          groups.push_back(grp);
        }
      itGrps->Delete();
      vtkIdType idZeFams(it1->Next());//zeFams
      it1->Delete();
      vtkAdjacentVertexIterator *itFams(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(idZeFams,itFams);
      while(itFams->HasNext())
        {
          vtkIdType idf(itFams->Next());
          Fam fam((const char *)verticesNames2->GetValue(idf));
          fams.push_back(fam);
        }
      itFams->Delete();
    }
  it0->Delete();
}

vtkMEDWriter::vtkMEDWriter():WriteAllTimeSteps(0),NumberOfTimeSteps(0),CurrentTimeIndex(0),FileName(0)
{
}

vtkMEDWriter::~vtkMEDWriter()
{
}

int vtkMEDWriter::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{ 
  //std::cerr << "########################################## vtkMEDWriter::RequestInformation ########################################## " << (const char *) this->FileName << std::endl;
  vtkInformation *inInfo(inputVector[0]->GetInformationObject(0));
  if(inInfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()))
    this->NumberOfTimeSteps=inInfo->Length(vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  else
    this->NumberOfTimeSteps=0;
  return 1;
}

int vtkMEDWriter::RequestUpdateExtent(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* vtkNotUsed(outputVector))
{
  double *inTimes(inputVector[0]->GetInformationObject(0)->Get(vtkStreamingDemandDrivenPipeline::TIME_STEPS()));
  if(inTimes && this->WriteAllTimeSteps)
    {
      double timeReq(inTimes[this->CurrentTimeIndex]);
      inputVector[0]->GetInformationObject(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(),timeReq);
    }
  return 1;
}

void ExceptionDisplayer(vtkMEDWriter *self, const std::string& fileName, std::exception& e)
{
  std::ostringstream oss;
  oss << "Exception has been thrown in vtkMEDWriter::RequestData : During writing of \"" << fileName << "\", the following exception has been thrown : "<< e.what() << std::endl;
  if(self->HasObserver("ErrorEvent") )
    self->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
  else
    vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
  vtkObject::BreakOnError();
}

int vtkMEDWriter::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkMEDWriter::RequestData        ########################################## " << (const char *) this->FileName << std::endl;
  try
    {
      bool writeAll((this->WriteAllTimeSteps!=0 && this->NumberOfTimeSteps>0));
      if(writeAll)
        {
          if(this->CurrentTimeIndex==0)
            request->Set(vtkStreamingDemandDrivenPipeline::CONTINUE_EXECUTING(),1);
        }
      else
        {
          request->Remove(vtkStreamingDemandDrivenPipeline::CONTINUE_EXECUTING());
          this->CurrentTimeIndex=0;
        }
      //////////////
      vtkInformation *inputInfo(inputVector[0]->GetInformationObject(0));
      std::string meshName;
      std::vector<Grp> groups; std::vector<Fam> fams;
      if(IsInformationOK(inputInfo))
        {
          vtkMutableDirectedGraph *famGrpGraph(vtkMutableDirectedGraph::SafeDownCast(inputInfo->Get(GetMEDReaderMetaDataIfAny())));
          LoadFamGrpMapInfo(famGrpGraph,meshName,groups,fams);
        }
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkDataObject *input(vtkDataObject::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
      if(!input)
        throw MZCException("Not recognized data object in input of the MEDWriter ! Maybe not implemented yet !");
      double timeStep;
      {
        vtkInformation *inInfo(inputVector[0]->GetInformationObject(0));
        vtkDataObject* input(vtkDataObject::GetData(inInfo));
        timeStep=input->GetInformation()->Get(vtkDataObject::DATA_TIME_STEP());
      }
      ////////////
      MCAuto<MEDFileData> mfd(MEDFileData::New());
      WriteMEDFileFromVTKGDS(mfd,input,timeStep,this->CurrentTimeIndex);
      PutFamGrpInfoIfAny(mfd,meshName,groups,fams);
      if(this->WriteAllTimeSteps==0 || (this->WriteAllTimeSteps!=0 && this->CurrentTimeIndex==0))
        mfd->write(this->FileName,2);
      else
        {
          mfd->getFields()->write(this->FileName,0);
        }
      outInfo->Set(vtkDataObject::DATA_OBJECT(),input);
      ///////////
      if(writeAll)
        {
          this->CurrentTimeIndex++;
          if(this->CurrentTimeIndex>=this->NumberOfTimeSteps)
            {
              request->Remove(vtkStreamingDemandDrivenPipeline::CONTINUE_EXECUTING());
              this->CurrentTimeIndex=0;
            }
        }
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      ExceptionDisplayer(this,(const char *) this->FileName,e);
      return 0;
    }
  catch(MZCException& e)
    {
      ExceptionDisplayer(this,(const char *) this->FileName,e);
      return 0;
    }
  return 1;
}

void vtkMEDWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkMEDWriter::Write()
{
  this->Update();
  return 0;
}
