// Copyright (C) 2016  CEA/DEN, EDF R&D
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
#include "MEDCouplingFieldDouble.hxx"
#include "MEDCouplingRefCountObject.hxx"

#include <map>
#include <deque>
#include <sstream>

using MEDCoupling::MEDFileData;
using MEDCoupling::MEDFileMesh;
using MEDCoupling::MEDFileCMesh;
using MEDCoupling::MEDFileUMesh;
using MEDCoupling::MEDFileFields;
using MEDCoupling::MEDFileMeshes;

using MEDCoupling::MEDFileIntField1TS;
using MEDCoupling::MEDFileField1TS;
using MEDCoupling::MEDFileIntFieldMultiTS;
using MEDCoupling::MEDFileFieldMultiTS;
using MEDCoupling::MEDFileAnyTypeFieldMultiTS;
using MEDCoupling::DataArray;
using MEDCoupling::DataArrayInt;
using MEDCoupling::DataArrayDouble;
using MEDCoupling::MEDCouplingMesh;
using MEDCoupling::MEDCouplingUMesh;
using MEDCoupling::MEDCouplingCMesh;
using MEDCoupling::MEDCouplingFieldDouble;
using MEDCoupling::MEDCouplingFieldInt;
using MEDCoupling::MCAuto;

vtkStandardNewMacro(vtkMEDWriter);

///////////////////

class MZCException : public std::exception
{
public:
  MZCException(const std::string& s):_reason(s) { }
  virtual const char *what() const throw() { return _reason.c_str(); }
  virtual ~MZCException() throw() { }
private:
  std::string _reason;
};

///////////////////

std::map<int,int> ComputeMapOfType()
{
  std::map<int,int> ret;
  int nbOfTypesInMC(sizeof(MEDCouplingUMesh::MEDCOUPLING2VTKTYPETRADUCER)/sizeof(int));
  for(int i=0;i<nbOfTypesInMC;i++)
    {
      int vtkId(MEDCouplingUMesh::MEDCOUPLING2VTKTYPETRADUCER[i]);
      if(vtkId!=-1)
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

DataArrayInt *ConvertVTKArrayToMCArrayInt(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArrayInt : internal error !");
  int nbTuples(data->GetNumberOfTuples()),nbComp(data->GetNumberOfComponents());
  std::size_t nbElts(nbTuples*nbComp);
  MCAuto<DataArrayInt> ret(DataArrayInt::New());
  ret->alloc(nbTuples,nbComp);
  for(int i=0;i<nbComp;i++)
    {
      const char *comp(data->GetComponentName(i));
      if(comp)
        ret->setInfoOnComponent(i,comp);
    }
  int *ptOut(ret->getPointer());
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
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArrayInt : unrecognized array \"" << typeid(*data).name() << "\" type !";
  throw MZCException(oss.str());
}

DataArrayDouble *ConvertVTKArrayToMCArrayDouble(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArrayDouble : internal error !");
  int nbTuples(data->GetNumberOfTuples()),nbComp(data->GetNumberOfComponents());
  std::size_t nbElts(nbTuples*nbComp);
  MCAuto<DataArrayDouble> ret(DataArrayDouble::New());
  ret->alloc(nbTuples,nbComp);
  for(int i=0;i<nbComp;i++)
    {
      const char *comp(data->GetComponentName(i));
      if(comp)
        ret->setInfoOnComponent(i,comp);
    }
  double *ptOut(ret->getPointer());
  vtkFloatArray *d0(vtkFloatArray::SafeDownCast(data));
  if(d0)
    {
      const float *pt(d0->GetPointer(0));
      for(std::size_t i=0;i<nbElts;i++)
        ptOut[i]=pt[i];
      return ret.retn();
    }
  vtkDoubleArray *d1(vtkDoubleArray::SafeDownCast(data));
  if(d1)
    {
      const double *pt(d1->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArrayDouble : unrecognized array \"" << typeid(*data).name() << "\" type !";
  throw MZCException(oss.str());
}

DataArray *ConvertVTKArrayToMCArray(vtkDataArray *data)
{
  if(!data)
    throw MZCException("ConvertVTKArrayToMCArray : internal error !");
  vtkFloatArray *d0(vtkFloatArray::SafeDownCast(data));
  vtkDoubleArray *d1(vtkDoubleArray::SafeDownCast(data));
  if(d0 || d1)
    return ConvertVTKArrayToMCArrayDouble(data);
  vtkIntArray *d2(vtkIntArray::SafeDownCast(data));
  vtkLongArray *d3(vtkLongArray::SafeDownCast(data));
  if(d2 || d3)
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
  vtkIdType nbEntries(ca->GetNumberOfConnectivityEntries());
  const vtkIdType *conn(ca->GetPointer());
  for(int i=0;i<nbCells;i++)
    {
      int sz(*conn++);
      std::vector<int> conn2(sz);
      for(int jj=0;jj<sz;jj++)
        conn2[jj]=conn[jj];
      subMesh->insertNextCell(type,sz,&conn2[0]);
      conn+=sz;
    }
  return subMesh.retn();
}

MEDCouplingUMesh *BuildMeshFromCellArrayTriangleStrip(vtkCellArray *ca, DataArrayDouble *coords, MCAuto<DataArrayInt>& ids)
{
  MCAuto<MEDCouplingUMesh> subMesh(MEDCouplingUMesh::New("",2));
  subMesh->setCoords(coords); subMesh->allocateCells();
  int nbCells(ca->GetNumberOfCells());
  if(nbCells==0)
    return 0;
  vtkIdType nbEntries(ca->GetNumberOfConnectivityEntries());
  const vtkIdType *conn(ca->GetPointer());
  ids=DataArrayInt::New() ; ids->alloc(0,1);
  for(int i=0;i<nbCells;i++)
    {
      int sz(*conn++);
      int nbTri(sz-2);
      if(nbTri>0)
        {
          for(int j=0;j<nbTri;j++,conn++)
            {
              int conn2[3]; conn2[0]=conn[0] ; conn2[1]=conn[1] ; conn2[2]=conn[2];
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
        nbElts=arrs2[ii].size();
      else
        if(arrs2[ii].size()!=nbElts)
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

void AppendMCFieldFrom(MEDCoupling::TypeOfField tf, MEDCouplingMesh *mesh, MEDFileData *mfd, MCAuto<DataArray> da, const DataArrayInt *n2oPtr)
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
  DataArrayDouble *dadPtr(dad);
  std::string fieldName;
  if(dadPtr)
    {
      fieldName=dadPtr->getName();
      MCAuto<MEDCouplingFieldDouble> f(MEDCouplingFieldDouble::New(tf));
      f->setName(fieldName);
      if(!n2oPtr)
        f->setArray(dadPtr);
      else
        {
          MCAuto<DataArrayDouble> dad2(dadPtr->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
          f->setArray(dad2);
        }
      f->setMesh(mesh);
      MCAuto<MEDFileFieldMultiTS> fmts(MEDFileFieldMultiTS::New());
      MCAuto<MEDFileField1TS> f1ts(MEDFileField1TS::New());
      f1ts->setFieldNoProfileSBT(f);
      fmts->pushBackTimeStep(f1ts);
      fs->pushField(fmts);
      return ;
    }
  MCAuto<DataArrayInt> dai(MEDCoupling::DynamicCast<DataArray,DataArrayInt>(da));
  DataArrayInt *daiPtr(dai);
  if(daiPtr)
    {
      fieldName=daiPtr->getName();
      if((fieldName!=FAMFIELD_FOR_CELLS || tf!=MEDCoupling::ON_CELLS) && (fieldName!=FAMFIELD_FOR_NODES || tf!=MEDCoupling::ON_NODES))
        {
          MCAuto<MEDCouplingFieldInt> f(MEDCouplingFieldInt::New(tf));
          f->setName(fieldName);
          f->setMesh(mesh);
          MCAuto<MEDFileIntFieldMultiTS> fmts(MEDFileIntFieldMultiTS::New());
          MCAuto<MEDFileIntField1TS> f1ts(MEDFileIntField1TS::New());
          if(!n2oPtr)
            {
              f->setArray(dai);
              f1ts->setFieldNoProfileSBT(f);
            }
          else
            {
              MCAuto<DataArrayInt> dai2(daiPtr->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
              f->setArray(dai2);
              f1ts->setFieldNoProfileSBT(f);
            }
          fmts->pushBackTimeStep(f1ts);
          fs->pushField(fmts);
          return ;
        }
      else if(fieldName==FAMFIELD_FOR_CELLS && tf==MEDCoupling::ON_CELLS)
        {
          MEDFileMesh *mm(ms->getMeshWithName(mesh->getName()));
          if(!mm)
            throw MZCException("AppendMCFieldFrom : internal error 3 !");
          if(!n2oPtr)
            mm->setFamilyFieldArr(mesh->getMeshDimension()-mm->getMeshDimension(),daiPtr);
          else
            {
              MCAuto<DataArrayInt> dai2(daiPtr->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
              mm->setFamilyFieldArr(mesh->getMeshDimension()-mm->getMeshDimension(),dai2);
            }
        }
      else if(fieldName==FAMFIELD_FOR_NODES || tf==MEDCoupling::ON_NODES)
        {
          MEDFileMesh *mm(ms->getMeshWithName(mesh->getName()));
          if(!mm)
            throw MZCException("AppendMCFieldFrom : internal error 4 !");
          if(!n2oPtr)
            mm->setFamilyFieldArr(1,daiPtr);
          else
            {
              MCAuto<DataArrayInt> dai2(daiPtr->selectByTupleId(n2oPtr->begin(),n2oPtr->end()));
              mm->setFamilyFieldArr(1,dai2);
            }
        }
    }
}

void PutAtLevelDealOrder(MEDFileData *mfd, int meshDimRel, const MicroField& mf)
{
  if(!mfd)
    throw MZCException("PutAtLevelDealOrder : internal error !");
  MEDFileMesh *mm(mfd->getMeshes()->getMeshAtPos(0));
  MEDFileUMesh *mmu(dynamic_cast<MEDFileUMesh *>(mm));
  if(!mmu)
    throw MZCException("PutAtLevelDealOrder : internal error 2 !");
  MCAuto<MEDCouplingUMesh> mesh(mf.getMesh());
  mesh->setName(mfd->getMeshes()->getMeshAtPos(0)->getName());
  MCAuto<DataArrayInt> o2n(mesh->sortCellsInMEDFileFrmt());
  const DataArrayInt *o2nPtr(o2n);
  MCAuto<DataArrayInt> n2o;
  mmu->setMeshAtLevel(meshDimRel,mesh);
  const DataArrayInt *n2oPtr(0);
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
      AppendMCFieldFrom(MEDCoupling::ON_CELLS,mesh,mfd,da,n2oPtr);
    }
}

void AssignSingleGTMeshes(MEDFileData *mfd, const std::vector< MicroField >& ms)
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
          PutAtLevelDealOrder(mfd,(*it).first-meshDim,vs[0]);
        }
      else
        {
          MicroField merge(vs);
          PutAtLevelDealOrder(mfd,(*it).first-meshDim,merge);
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
  MCAuto<DataArrayDouble> coords(ConvertVTKArrayToMCArrayDouble(data));
  return coords.retn();
}

void AddNodeFields(MEDFileData *mfd, vtkDataSetAttributes *dsa)
{
  if(!mfd || !dsa)
    throw MZCException("AddNodeFields : internal error !");
  MEDFileMesh *mm(mfd->getMeshes()->getMeshAtPos(0));
  MEDFileUMesh *mmu(dynamic_cast<MEDFileUMesh *>(mm));
  if(!mmu)
    throw MZCException("AddNodeFields : internal error 2 !");
  MCAuto<MEDCouplingUMesh> mesh(mmu->getMeshAtLevel(0));
  int nba(dsa->GetNumberOfArrays());
  for(int i=0;i<nba;i++)
    {
      vtkDataArray *arr(dsa->GetArray(i));
      const char *name(arr->GetName());
      if(!arr)
        continue;
      MCAuto<DataArray> da(ConvertVTKArrayToMCArray(arr));
      da->setName(name);
      AppendMCFieldFrom(MEDCoupling::ON_NODES,mesh,mfd,da,0);
    }
}

std::vector<MCAuto<DataArray> > AddPartFields(const DataArrayInt *part, vtkDataSetAttributes *dsa)
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
      int nbCompo(arr->GetNumberOfComponents());
      vtkIdType nbTuples(arr->GetNumberOfTuples());
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
      int nbCompo(arr->GetNumberOfComponents());
      vtkIdType nbTuples(arr->GetNumberOfTuples());
      MCAuto<DataArray> mcarr(ConvertVTKArrayToMCArray(arr));
      mcarr=mcarr->selectByTupleIdSafeSlice(bg,end,1);
      mcarr->setName(name);
      ret.push_back(mcarr);
    }
  return ret;
}

void ConvertFromRectilinearGrid(MEDFileData *ret, vtkRectilinearGrid *ds, const std::vector<int>& context)
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
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDouble(cx));
      cmeshmc->setCoordsAt(0,arr);
    }
  if(cy)
    {
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDouble(cy));
      cmeshmc->setCoordsAt(1,arr);
    }
  if(cz)
    {
      MCAuto<DataArrayDouble> arr(ConvertVTKArrayToMCArrayDouble(cz));
      cmeshmc->setCoordsAt(2,arr);
    }
  std::string meshName(GetMeshNameWithContext(context));
  cmeshmc->setName(meshName);
  cmesh->setMesh(cmeshmc);
  std::vector<MCAuto<DataArray> > cellFs(AddPartFields(0,ds->GetCellData()));
  for(std::vector<MCAuto<DataArray> >::const_iterator it=cellFs.begin();it!=cellFs.end();it++)
    {
      MCAuto<DataArray> da(*it);
      AppendMCFieldFrom(MEDCoupling::ON_CELLS,cmeshmc,ret,da,0);
    }
  std::vector<MCAuto<DataArray> > nodeFs(AddPartFields(0,ds->GetPointData()));
  for(std::vector<MCAuto<DataArray> >::const_iterator it=nodeFs.begin();it!=nodeFs.end();it++)
    {
      MCAuto<DataArray> da(*it);
      AppendMCFieldFrom(MEDCoupling::ON_NODES,cmeshmc,ret,da,0);
    }
}

void ConvertFromPolyData(MEDFileData *ret, vtkPolyData *ds, const std::vector<int>& context)
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
      MCAuto<MEDCouplingUMesh> subMesh(BuildMeshFromCellArray(cc,coords,1,INTERP_KERNEL::NORM_SEG2));
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
      MCAuto<DataArrayInt> ids;
      MCAuto<MEDCouplingUMesh> subMesh(BuildMeshFromCellArrayTriangleStrip(ca,coords,ids));
      if((const MEDCouplingUMesh *)subMesh)
        {
          std::vector<MCAuto<DataArray> > cellFs(AddPartFields(ids,ds->GetCellData()));
          offset+=subMesh->getNumberOfCells();
          ms.push_back(MicroField(subMesh,cellFs));
        }
    }
  AssignSingleGTMeshes(ret,ms);
  AddNodeFields(ret,ds->GetPointData());
}

void ConvertFromUnstructuredGrid(MEDFileData *ret, vtkUnstructuredGrid *ds, const std::vector<int>& context)
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
  vtkIdType nbEnt(ca->GetNumberOfConnectivityEntries());
  vtkIdType *caPtr(ca->GetPointer());
  vtkUnsignedCharArray *ct(ds->GetCellTypesArray());
  if(!ct)
    throw MZCException("ConvertFromUnstructuredGrid : internal error");
  vtkIdTypeArray *cla(ds->GetCellLocationsArray());
  const vtkIdType *claPtr(cla->GetPointer(0));
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
          std::ostringstream oss; oss << "ConvertFromUnstructuredGrid : at pos #" << i << " unrecognized VTK cell with type =" << ctPtr[i];
          throw MZCException(oss.str());
        }
    }
  int dummy(0);
  int meshDim(lev->getMaxValue(dummy));
  MCAuto<DataArrayInt> levs(lev->getDifferentValues());
  std::vector< MicroField > ms;
  vtkIdTypeArray *faces(ds->GetFaces()),*faceLoc(ds->GetFaceLocations());
  for(const int *curLev=levs->begin();curLev!=levs->end();curLev++)
    {
      MCAuto<MEDCouplingUMesh> m0(MEDCouplingUMesh::New("",*curLev));
      m0->setCoords(coords); m0->allocateCells();
      MCAuto<DataArrayInt> cellIdsCurLev(lev->findIdsEqual(*curLev));
      for(const int *cellId=cellIdsCurLev->begin();cellId!=cellIdsCurLev->end();cellId++)
        {
          std::map<int,int>::iterator it(m.find(ctPtr[*cellId]));
          vtkIdType offset(claPtr[*cellId]);
          vtkIdType sz(caPtr[offset]);
          INTERP_KERNEL::NormalizedCellType ct((INTERP_KERNEL::NormalizedCellType)(*it).second);
          if(ct!=INTERP_KERNEL::NORM_POLYHED)
            {
              std::vector<int> conn2(sz);
              for(int kk=0;kk<sz;kk++)
                conn2[kk]=caPtr[offset+1+kk];
              m0->insertNextCell(ct,sz,&conn2[0]);
            }
          else
            {
              if(!faces || !faceLoc)
                throw MZCException("ConvertFromUnstructuredGrid : faces are expected when there are polyhedra !");
              const vtkIdType *facPtr(faces->GetPointer(0)),*facLocPtr(faceLoc->GetPointer(0));
              std::vector<int> conn;
              int off0(facLocPtr[*cellId]);
              int nbOfFaces(facPtr[off0++]);
              for(int k=0;k<nbOfFaces;k++)
                {
                  int nbOfNodesInFace(facPtr[off0++]);
                  std::copy(facPtr+off0,facPtr+off0+nbOfNodesInFace,std::back_inserter(conn));
                  off0+=nbOfNodesInFace;
                  if(k<nbOfFaces-1)
                    conn.push_back(-1);
                }
              m0->insertNextCell(ct,conn.size(),&conn[0]);
            }
        }
      std::vector<MCAuto<DataArray> > cellFs(AddPartFields(cellIdsCurLev,ds->GetCellData()));
      ms.push_back(MicroField(m0,cellFs));
    }
  AssignSingleGTMeshes(ret,ms);
  AddNodeFields(ret,ds->GetPointData());
}

///////////////////

vtkMEDWriter::vtkMEDWriter():WriteAllTimeSteps(0),FileName(0),IsTouched(false)
{
}

vtkMEDWriter::~vtkMEDWriter()
{
}

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

class Grp
{
public:
  Grp(const std::string& name):_name(name) { }
  void setFamilies(const std::vector<std::string>& fams) { _fams=fams; }
  std::string getName() const { return _name; }
  std::vector<std::string> getFamilies() const { return _fams; }
private:
  std::string _name;
  std::vector<std::string> _fams;
};

class Fam
{
public:
  Fam(const std::string& name)
  {
    static const char ZE_SEP[]="@@][@@";
    std::size_t pos(name.find(ZE_SEP));
    std::string name0(name.substr(0,pos)),name1(name.substr(pos+strlen(ZE_SEP)));
    std::istringstream iss(name1);
    iss >> _id;
    _name=name0;
  }
  std::string getName() const { return _name; }
  int getID() const { return _id; }
private:
  std::string _name;
  int _id;
};

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

int vtkMEDWriter::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{ 
  //std::cerr << "########################################## vtkMEDWriter::RequestInformation ########################################## " << (const char *) this->FileName << std::endl;
  return 1;
}

void WriteMEDFileFromVTKDataSet(MEDFileData *mfd, vtkDataSet *ds, const std::vector<int>& context)
{
  if(!ds || !mfd)
    throw MZCException("Internal error in WriteMEDFileFromVTKDataSet.");
  vtkPolyData *ds2(vtkPolyData::SafeDownCast(ds));
  vtkUnstructuredGrid *ds3(vtkUnstructuredGrid::SafeDownCast(ds));
  vtkRectilinearGrid *ds4(vtkRectilinearGrid::SafeDownCast(ds));
  if(ds2)
    {
      ConvertFromPolyData(mfd,ds2,context);
    }
  else if(ds3)
    {
      ConvertFromUnstructuredGrid(mfd,ds3,context);
    }
  else if(ds4)
    {
      ConvertFromRectilinearGrid(mfd,ds4,context);
    }
  else
    throw MZCException("Unrecognized vtkDataSet ! Sorry ! Try to convert it to UnstructuredGrid to be able to write it !");
}

void WriteMEDFileFromVTKMultiBlock(MEDFileData *mfd, vtkMultiBlockDataSet *ds, const std::vector<int>& context)
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
          WriteMEDFileFromVTKDataSet(mfd,uniqueEltc,context);
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
          WriteMEDFileFromVTKDataSet(mfd,elt1,context);
          continue;
        }
      vtkMultiBlockDataSet *elt2(vtkMultiBlockDataSet::SafeDownCast(elt));
      if(elt2)
        {
          WriteMEDFileFromVTKMultiBlock(mfd,elt2,context);
          continue;
        }
      std::ostringstream oss; oss << "In context ";
      std::copy(context.begin(),context.end(),std::ostream_iterator<int>(oss," "));
      oss << " at pos #" << i << " elt not recognized data type !";
      throw MZCException(oss.str());
    }
}

void WriteMEDFileFromVTKGDS(MEDFileData *mfd, vtkDataObject *input)
{
  if(!input || !mfd)
    throw MZCException("WriteMEDFileFromVTKGDS : internal error !");
  std::vector<int> context;
  vtkDataSet *input1(vtkDataSet::SafeDownCast(input));
  if(input1)
    {
      WriteMEDFileFromVTKDataSet(mfd,input1,context);
      return ;
    }
  vtkMultiBlockDataSet *input2(vtkMultiBlockDataSet::SafeDownCast(input));
  if(input2)
    {
      WriteMEDFileFromVTKMultiBlock(mfd,input2,context);
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

int vtkMEDWriter::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkMEDWriter::RequestData        ########################################## " << (const char *) this->FileName << std::endl;
  try
    {
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
      MCAuto<MEDFileData> mfd(MEDFileData::New());
      WriteMEDFileFromVTKGDS(mfd,input);
      PutFamGrpInfoIfAny(mfd,meshName,groups,fams);
      mfd->write(this->FileName,this->IsTouched?0:2); this->IsTouched=true;
      outInfo->Set(vtkDataObject::DATA_OBJECT(),input);
    }
  catch(MZCException& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkMEDWriter::RequestData : During writing of \"" << (const char *) this->FileName << "\", the following exception has been thrown : "<< e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
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
