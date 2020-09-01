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

#include "vtkMEDReader.h"
#include "vtkGenerateVectors.h"
#include "MEDUtilities.hxx"

#include "vtkMultiBlockDataSet.h"
#include "vtkInformation.h"
#include "vtkDataSetAttributes.h"
#include "vtkStringArray.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkInformationStringKey.h"
//
#include "vtkUnsignedCharArray.h"
#include "vtkInformationVector.h"
#include "vtkSmartPointer.h"
#include "vtkVariantArray.h"
#include "vtkExecutive.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkMultiTimeStepAlgorithm.h"
#include "vtkUnstructuredGrid.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkQuadratureSchemeDefinition.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkInformationDataObjectMetaDataKey.h"

#ifdef MEDREADER_USE_MPI
#include "vtkMultiProcessController.h"
#include "vtkPUnstructuredGridGhostCellsGenerator.h"
#endif

#include "MEDFileFieldRepresentationTree.hxx"

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

/*!
 * This class stores properties in loading state mode (pvsm) when the MED file has not been read yet.
 * The file is not read beacause FileName has not been informed yet ! So this class stores properties of vtkMEDReader instance that
 * owns it and wait the vtkMEDReader::SetFileName to apply properties afterwards.
 */
class PropertyKeeper
{
public:
  PropertyKeeper(vtkMEDReader *master):IsGVActivated(false),GVValue(0),IsCMActivated(false),CMValue(0),IsGhostActivated(false),GCGCP(1),_master(master) { }
  void assignPropertiesIfNeeded();
  bool arePropertiesOnTreeToSetAfter() const;
  //
  void pushFieldStatusEntry(const char* name, int status);
  void pushGenerateVectorsValue(int value);
  void pushChangeModeValue(int value);
  void pushTimesFlagsStatusEntry(const char* name, int status);
  void pushGhost(int value);
protected:
  // pool of pairs to assign in SetFieldsStatus if needed. The use case is the load using pvsm.
  std::vector< std::pair<std::string,int> > SetFieldsStatusPairs;
  // generate vector
  bool IsGVActivated;
  int GVValue;
  // change mode
  bool IsCMActivated;
  int CMValue;
  // ghost cells
  bool IsGhostActivated;
  int GCGCP;
  //
  std::vector< std::pair<std::string,int> > TimesFlagsStatusPairs;
  vtkMEDReader *_master;
};

void PropertyKeeper::assignPropertiesIfNeeded()
{
  if(!this->SetFieldsStatusPairs.empty())
    {
      for(std::vector< std::pair<std::string,int> >::const_iterator it=this->SetFieldsStatusPairs.begin();it!=this->SetFieldsStatusPairs.end();it++)
        _master->SetFieldsStatus((*it).first.c_str(),(*it).second);
      this->SetFieldsStatusPairs.clear();
    }
  if(!this->TimesFlagsStatusPairs.empty())
    {
      for(std::vector< std::pair<std::string,int> >::const_iterator it=this->TimesFlagsStatusPairs.begin();it!=this->TimesFlagsStatusPairs.end();it++)
        _master->SetTimesFlagsStatus((*it).first.c_str(),(*it).second);
      this->TimesFlagsStatusPairs.clear();
    }
  if(this->IsGVActivated)
    {
      _master->GenerateVectors(this->GVValue);
      this->IsGVActivated=false;
    }
  if(this->IsCMActivated)
    {
      _master->ChangeMode(this->CMValue);
      this->IsCMActivated=false;
    }
  if(this->IsGhostActivated)
    {
      _master->GhostCellGeneratorCallForPara(this->GCGCP);
      this->IsGhostActivated=false;
    }
}

void PropertyKeeper::pushFieldStatusEntry(const char* name, int status)
{
  bool found(false);
  for(std::vector< std::pair<std::string,int> >::const_iterator it=this->SetFieldsStatusPairs.begin();it!=this->SetFieldsStatusPairs.end() && !found;it++)
    found=(*it).first==name;
  if(!found)
    this->SetFieldsStatusPairs.push_back(std::pair<std::string,int>(name,status));
}

void PropertyKeeper::pushTimesFlagsStatusEntry(const char* name, int status)
{
  bool found(false);
  for(std::vector< std::pair<std::string,int> >::const_iterator it=this->TimesFlagsStatusPairs.begin();it!=this->TimesFlagsStatusPairs.end() && !found;it++)
    found=(*it).first==name;
  if(!found)
    this->TimesFlagsStatusPairs.push_back(std::pair<std::string,int>(name,status));
}

void PropertyKeeper::pushGenerateVectorsValue(int value)
{
  this->IsGVActivated=true;
  this->GVValue=value;
}

void PropertyKeeper::pushChangeModeValue(int value)
{
  this->IsCMActivated=true;
  this->CMValue=value;
}

void PropertyKeeper::pushGhost(int value)
{
  this->IsGhostActivated=true;
  this->GCGCP=value;
}

bool PropertyKeeper::arePropertiesOnTreeToSetAfter() const
{
  return !SetFieldsStatusPairs.empty();
}

class vtkMEDReader::vtkMEDReaderInternal
{

public:
  vtkMEDReaderInternal(vtkMEDReader *master):TK(0),IsMEDOrSauv(true),IsStdOrMode(false),GenerateVect(false),SIL(0),LastLev0(-1),PK(master),MyMTime(0),GCGCP(true),FirstCall0(2)
  {
  }

  bool PluginStart0()
  {
    return false; // TODO Useless and buggy
    if(FirstCall0==0)
      return false;
    FirstCall0--;
    return true;
  }

  ~vtkMEDReaderInternal()
  {
    if(this->SIL)
      this->SIL->Delete();
  }
public:
  MEDFileFieldRepresentationTree Tree;
  TimeKeeper TK;
  std::string FileName;
  //when true the file is MED file. when false it is a Sauv file
  bool IsMEDOrSauv;
  //when false -> std, true -> mode. By default std (false).
  bool IsStdOrMode;
  //when false -> do nothing. When true cut off or extend to nbOfCompo=3 vector arrays.
  bool GenerateVect;
  std::string DftMeshName;
  // Store the vtkMutableDirectedGraph that represents links between family, groups and cell types
  vtkMutableDirectedGraph* SIL;
  // store the lev0 id in Tree corresponding to the TIME_STEPS in the pipeline.
  int LastLev0;
  // The property keeper is usable only in pvsm mode.
  PropertyKeeper PK;
  int MyMTime;
  std::set<std::string> _wonderful_set;// this set is used by SetFieldsStatus method to detect the fact that SetFieldsStatus has been called for all items ! Great Items are not sorted ! Why ?
  std::map<std::string,bool> _wonderful_ref;// this map stores the state before a SetFieldsStatus status.
  bool GCGCP;

private:
  unsigned char FirstCall0;
};

vtkStandardNewMacro(vtkMEDReader)

// vtkInformationKeyMacro(vtkMEDReader, META_DATA, DataObjectMetaData); // Here we need to customize vtkMEDReader::META_DATA method
// start of overload of vtkInformationKeyMacro
static vtkInformationDataObjectMetaDataKey *vtkMEDReader_META_DATA=new vtkInformationDataObjectMetaDataKey("META_DATA","vtkMEDReader");

vtkInformationDataObjectMetaDataKey *vtkMEDReader::META_DATA()  
{
  static const char ZE_KEY[]="vtkMEDReader::META_DATA";
  vtkInformationDataObjectMetaDataKey *ret(vtkMEDReader_META_DATA);
  MEDCoupling::GlobalDict *gd(MEDCoupling::GlobalDict::GetInstance());
  if(!gd->hasKey(ZE_KEY))
    {// here META_DATA is put on global var to be exchanged with other filters without dependancy of MEDReader. Please do not change ZE_KEY !
      std::ostringstream oss; oss << ret;
      gd->setKeyValue(ZE_KEY,oss.str());
    }
  return ret;
}

static vtkInformationGaussDoubleVectorKey *vtkMEDReader_GAUSS_DATA=new vtkInformationGaussDoubleVectorKey("GAUSS_DATA","vtkMEDReader");

vtkInformationGaussDoubleVectorKey *vtkMEDReader::GAUSS_DATA()  
{
  static const char ZE_KEY[]="vtkMEDReader::GAUSS_DATA";
  vtkInformationGaussDoubleVectorKey *ret(vtkMEDReader_GAUSS_DATA);
  MEDCoupling::GlobalDict *gd(MEDCoupling::GlobalDict::GetInstance());
  if(!gd->hasKey(ZE_KEY))
    {// here META_DATA is put on global var to be exchanged with other filters without dependancy of MEDReader. Please do not change ZE_KEY !
      vtkInformationDoubleVectorKey *ret2(ret);
      std::ostringstream oss; oss << ret2;
      gd->setKeyValue(ZE_KEY,oss.str());
    }
  return ret;
}
// end of overload of vtkInformationKeyMacro

vtkMEDReader::vtkMEDReader():Internal(new vtkMEDReaderInternal(this))
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkMEDReader::~vtkMEDReader()
{
  delete this->Internal;
  this->Internal = 0;
}

void vtkMEDReader::Reload()
{
  std::string fName((const char *)this->GetFileName());
  delete this->Internal;
  this->Internal=new vtkMEDReaderInternal(this);
  this->SetFileName(fName.c_str());
}

int vtkMEDReader::GetServerModifTime()
{
  if( !this->Internal )
    return -1;
  return this->Internal->MyMTime;
}

void vtkMEDReader::GenerateVectors(int val)
{
  if ( !this->Internal )
    return;
  
  if(this->Internal->FileName.empty())
    {//pvsm mode
      this->Internal->PK.pushGenerateVectorsValue(val);
      return ;
    }
  //not pvsm mode (general case)
  bool val2((bool)val);
  if(val2!=this->Internal->GenerateVect)
    {
      this->Internal->GenerateVect=val2;
      this->Modified();
    }
}

void vtkMEDReader::ChangeMode(int newMode)
{
  if ( !this->Internal )
    return;
  
  if(this->Internal->FileName.empty())
    {//pvsm mode
      this->Internal->PK.pushChangeModeValue(newMode);
      return ;
    }
  //not pvsm mode (general case)
  this->Internal->IsStdOrMode=newMode!=0;
  this->Modified();
}

void vtkMEDReader::GhostCellGeneratorCallForPara(int gcgcp)
{
  if ( !this->Internal )
    return;
  
  if(this->Internal->FileName.empty())
    {//pvsm mode
      this->Internal->PK.pushGhost(gcgcp);
      return ;
    }
  bool newVal(gcgcp!=0);
  if(newVal!=this->Internal->GCGCP)
    {
      this->Internal->GCGCP=newVal;
      this->Modified();
    }
}

const char *vtkMEDReader::GetSeparator()
{
  return MEDFileFieldRepresentationLeavesArrays::ZE_SEP;
}

void vtkMEDReader::SetFileName(const char *fname)
{
  if(!this->Internal)
    return;
  try
    {
      this->Internal->FileName=fname;
      std::size_t pos(this->Internal->FileName.find_last_of('.'));
      if(pos!=std::string::npos)
        {
          std::string ext(this->Internal->FileName.substr(pos));
          if(ext.find("sauv")!=std::string::npos)
            this->Internal->IsMEDOrSauv=false;
        }
      if(this->Internal->Tree.getNumberOfLeavesArrays()==0)
        {
          int iPart(-1),nbOfParts(-1);
#ifdef MEDREADER_USE_MPI
          vtkMultiProcessController *vmpc(vtkMultiProcessController::GetGlobalController());
          if(vmpc)
            {
              iPart=vmpc->GetLocalProcessId();
              nbOfParts=vmpc->GetNumberOfProcesses();
            }
#endif
          this->Internal->Tree.loadMainStructureOfFile(this->Internal->FileName.c_str(),this->Internal->IsMEDOrSauv,iPart,nbOfParts);
          if(!this->Internal->PK.arePropertiesOnTreeToSetAfter())
            this->Internal->Tree.activateTheFirst();//This line manually initialize the status of server (this) with the remote client.
          this->Internal->TK.setMaxNumberOfTimeSteps(this->Internal->Tree.getMaxNumberOfTimeSteps());
        }
      this->Modified();
      this->Internal->PK.assignPropertiesIfNeeded();
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      delete this->Internal;
      this->Internal=0;
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkMEDReader::SetFileName : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
    }
}

char *vtkMEDReader::GetFileName()
{
  if (!this->Internal)
    return 0;
  return const_cast<char *>(this->Internal->FileName.c_str());
}

int vtkMEDReader::RequestInformation(vtkInformation *request, vtkInformationVector ** /*inputVector*/, vtkInformationVector *outputVector)
{
//  std::cout << "########################################## vtkMEDReader::RequestInformation ##########################################" << std::endl;
  if(!this->Internal)
    return 0;
  try
    {
//      request->Print(cout);
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      outInfo->Set(vtkDataObject::DATA_TYPE_NAME(),"vtkMultiBlockDataSet");
      this->UpdateSIL(request, outInfo);

      // Set the meta data graph as a meta data key in the information
      // That's all that is needed to transfer it along the pipeline
      outInfo->Set(vtkMEDReader::META_DATA(),this->Internal->SIL);

      bool dummy(false);
      this->PublishTimeStepsIfNeeded(outInfo,dummy);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkMEDReader::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

int vtkMEDReader::RequestData(vtkInformation *request, vtkInformationVector ** /*inputVector*/, vtkInformationVector *outputVector)
{
//  std::cout << "########################################## vtkMEDReader::RequestData ##########################################" << std::endl;
  if(!this->Internal)
    return 0;
  try
    {
//      request->Print(cout);
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkMultiBlockDataSet *output(vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      //bool isUpdated(false); // todo: unused
      double reqTS(0.);
      if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
        reqTS=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
      ExportedTinyInfo ti;
#ifndef MEDREADER_USE_MPI
      this->FillMultiBlockDataSetInstance(output,reqTS,&ti);
#else
      if(this->Internal->GCGCP)
	{
	  vtkSmartPointer<vtkPUnstructuredGridGhostCellsGenerator> gcg(vtkSmartPointer<vtkPUnstructuredGridGhostCellsGenerator>::New());
	  {
	    vtkDataSet *ret(RetrieveDataSetAtTime(reqTS,&ti));
	    gcg->SetInputData(ret);
	    ret->Delete();
	  }
	  gcg->SetUseGlobalPointIds(true);
	  gcg->SetBuildIfRequired(false);
	  gcg->Update();
	  output->SetBlock(0,gcg->GetOutput());
	}
      else
	this->FillMultiBlockDataSetInstance(output,reqTS,&ti);
#endif
      if(!ti.empty())
        {
          const std::vector<double>& data(ti.getData());
          outInfo->Set(vtkMEDReader::GAUSS_DATA(),&data[0],(int)data.size());
          request->Append(vtkExecutive::KEYS_TO_COPY(),vtkMEDReader::GAUSS_DATA());// Thank you to SciberQuest and DIPOLE_CENTER ! Don't understand why ! In RequestInformation it does not work !
        }
      output->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(),reqTS);
      // Is it really needed ? TODO
      this->UpdateSIL(request, outInfo);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::cerr << "Exception has been thrown in vtkMEDReader::RequestData : " << e.what() << std::endl;
      return 0;
    }
  return 1;
}

void vtkMEDReader::SetFieldsStatus(const char* name, int status)
{
  if( !this->Internal )
    return;

  //this->Internal->_wonderful_set.insert(name);
  if(this->Internal->FileName.empty())
    {//pvsm mode
      this->Internal->PK.pushFieldStatusEntry(name,status);
      return ;
    }
  if(this->Internal->_wonderful_set.empty())
    this->Internal->_wonderful_ref=this->Internal->Tree.dumpState();// start of SetFieldsStatus serie -> store ref to compare at the end of the SetFieldsStatus serie.
  this->Internal->_wonderful_set.insert(name);
  //not pvsm mode (general case)
  try
    {
      this->Internal->Tree.changeStatusOfAndUpdateToHaveCoherentVTKDataSet(this->Internal->Tree.getIdHavingZeName(name),status);
      if((int)this->Internal->_wonderful_set.size()==GetNumberOfFieldsTreeArrays())
        {
          if(this->Internal->_wonderful_ref!=this->Internal->Tree.dumpState())
            {
              if(!this->Internal->PluginStart0())
                {
                  this->Modified();
                }
              this->Internal->MyMTime++;
            }
          this->Internal->_wonderful_set.clear();
        }
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      if(!this->Internal->FileName.empty())
        {
          std::cerr << "vtkMEDReader::SetFieldsStatus error : " << e.what() << " *** WITH STATUS=" << status << endl;
          return ;
        }
    }
}

int vtkMEDReader::GetNumberOfFieldsTreeArrays()
{
  if(!this->Internal)
    return 0;
  return this->Internal->Tree.getNumberOfLeavesArrays();
}

const char *vtkMEDReader::GetFieldsTreeArrayName(int index)
{
  if(!this->Internal)
    return 0;
  return this->Internal->Tree.getNameOfC(index);
}

int vtkMEDReader::GetFieldsTreeArrayStatus(const char *name)
{
  if(!this->Internal)
    return -1;

  int zeId(this->Internal->Tree.getIdHavingZeName(name));
  int ret(this->Internal->Tree.getStatusOf(zeId));
  return ret;
}

void vtkMEDReader::SetTimesFlagsStatus(const char *name, int status)
{
  if (!this->Internal)
    return;
  
  if(this->Internal->FileName.empty())
    {//pvsm mode
      this->Internal->PK.pushTimesFlagsStatusEntry(name,status);
      return ;
    }
  //not pvsm mode (general case)
  int pos(0);
  std::istringstream iss(name); iss >> pos;
  this->Internal->TK.getTimesFlagArray()[pos].first=(bool)status;
  if(pos==(int)this->Internal->TK.getTimesFlagArray().size()-1)
    if(!this->Internal->PluginStart0())
      {
        this->Modified();
        //this->Internal->TK.printSelf(std::cerr);
      }
}

int vtkMEDReader::GetNumberOfTimesFlagsArrays()
{
  if(!this->Internal)
    return 0;
  return (int)this->Internal->TK.getTimesFlagArray().size();
}

const char *vtkMEDReader::GetTimesFlagsArrayName(int index)
{
  return this->Internal->TK.getTimesFlagArray()[index].second.c_str();
}

int vtkMEDReader::GetTimesFlagsArrayStatus(const char *name)
{
  if(!this->Internal)
    return -1;
  int pos(0);
  std::istringstream iss(name); iss >> pos;
  return (int)this->Internal->TK.getTimesFlagArray()[pos].first;
}

void vtkMEDReader::UpdateSIL(vtkInformation* /*request*/, vtkInformation * /*info*/)
{
  if(!this->Internal)
      return;
  std::string meshName(this->Internal->Tree.getActiveMeshName());
  if(!this->Internal->SIL || meshName!=this->Internal->DftMeshName)
    {
      vtkMutableDirectedGraph *sil(vtkMutableDirectedGraph::New());
      this->BuildSIL(sil);
      if(this->Internal->SIL)
        this->Internal->SIL->Delete();
      this->Internal->SIL=sil;
      this->Internal->DftMeshName=meshName;
    }
}

/*!
 * The returned string is the name of the mesh activated which groups and families are in \a sil.
 */
std::string vtkMEDReader::BuildSIL(vtkMutableDirectedGraph* sil)
{
  if (!this->Internal)
    return std::string();
  sil->Initialize();
  vtkSmartPointer<vtkVariantArray> childEdge(vtkSmartPointer<vtkVariantArray>::New());
  childEdge->InsertNextValue(0);
  vtkSmartPointer<vtkVariantArray> crossEdge(vtkSmartPointer<vtkVariantArray>::New());
  crossEdge->InsertNextValue(1);
  // CrossEdge is an edge linking hierarchies.
  vtkUnsignedCharArray* crossEdgesArray=vtkUnsignedCharArray::New();
  crossEdgesArray->SetName("CrossEdges");
  sil->GetEdgeData()->AddArray(crossEdgesArray);
  crossEdgesArray->Delete();
  std::vector<std::string> names;
  // Now build the hierarchy.
  vtkIdType rootId=sil->AddVertex();
  names.push_back("SIL");
  // Add global fields root
  vtkIdType fieldsRoot(sil->AddChild(rootId,childEdge));
  names.push_back("FieldsStatusTree");
  this->Internal->Tree.feedSIL(sil,fieldsRoot,childEdge,names);
  vtkIdType meshesFamsGrpsRoot(sil->AddChild(rootId,childEdge));
  names.push_back("MeshesFamsGrps");
  std::string dftMeshName(this->Internal->Tree.feedSILForFamsAndGrps(sil,meshesFamsGrpsRoot,childEdge,names));
  // This array is used to assign names to nodes.
  vtkStringArray *namesArray(vtkStringArray::New());
  namesArray->SetName("Names");
  namesArray->SetNumberOfTuples(sil->GetNumberOfVertices());
  sil->GetVertexData()->AddArray(namesArray);
  namesArray->Delete();
  std::vector<std::string>::const_iterator iter;
  vtkIdType cc;
  for(cc=0, iter=names.begin(); iter!=names.end(); ++iter, ++cc)
    namesArray->SetValue(cc,(*iter).c_str());
  return dftMeshName;
}

double vtkMEDReader::PublishTimeStepsIfNeeded(vtkInformation *outInfo, bool& isUpdated)
{
  if(!this->Internal)
    return 0.0;

  int lev0(-1);
  std::vector<double> tsteps;
  if(!this->Internal->IsStdOrMode)
    tsteps=this->Internal->Tree.getTimeSteps(lev0,this->Internal->TK);
  else
    { tsteps.resize(1); tsteps[0]=0.; }
  isUpdated=false;
  if(lev0!=this->Internal->LastLev0)
    {
      isUpdated=true;
      double timeRange[2];
      timeRange[0]=tsteps.front();
      timeRange[1]=tsteps.back();
      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),&tsteps[0],(int)tsteps.size());
      outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),timeRange,2);
      this->Internal->LastLev0=lev0;
    }
  return tsteps.front();
}

void vtkMEDReader::FillMultiBlockDataSetInstance(vtkMultiBlockDataSet *output, double reqTS, ExportedTinyInfo *internalInfo)
{
  if( !this->Internal )
    return;
  vtkDataSet *ret(RetrieveDataSetAtTime(reqTS,internalInfo));
  output->SetBlock(0,ret);
  ret->Delete();
}

vtkDataSet *vtkMEDReader::RetrieveDataSetAtTime(double reqTS, ExportedTinyInfo *internalInfo)
{
  if( !this->Internal )
    return 0;
  std::string meshName;
  vtkDataSet *ret(this->Internal->Tree.buildVTKInstance(this->Internal->IsStdOrMode,reqTS,meshName,this->Internal->TK,internalInfo));
  if(this->Internal->GenerateVect)
    {
      vtkGenerateVectors::Operate(ret->GetPointData());
      vtkGenerateVectors::Operate(ret->GetCellData());
      vtkGenerateVectors::Operate(ret->GetFieldData());
      // The operations above have potentially created new arrays -> This breaks the optimization of StaticMesh that expects the same field arrays over time.
      // To enforce the cache recomputation declare modification of mesh.
      //vtkGenerateVectors::ChangeMeshTimeToUpdateCache(ret);
    }
  return ret;
}

void vtkMEDReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
