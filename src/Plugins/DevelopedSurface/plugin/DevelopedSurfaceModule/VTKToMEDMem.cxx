// Copyright (C) 2017-2021  CEA/DEN, EDF R&D
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

#include "VTKToMEDMem.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkLongLongArray.h"
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

#include <map>
#include <deque>
#include <sstream>
#include <cstring>

using VTKToMEDMem::Grp;
using VTKToMEDMem::Fam;

using MEDCoupling::MEDFileData;
using MEDCoupling::MEDFileMesh;
using MEDCoupling::MEDFileCMesh;
using MEDCoupling::MEDFileUMesh;
using MEDCoupling::MEDFileFields;
using MEDCoupling::MEDFileMeshes;

using MEDCoupling::MEDFileInt32Field1TS;
using MEDCoupling::MEDFileInt64Field1TS;
using MEDCoupling::MEDFileField1TS;
using MEDCoupling::MEDFileIntFieldMultiTS;
using MEDCoupling::MEDFileFieldMultiTS;
using MEDCoupling::MEDFileAnyTypeFieldMultiTS;
using MEDCoupling::DataArray;
using MEDCoupling::DataArrayInt32;
using MEDCoupling::DataArrayInt64;
using MEDCoupling::DataArrayFloat;
using MEDCoupling::DataArrayDouble;
using MEDCoupling::MEDCouplingMesh;
using MEDCoupling::MEDCouplingUMesh;
using MEDCoupling::MEDCouplingCMesh;
using MEDCoupling::MEDCouplingFieldDouble;
using MEDCoupling::MEDCouplingFieldFloat;
using MEDCoupling::MEDCouplingFieldInt;
using MEDCoupling::MEDCouplingFieldInt64;
using MEDCoupling::MCAuto;
using MEDCoupling::Traits;
using MEDCoupling::MLFieldTraits;

///////////////////

Fam::Fam(const std::string& name)
{
  static const char ZE_SEP[]="@@][@@";
  std::size_t pos(name.find(ZE_SEP));
  std::string name0(name.substr(0,pos)),name1(name.substr(pos+strlen(ZE_SEP)));
  std::istringstream iss(name1);
  iss >> _id;
  _name=name0;
}

///////////////////

#include "VTKMEDTraits.hxx"

std::map<int,int> ComputeMapOfType()
{
  std::map<int,int> ret;
  int nbOfTypesInMC(sizeof(MEDCOUPLING2VTKTYPETRADUCER)/sizeof( decltype(MEDCOUPLING2VTKTYPETRADUCER[0]) ));
  for(int i=0;i<nbOfTypesInMC;i++)
    {
      auto vtkId(MEDCOUPLING2VTKTYPETRADUCER[i]);
      if(vtkId!=MEDCOUPLING2VTKTYPETRADUCER_NONE)
        ret[vtkId]=i;
    }
  return ret;
}

std::string GetMeshNameWithContext(const std::vector<int>& context)
{
  static const char DFT_MESH_NAME[]="Mesh";
  if(context.empty())
    return DFT_MESH_NAME;
  std::ostringstream oss; oss << DFT_MESH_NAME;
  for(std::vector<int>::const_iterator it=context.begin();it!=context.end();it++)
    oss << "_" << *it;
  return oss.str();
}

DataArrayIdType *ConvertVTKArrayToMCArrayInt(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArrayInt : internal error !");
  int nbTuples(data->GetNumberOfTuples()),nbComp(data->GetNumberOfComponents());
  std::size_t nbElts(nbTuples*nbComp);
  MCAuto<DataArrayIdType> ret(DataArrayIdType::New());
  ret->alloc(nbTuples,nbComp);
  for(int i=0;i<nbComp;i++)
    {
      const char *comp(data->GetComponentName(i));
      if(comp)
        ret->setInfoOnComponent(i,comp);
    }
  mcIdType *ptOut(ret->getPointer());
  vtkIntArray *d0(vtkIntArray::SafeDownCast(data));
  if(d0)
    {
      const int *pt(d0->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  vtkLongArray *d1(vtkLongArray::SafeDownCast(data));
  if(d1)
    {
      const long *pt(d1->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  vtkLongLongArray *d1l(vtkLongLongArray::SafeDownCast(data));
  if(d1l)
    {
      const long long *pt(d1l->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  vtkIdTypeArray *d3(vtkIdTypeArray::SafeDownCast(data));
  if(d3)
    {
      const vtkIdType *pt(d3->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  vtkUnsignedCharArray *d2(vtkUnsignedCharArray::SafeDownCast(data));
  if(d2)
    {
      const unsigned char *pt(d2->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArrayInt : unrecognized array \"" << typeid(*data).name() << "\" type !";
  throw MZCException(oss.str());
}

template<class T>
typename Traits<T>::ArrayType *ConvertVTKArrayToMCArrayDouble(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArrayDouble : internal error !");
  int nbTuples(data->GetNumberOfTuples()),nbComp(data->GetNumberOfComponents());
  std::size_t nbElts(nbTuples*nbComp);
  MCAuto< typename Traits<T>::ArrayType > ret(Traits<T>::ArrayType::New());
  ret->alloc(nbTuples,nbComp);
  for(int i=0;i<nbComp;i++)
    {
      const char *comp(data->GetComponentName(i));
      if(comp)
        ret->setInfoOnComponent(i,comp);
      else
        {
          if(nbComp>1 && nbComp<=3)
            {
              char tmp[2];
              tmp[0]=(char)('X'+i); tmp[1]='\0';
              ret->setInfoOnComponent(i,tmp);
            }
        }
    }
  T *ptOut(ret->getPointer());
  typename MEDFileVTKTraits<T>::VtkType *d0(MEDFileVTKTraits<T>::VtkType::SafeDownCast(data));
  if(d0)
    {
      const T *pt(d0->GetPointer(0));
      for(std::size_t i=0;i<nbElts;i++)
        ptOut[i]=(T)pt[i];
      return ret.retn();
    }
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArrayDouble : unrecognized array \"" << data->GetClassName() << "\" type !";
  throw MZCException(oss.str());
}

DataArrayDouble *ConvertVTKArrayToMCArrayDoubleForced(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArrayDoubleForced : internal error 0 !");
  vtkFloatArray *d0(vtkFloatArray::SafeDownCast(data));
  if(d0)
    {
      MCAuto<DataArrayFloat> ret(ConvertVTKArrayToMCArrayDouble<float>(data));
      MCAuto<DataArrayDouble> ret2(ret->convertToDblArr());
      return ret2.retn();
    }
  vtkDoubleArray *d1(vtkDoubleArray::SafeDownCast(data));
  if(d1)
    return ConvertVTKArrayToMCArrayDouble<double>(data);
  throw MZCException("ConvertVTKArrayToMCArrayDoubleForced : unrecognized type of data for double !");
}

DataArray *ConvertVTKArrayToMCArray(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArray : internal error !");
  vtkFloatArray *d0(vtkFloatArray::SafeDownCast(data));
  if(d0)
    return ConvertVTKArrayToMCArrayDouble<float>(data);
  vtkDoubleArray *d1(vtkDoubleArray::SafeDownCast(data));
  if(d1)
    return ConvertVTKArrayToMCArrayDouble<double>(data);
  vtkIntArray *d2(vtkIntArray::SafeDownCast(data));
  vtkLongArray *d3(vtkLongArray::SafeDownCast(data));
  vtkUnsignedCharArray *d4(vtkUnsignedCharArray::SafeDownCast(data));
  vtkIdTypeArray *d5(vtkIdTypeArray::SafeDownCast(data));
  vtkLongLongArray *d6(vtkLongLongArray::SafeDownCast(data));
  if(d2 || d3 || d4 || d5 || d6)
    return ConvertVTKArrayToMCArrayInt(data);
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArray : unrecognized array \"" << typeid(*data).name() << "\" type !";
  throw MZCException(oss.str());
}

MEDCouplingUMesh *BuildMeshFromCellArray(vtkCellArray *ca, DataArrayDouble *coords, int meshDim, INTERP_KERNEL::NormalizedCellType type)
{
  MCAuto<MEDCouplingUMesh> subMesh(MEDCouplingUMesh::New("",meshDim));
  subMesh->setCoords(coords); subMesh->allocateCells();
  int nbCells(ca->GetNumberOfCells());
  if(nbCells==0)
    return 0;
  //vtkIdType nbEntries(ca->GetNumberOfConnectivityEntries()); // todo: unused
  const vtkIdType *conn(ca->GetData()->GetPointer(0));
  for(int i=0;i<nbCells;i++)
    {
      mcIdType sz(ToIdType(*conn++));
      std::vector<mcIdType> conn2(sz);
      for(int jj=0;jj<sz;jj++)
        conn2[jj]=ToIdType(conn[jj]);
      subMesh->insertNextCell(type,sz,&conn2[0]);
      conn+=sz;
    }
  return subMesh.retn();
}

MEDCouplingUMesh *BuildMeshFromCellArrayTriangleStrip(vtkCellArray *ca, DataArrayDouble *coords, MCAuto<DataArrayIdType>& ids)
{
  MCAuto<MEDCouplingUMesh> subMesh(MEDCouplingUMesh::New("",2));
  subMesh->setCoords(coords); subMesh->allocateCells();
  int nbCells(ca->GetNumberOfCells());
  if(nbCells==0)
    return 0;
  //vtkIdType nbEntries(ca->GetNumberOfConnectivityEntries()); // todo: unused
  const vtkIdType *conn(ca->GetData()->GetPointer(0));
  ids=DataArrayIdType::New() ; ids->alloc(0,1);
  for(int i=0;i<nbCells;i++)
    {
      int sz(*conn++);
      int nbTri(sz-2);
      if(nbTri>0)
        {
          for(int j=0;j<nbTri;j++,conn++)
            {
              mcIdType conn2[3]; conn2[0]=conn[0] ; conn2[1]=conn[1] ; conn2[2]=conn[2];
              subMesh->insertNextCell(INTERP_KERNEL::NORM_TRI3,3,conn2);
              ids->pushBackSilent(i);
            }
        }
      else
        {
          std::ostringstream oss; oss << "BuildMeshFromCellArrayTriangleStrip : on cell #" << i << " the triangle stip looks bab !";
          throw MZCException(oss.str());
        }
      conn+=sz;
    }
  return subMesh.retn();
}

class MicroField
{
public:
  MicroField(const MCAuto<MEDCouplingUMesh>& m, const std::vector<MCAuto<DataArray> >& cellFs):_m(m),_cellFs(cellFs) { }
  MicroField(const std::vector< MicroField >& vs);
  void setNodeFields(const std::vector<MCAuto<DataArray> >& nf) { _nodeFs=nf; }
  MCAuto<MEDCouplingUMesh> getMesh() const { return _m; }
  std::vector<MCAuto<DataArray> > getCellFields() const { return _cellFs; }
private:
  MCAuto<MEDCouplingUMesh> _m;
  std::vector<MCAuto<DataArray> > _cellFs;
  std::vector<MCAuto<DataArray> > _nodeFs;
};

MicroField::MicroField(const std::vector< MicroField >& vs)
{
  std::size_t sz(vs.size());
  std::vector<const MEDCouplingUMesh *> vs2(sz);
  std::vector< std::vector< MCAuto<DataArray> > > arrs2(sz);
  int nbElts(-1);
  for(std::size_t ii=0;ii<sz;ii++)
    {
      vs2[ii]=vs[ii].getMesh();
      arrs2[ii]=vs[ii].getCellFields();
      if(nbElts<0)
        nbElts=(int)arrs2[ii].size();
      else
        if((int)arrs2[ii].size()!=nbElts)
          throw MZCException("MicroField cstor : internal error !");
    }
  _cellFs.resize(nbElts);
  for(int ii=0;ii<nbElts;ii++)
    {
      std::vector<const DataArray *> arrsTmp(sz);
      for(std::size_t jj=0;jj<sz;jj++)
        {
          arrsTmp[jj]=arrs2[jj][ii];
        }
      _cellFs[ii]=DataArray::Aggregate(arrsTmp);
    }
  _m=MEDCouplingUMesh::MergeUMeshesOnSameCoords(vs2);
}

template<class T>
void AppendToFields(MEDCoupling::TypeOfField tf, MEDCouplingMesh *mesh, const DataArrayIdType *n2oPtr, typename MEDFileVTKTraits<T>::MCType *dadPtr, MEDFileFields *fs, double timeStep, int tsId)
{
  std::string fieldName(dadPtr->getName());
  MCAuto< typename Traits<T>::FieldType > f(Traits<T>::FieldType::New(tf));
  f->setTime(timeStep,tsId,0);
  {
    std::string fieldNameForChuckNorris(MEDCoupling::MEDFileAnyTypeField1TSWithoutSDA::FieldNameToMEDFileConvention(fieldName));
    f->setName(fieldNameForChuckNorris);
  }
  if(!n2oPtr)
    f->setArray(dadPtr);
  else
    {
      MCAuto< typename Traits<T>::ArrayType > dad2(dadPtr->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
      f->setArray(dad2);
    }
  f->setMesh(mesh);
  MCAuto< typename MLFieldTraits<T>::FMTSType > fmts(MLFieldTraits<T>::FMTSType::New());
  MCAuto< typename MLFieldTraits<T>::F1TSType > f1ts(MLFieldTraits<T>::F1TSType::New());
  f1ts->setFieldNoProfileSBT(f);
  fmts->pushBackTimeStep(f1ts);
  fs->pushField(fmts);
}

void AppendMCFieldFrom(MEDCoupling::TypeOfField tf, MEDCouplingMesh *mesh, MEDFileData *mfd, MCAuto<DataArray> da, const DataArrayIdType *n2oPtr, double timeStep, int tsId)
{
  static const char FAMFIELD_FOR_CELLS[]="FamilyIdCell";
  static const char FAMFIELD_FOR_NODES[]="FamilyIdNode";
  if(!da || !mesh || !mfd)
    throw MZCException("AppendMCFieldFrom : internal error !");
  MEDFileFields *fs(mfd->getFields());
  MEDFileMeshes *ms(mfd->getMeshes());
  if(!fs || !ms)
    throw MZCException("AppendMCFieldFrom : internal error 2 !");
  MCAuto<DataArrayDouble> dad(MEDCoupling::DynamicCast<DataArray,DataArrayDouble>(da));
  if(dad.isNotNull())
    {
      AppendToFields<double>(tf,mesh,n2oPtr,dad,fs,timeStep,tsId);
      return ;
    }
  MCAuto<DataArrayFloat> daf(MEDCoupling::DynamicCast<DataArray,DataArrayFloat>(da));
  if(daf.isNotNull())
    {
      AppendToFields<float>(tf,mesh,n2oPtr,daf,fs,timeStep,tsId);
      return ;
    }
  MCAuto<DataArrayInt> dai(MEDCoupling::DynamicCast<DataArray,DataArrayInt>(da));
  MCAuto<DataArrayIdType> daId(MEDCoupling::DynamicCast<DataArray,DataArrayIdType>(da));
  if(dai.isNotNull() || daId.isNotNull())
    {
      std::string fieldName(da->getName());
      if((fieldName!=FAMFIELD_FOR_CELLS || tf!=MEDCoupling::ON_CELLS) && (fieldName!=FAMFIELD_FOR_NODES || tf!=MEDCoupling::ON_NODES))
        {
          if(!dai)
            AppendToFields<mcIdType>(tf,mesh,n2oPtr,daId,fs,timeStep,tsId);
          else
            AppendToFields<int>(tf,mesh,n2oPtr,dai,fs,timeStep,tsId);
          return ;
        }
      else if(fieldName==FAMFIELD_FOR_CELLS && tf==MEDCoupling::ON_CELLS)
        {
          MEDFileMesh *mm(ms->getMeshWithName(mesh->getName()));
          if(!mm)
            throw MZCException("AppendMCFieldFrom : internal error 3 !");
          if(!daId)
            throw MZCException("AppendMCFieldFrom : internal error 3 (not mcIdType) !");
          if(!n2oPtr)
            mm->setFamilyFieldArr(mesh->getMeshDimension()-mm->getMeshDimension(),daId);
          else
            {
              MCAuto<DataArrayIdType> dai2(daId->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
              mm->setFamilyFieldArr(mesh->getMeshDimension()-mm->getMeshDimension(),dai2);
            }
        }
      else if(fieldName==FAMFIELD_FOR_NODES || tf==MEDCoupling::ON_NODES)
        {
          MEDFileMesh *mm(ms->getMeshWithName(mesh->getName()));
          if(!mm)
            throw MZCException("AppendMCFieldFrom : internal error 4 !");
          if(!daId)
            throw MZCException("AppendMCFieldFrom : internal error 4 (not mcIdType) !");
          if(!n2oPtr)
            mm->setFamilyFieldArr(1,daId);
          else
            {
              MCAuto<DataArrayIdType> dai2(daId->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
              mm->setFamilyFieldArr(1,dai2);
            }
        }
    }
}

void PutAtLevelDealOrder(MEDFileData *mfd, int meshDimRel, const MicroField& mf, double timeStep, int tsId)
{
  if(!mfd)
    throw MZCException("PutAtLevelDealOrder : internal error !");
  MEDFileMesh *mm(mfd->getMeshes()->getMeshAtPos(0));
  MEDFileUMesh *mmu(dynamic_cast<MEDFileUMesh *>(mm));
  if(!mmu)
    throw MZCException("PutAtLevelDealOrder : internal error 2 !");
  MCAuto<MEDCouplingUMesh> mesh(mf.getMesh());
  mesh->setName(mfd->getMeshes()->getMeshAtPos(0)->getName());
  MCAuto<DataArrayIdType> o2n(mesh->sortCellsInMEDFileFrmt());
  //const DataArrayIdType *o2nPtr(o2n); // todo: unused
  MCAuto<DataArrayIdType> n2o;
  mmu->setMeshAtLevel(meshDimRel,mesh);
  const DataArrayIdType *n2oPtr(0);
  if(o2n)
    {
      n2o=o2n->invertArrayO2N2N2O(mesh->getNumberOfCells());
      n2oPtr=n2o;
      if(n2oPtr && n2oPtr->isIota(mesh->getNumberOfCells()))
        n2oPtr=0;
      if(n2oPtr)
        mm->setRenumFieldArr(meshDimRel,n2o);
    }
  //
  std::vector<MCAuto<DataArray> > cells(mf.getCellFields());
  for(std::vector<MCAuto<DataArray> >::const_iterator it=cells.begin();it!=cells.end();it++)
    {
      MCAuto<DataArray> da(*it);
      AppendMCFieldFrom(MEDCoupling::ON_CELLS,mesh,mfd,da,n2oPtr,timeStep,tsId);
    }
}

void AssignSingleGTMeshes(MEDFileData *mfd, const std::vector< MicroField >& ms, double timeStep, int tsId)
{
  if(!mfd)
    throw MZCException("AssignSingleGTMeshes : internal error !");
  MEDFileMesh *mm0(mfd->getMeshes()->getMeshAtPos(0));
  MEDFileUMesh *mm(dynamic_cast<MEDFileUMesh *>(mm0));
  if(!mm)
    throw MZCException("AssignSingleGTMeshes : internal error 2 !");
  int meshDim(-std::numeric_limits<int>::max());
  std::map<int, std::vector< MicroField > > ms2;
  for(std::vector< MicroField >::const_iterator it=ms.begin();it!=ms.end();it++)
    {
      const MEDCouplingUMesh *elt((*it).getMesh());
      if(elt)
        {
          int myMeshDim(elt->getMeshDimension());
          meshDim=std::max(meshDim,myMeshDim);
          ms2[myMeshDim].push_back(*it);
        }
    }
  if(ms2.empty())
    return ;
  for(std::map<int, std::vector< MicroField > >::const_iterator it=ms2.begin();it!=ms2.end();it++)
    {
      const std::vector< MicroField >& vs((*it).second);
      if(vs.size()==1)
        {
          PutAtLevelDealOrder(mfd,(*it).first-meshDim,vs[0],timeStep,tsId);
        }
      else
        {
          MicroField merge(vs);
          PutAtLevelDealOrder(mfd,(*it).first-meshDim,merge,timeStep,tsId);
        }
    }
}

DataArrayDouble *BuildCoordsFrom(vtkPointSet *ds)
{
  if(!ds)
    throw MZCException("BuildCoordsFrom : internal error !");
  vtkPoints *pts(ds->GetPoints());
  if(!pts)
    throw MZCException("BuildCoordsFrom : internal error 2 !");
  vtkDataArray *data(pts->GetData());
  if(!data)
    throw MZCException("BuildCoordsFrom : internal error 3 !");
  return ConvertVTKArrayToMCArrayDoubleForced(data);
}

void AddNodeFields(MEDFileData *mfd, vtkDataSetAttributes *dsa, double timeStep, int tsId)
{
  if(!mfd || !dsa)
    throw MZCException("AddNodeFields : internal error !");
  MEDFileMesh *mm(mfd->getMeshes()->getMeshAtPos(0));
  MEDFileUMesh *mmu(dynamic_cast<MEDFileUMesh *>(mm));
  if(!mmu)
    throw MZCException("AddNodeFields : internal error 2 !");
  MCAuto<MEDCouplingUMesh> mesh;
  if(!mmu->getNonEmptyLevels().empty())
    mesh=mmu->getMeshAtLevel(0);
  else
    {
      mesh=MEDCouplingUMesh::Build0DMeshFromCoords(mmu->getCoords());
      mesh->setName(mmu->getName());
    }
  int nba(dsa->GetNumberOfArrays());
  for(int i=0;i<nba;i++)
    {
      vtkDataArray *arr(dsa->GetArray(i));
      const char *name(arr->GetName());
      if(!arr)
        continue;
      MCAuto<DataArray> da(ConvertVTKArrayToMCArray(arr));
      da->setName(name);
      AppendMCFieldFrom(MEDCoupling::ON_NODES,mesh,mfd,da,NULL,timeStep,tsId);
    }
}

std::vector<MCAuto<DataArray> > AddPartFields(const DataArrayIdType *part, vtkDataSetAttributes *dsa)
{
  std::vector< MCAuto<DataArray> > ret;
  if(!dsa)
    return ret;
  int nba(dsa->GetNumberOfArrays());
  for(int i=0;i<nba;i++)
    {
      vtkDataArray *arr(dsa->GetArray(i));
      if(!arr)
        continue;
      const char *name(arr->GetName());
      //int nbCompo(arr->GetNumberOfComponents()); // todo: unused
      //vtkIdType nbTuples(arr->GetNumberOfTuples()); // todo: unused
      MCAuto<DataArray> mcarr(ConvertVTKArrayToMCArray(arr));
      if(part)
        mcarr=mcarr->selectByTupleId(part->begin(),part->end());
      mcarr->setName(name);
      ret.push_back(mcarr);
    }
  return ret;
}

std::vector<MCAuto<DataArray> > AddPartFields2(int bg, int end, vtkDataSetAttributes *dsa)
{
  std::vector< MCAuto<DataArray> > ret;
  if(!dsa)
    return ret;
  int nba(dsa->GetNumberOfArrays());
  for(int i=0;i<nba;i++)
    {
      vtkDataArray *arr(dsa->GetArray(i));
      if(!arr)
        continue;
      const char *name(arr->GetName());
      //int nbCompo(arr->GetNumberOfComponents()); // todo: unused
      //vtkIdType nbTuples(arr->GetNumberOfTuples()); // todo: unused
      MCAuto<DataArray> mcarr(ConvertVTKArrayToMCArray(arr));
      mcarr=mcarr->selectByTupleIdSafeSlice(bg,end,1);
      mcarr->setName(name);
      ret.push_back(mcarr);
    }
  return ret;
}

void ConvertFromRectilinearGrid(MEDFileData *ret, vtkRectilinearGrid *ds, const std::vector<int>& context, double timeStep, int tsId)
{
  if(!ds || !ret)
    throw MZCException("ConvertFromRectilinearGrid : internal error !");
  //
  MCAuto<MEDFileMeshes> meshes(MEDFileMeshes::New());
  ret->setMeshes(meshes);
  MCAuto<MEDFileFields> fields(MEDFileFields::New());
  ret->setFields(fields);
  //
  MCAuto<MEDFileCMesh> cmesh(MEDFileCMesh::New());
  meshes->pushMesh(cmesh);
  MCAuto<MEDCouplingCMesh> cmeshmc(MEDCouplingCMesh::New());
  vtkDataArray *cx(ds->GetXCoordinates()),*cy(ds->GetYCoordinates()),*cz(ds->GetZCoordinates());
  if(cx)
    {
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDoubleForced(cx));
      cmeshmc->setCoordsAt(0,arr);
    }
  if(cy)
    {
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDoubleForced(cy));
      cmeshmc->setCoordsAt(1,arr);
    }
  if(cz)
    {
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDoubleForced(cz));
      cmeshmc->setCoordsAt(2,arr);
    }
  std::string meshName(GetMeshNameWithContext(context));
  cmeshmc->setName(meshName);
  cmesh->setMesh(cmeshmc);
  std::vector<MCAuto<DataArray> > cellFs(AddPartFields(0,ds->GetCellData()));
  for(std::vector<MCAuto<DataArray> >::const_iterator it=cellFs.begin();it!=cellFs.end();it++)
    {
      MCAuto<DataArray> da(*it);
      AppendMCFieldFrom(MEDCoupling::ON_CELLS,cmeshmc,ret,da,NULL,timeStep,tsId);
    }
  std::vector<MCAuto<DataArray> > nodeFs(AddPartFields(0,ds->GetPointData()));
  for(std::vector<MCAuto<DataArray> >::const_iterator it=nodeFs.begin();it!=nodeFs.end();it++)
    {
      MCAuto<DataArray> da(*it);
      AppendMCFieldFrom(MEDCoupling::ON_NODES,cmeshmc,ret,da,NULL,timeStep,tsId);
    }
}

void ConvertFromPolyData(MEDFileData *ret, vtkPolyData *ds, const std::vector<int>& context, double timeStep, int tsId)
{
  if(!ds || !ret)
    throw MZCException("ConvertFromPolyData : internal error !");
  //
  MCAuto<MEDFileMeshes> meshes(MEDFileMeshes::New());
  ret->setMeshes(meshes);
  MCAuto<MEDFileFields> fields(MEDFileFields::New());
  ret->setFields(fields);
  //
  MCAuto<MEDFileUMesh> umesh(MEDFileUMesh::New());
  meshes->pushMesh(umesh);
  MCAuto<DataArrayDouble> coords(BuildCoordsFrom(ds));
  umesh->setCoords(coords);
  umesh->setName(GetMeshNameWithContext(context));
  //
  int offset(0);
  std::vector< MicroField > ms;
  vtkCellArray *cd(ds->GetVerts());
  if(cd)
    {
      MCAuto<MEDCouplingUMesh> subMesh(BuildMeshFromCellArray(cd,coords,0,INTERP_KERNEL::NORM_POINT1));
      if((const MEDCouplingUMesh *)subMesh)
        {
          std::vector<MCAuto<DataArray> > cellFs(AddPartFields2(offset,offset+subMesh->getNumberOfCells(),ds->GetCellData()));
          offset+=subMesh->getNumberOfCells();
          ms.push_back(MicroField(subMesh,cellFs));
        }
    }
  vtkCellArray *cc(ds->GetLines());
  if(cc)
    {
      MCAuto<MEDCouplingUMesh> subMesh;
      try
        {
          subMesh=BuildMeshFromCellArray(cc,coords,1,INTERP_KERNEL::NORM_SEG2);
        }
      catch(INTERP_KERNEL::Exception& e)
        {
          std::ostringstream oss; oss << "MEDWriter does not manage polyline cell type because MED file format does not support it ! Maybe it is the source of the problem ? The cause of this exception was " << e.what() << std::endl;
          throw INTERP_KERNEL::Exception(oss.str());
        }
      if((const MEDCouplingUMesh *)subMesh)
        {
          std::vector<MCAuto<DataArray> > cellFs(AddPartFields2(offset,offset+subMesh->getNumberOfCells(),ds->GetCellData()));
          offset+=subMesh->getNumberOfCells();
          ms.push_back(MicroField(subMesh,cellFs));
        }
    }
  vtkCellArray *cb(ds->GetPolys());
  if(cb)
    {
      MCAuto<MEDCouplingUMesh> subMesh(BuildMeshFromCellArray(cb,coords,2,INTERP_KERNEL::NORM_POLYGON));
      if((const MEDCouplingUMesh *)subMesh)
        {
          std::vector<MCAuto<DataArray> > cellFs(AddPartFields2(offset,offset+subMesh->getNumberOfCells(),ds->GetCellData()));
          offset+=subMesh->getNumberOfCells();
          ms.push_back(MicroField(subMesh,cellFs));
        }
    }
  vtkCellArray *ca(ds->GetStrips());
  if(ca)
    {
      MCAuto<DataArrayIdType> ids;
      MCAuto<MEDCouplingUMesh> subMesh(BuildMeshFromCellArrayTriangleStrip(ca,coords,ids));
      if((const MEDCouplingUMesh *)subMesh)
        {
          std::vector<MCAuto<DataArray> > cellFs(AddPartFields(ids,ds->GetCellData()));
          offset+=subMesh->getNumberOfCells();
          ms.push_back(MicroField(subMesh,cellFs));
        }
    }
  AssignSingleGTMeshes(ret,ms,timeStep,tsId);
  AddNodeFields(ret,ds->GetPointData(),timeStep,tsId);
}

void ConvertFromUnstructuredGrid(MEDFileData *ret, vtkUnstructuredGrid *ds, const std::vector<int>& context, double timeStep, int tsId)
{
  if(!ds || !ret)
    throw MZCException("ConvertFromUnstructuredGrid : internal error !");
  //
  MCAuto<MEDFileMeshes> meshes(MEDFileMeshes::New());
  ret->setMeshes(meshes);
  MCAuto<MEDFileFields> fields(MEDFileFields::New());
  ret->setFields(fields);
  //
  MCAuto<MEDFileUMesh> umesh(MEDFileUMesh::New());
  meshes->pushMesh(umesh);
  MCAuto<DataArrayDouble> coords(BuildCoordsFrom(ds));
  umesh->setCoords(coords);
  umesh->setName(GetMeshNameWithContext(context));
  vtkIdType nbCells(ds->GetNumberOfCells());
  vtkCellArray *ca(ds->GetCells());
  if(!ca)
    return ;
  //vtkIdType nbEnt(ca->GetNumberOfConnectivityEntries()); // todo: unused
  //vtkIdType *caPtr(ca->GetData()->GetPointer(0)); // todo: unused
  vtkUnsignedCharArray *ct(ds->GetCellTypesArray());
  if(!ct)
    throw MZCException("ConvertFromUnstructuredGrid : internal error");
  vtkIdTypeArray *cla(ds->GetCellLocationsArray());
  //const vtkIdType *claPtr(cla->GetPointer(0)); // todo: unused
  if(!cla)
    throw MZCException("ConvertFromUnstructuredGrid : internal error 2");
  const unsigned char *ctPtr(ct->GetPointer(0));
  std::map<int,int> m(ComputeMapOfType());
  MCAuto<DataArrayInt> lev(DataArrayInt::New()) ;  lev->alloc(nbCells,1);
  int *levPtr(lev->getPointer());
  for(vtkIdType i=0;i<nbCells;i++)
    {
      std::map<int,int>::iterator it(m.find(ctPtr[i]));
      if(it!=m.end())
        {
          const INTERP_KERNEL::CellModel& cm(INTERP_KERNEL::CellModel::GetCellModel((INTERP_KERNEL::NormalizedCellType)(*it).second));
          levPtr[i]=cm.getDimension();
        }
      else
        {
          if(ctPtr[i]==VTK_POLY_VERTEX)
            {
              const INTERP_KERNEL::CellModel& cm(INTERP_KERNEL::CellModel::GetCellModel(INTERP_KERNEL::NORM_POINT1));
              levPtr[i]=cm.getDimension();
            }
          else
            {
              std::ostringstream oss; oss << "ConvertFromUnstructuredGrid : at pos #" << i << " unrecognized VTK cell with type =" << ctPtr[i];
              throw MZCException(oss.str());
            }
        }
    }
  MCAuto<DataArrayInt> levs(lev->getDifferentValues());
  std::vector< MicroField > ms;
  //vtkIdTypeArray *faces(ds->GetFaces()),*faceLoc(ds->GetFaceLocations()); // todo: unused
  for(const int *curLev=levs->begin();curLev!=levs->end();curLev++)
    {
      MCAuto<MEDCouplingUMesh> m0(MEDCouplingUMesh::New("",*curLev));
      m0->setCoords(coords); m0->allocateCells();
      MCAuto<DataArrayIdType> cellIdsCurLev(lev->findIdsEqual(*curLev));
      for(const mcIdType *cellId=cellIdsCurLev->begin();cellId!=cellIdsCurLev->end();cellId++)
        {
          int vtkType(ds->GetCellType(*cellId));
          std::map<int,int>::iterator it(m.find(vtkType));
          INTERP_KERNEL::NormalizedCellType ct=it!=m.end()?(INTERP_KERNEL::NormalizedCellType)((*it).second):INTERP_KERNEL::NORM_POINT1;
          if(ct!=INTERP_KERNEL::NORM_POLYHED && vtkType!=VTK_POLY_VERTEX)
            {
              vtkIdType sz(0);
              const vtkIdType *pts(nullptr);
              ds->GetCellPoints(*cellId, sz, pts);
              std::vector<mcIdType> conn2(pts,pts+sz);
              m0->insertNextCell(ct,sz,conn2.data());
            }
          else if(ct==INTERP_KERNEL::NORM_POLYHED)
            {
              // # de faces du polyèdre
              vtkIdType nbOfFaces(0);
              // connectivé des faces (numFace0Pts, id1, id2, id3, numFace1Pts,id1, id2, id3, ...)
              const vtkIdType *facPtr(nullptr);
              ds->GetFaceStream(*cellId, nbOfFaces, facPtr);
              std::vector<mcIdType> conn;
              for(vtkIdType k=0;k<nbOfFaces;k++)
                {
                  vtkIdType nbOfNodesInFace(*facPtr++);
                  std::copy(facPtr,facPtr+nbOfNodesInFace,std::back_inserter(conn));
                  if(k<nbOfFaces-1)
                    conn.push_back(-1);
                  facPtr+=nbOfNodesInFace;
                }
              m0->insertNextCell(ct,ToIdType(conn.size()),&conn[0]);
            }
          else
            {
              vtkIdType sz(0);
              const vtkIdType *pts(nullptr);
              ds->GetCellPoints(*cellId, sz, pts);
              if(sz!=1)
                throw MZCException("ConvertFromUnstructuredGrid : non single poly vertex not managed by MED !");
              m0->insertNextCell(ct,1,(const mcIdType*)pts);
            }
        }
      std::vector<MCAuto<DataArray> > cellFs(AddPartFields(cellIdsCurLev,ds->GetCellData()));
      ms.push_back(MicroField(m0,cellFs));
    }
  AssignSingleGTMeshes(ret,ms,timeStep,tsId);
  AddNodeFields(ret,ds->GetPointData(),timeStep,tsId);
}

///////////////////

void WriteMEDFileFromVTKDataSet(MEDFileData *mfd, vtkDataSet *ds, const std::vector<int>& context, double timeStep, int tsId)
{
  if(!ds || !mfd)
    throw MZCException("Internal error in WriteMEDFileFromVTKDataSet.");
  vtkPolyData *ds2(vtkPolyData::SafeDownCast(ds));
  vtkUnstructuredGrid *ds3(vtkUnstructuredGrid::SafeDownCast(ds));
  vtkRectilinearGrid *ds4(vtkRectilinearGrid::SafeDownCast(ds));
  if(ds2)
    {
      ConvertFromPolyData(mfd,ds2,context,timeStep,tsId);
    }
  else if(ds3)
    {
      ConvertFromUnstructuredGrid(mfd,ds3,context,timeStep,tsId);
    }
  else if(ds4)
    {
      ConvertFromRectilinearGrid(mfd,ds4,context,timeStep,tsId);
    }
  else
    throw MZCException("Unrecognized vtkDataSet ! Sorry ! Try to convert it to UnstructuredGrid to be able to write it !");
}

void WriteMEDFileFromVTKMultiBlock(MEDFileData *mfd, vtkMultiBlockDataSet *ds, const std::vector<int>& context, double timeStep, int tsId)
{
  if(!ds || !mfd)
    throw MZCException("Internal error in WriteMEDFileFromVTKMultiBlock.");
  int nbBlocks(ds->GetNumberOfBlocks());
  if(nbBlocks==1 && context.empty())
    {
      vtkDataObject *uniqueElt(ds->GetBlock(0));
      if(!uniqueElt)
        throw MZCException("Unique elt in multiblock is NULL !");
      vtkDataSet *uniqueEltc(vtkDataSet::SafeDownCast(uniqueElt));
      if(uniqueEltc)
        {
          WriteMEDFileFromVTKDataSet(mfd,uniqueEltc,context,timeStep,tsId);
          return ;
        }
    }
  for(int i=0;i<nbBlocks;i++)
    {
      vtkDataObject *elt(ds->GetBlock(i));
      std::vector<int> context2;
      context2.push_back(i);
      if(!elt)
        {
          std::ostringstream oss; oss << "In context ";
          std::copy(context.begin(),context.end(),std::ostream_iterator<int>(oss," "));
          oss << " at pos #" << i << " elt is NULL !";
          throw MZCException(oss.str());
        }
      vtkDataSet *elt1(vtkDataSet::SafeDownCast(elt));
      if(elt1)
        {
          WriteMEDFileFromVTKDataSet(mfd,elt1,context,timeStep,tsId);
          continue;
        }
      vtkMultiBlockDataSet *elt2(vtkMultiBlockDataSet::SafeDownCast(elt));
      if(elt2)
        {
          WriteMEDFileFromVTKMultiBlock(mfd,elt2,context,timeStep,tsId);
          continue;
        }
      std::ostringstream oss; oss << "In context ";
      std::copy(context.begin(),context.end(),std::ostream_iterator<int>(oss," "));
      oss << " at pos #" << i << " elt not recognized data type !";
      throw MZCException(oss.str());
    }
}

void WriteMEDFileFromVTKGDS(MEDFileData *mfd, vtkDataObject *input, double timeStep, int tsId)
{
  if(!input || !mfd)
    throw MZCException("WriteMEDFileFromVTKGDS : internal error !");
  std::vector<int> context;
  vtkDataSet *input1(vtkDataSet::SafeDownCast(input));
  if(input1)
    {
      WriteMEDFileFromVTKDataSet(mfd,input1,context,timeStep,tsId);
      return ;
    }
  vtkMultiBlockDataSet *input2(vtkMultiBlockDataSet::SafeDownCast(input));
  if(input2)
    {
      WriteMEDFileFromVTKMultiBlock(mfd,input2,context,timeStep,tsId);
      return ;
    }
  throw MZCException("WriteMEDFileFromVTKGDS : not recognized data type !");
}

void PutFamGrpInfoIfAny(MEDFileData *mfd, const std::string& meshName, const std::vector<Grp>& groups, const std::vector<Fam>& fams)
{
  if(!mfd)
    return ;
  if(meshName.empty())
    return ;
  MEDFileMeshes *meshes(mfd->getMeshes());
  if(!meshes)
    return ;
  if(meshes->getNumberOfMeshes()!=1)
    return ;
  MEDFileMesh *mm(meshes->getMeshAtPos(0));
  if(!mm)
    return ;
  mm->setName(meshName);
  for(std::vector<Fam>::const_iterator it=fams.begin();it!=fams.end();it++)
    mm->setFamilyId((*it).getName(),(*it).getID());
  for(std::vector<Grp>::const_iterator it=groups.begin();it!=groups.end();it++)
    mm->setFamiliesOnGroup((*it).getName(),(*it).getFamilies());
  MEDFileFields *fields(mfd->getFields());
  if(!fields)
    return ;
  for(int i=0;i<fields->getNumberOfFields();i++)
    {
      MEDFileAnyTypeFieldMultiTS *fmts(fields->getFieldAtPos(i));
      if(!fmts)
        continue;
      fmts->setMeshName(meshName);
    }
}
