// Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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

#include "vtkVoroGauss.h"

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
#include "MEDCouplingMemArray.txx"
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

vtkStandardNewMacro(vtkVoroGauss)
///////////////////

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

std::map<int,int> ComputeRevMapOfType()
{
  std::map<int,int> ret;
  int nbOfTypesInMC(sizeof(MEDCOUPLING2VTKTYPETRADUCER)/sizeof( decltype(MEDCOUPLING2VTKTYPETRADUCER[0]) ));
  for(int i=0;i<nbOfTypesInMC;i++)
    {
      auto vtkId(MEDCOUPLING2VTKTYPETRADUCER[i]);
      if(vtkId!=MEDCOUPLING2VTKTYPETRADUCER_NONE)
        ret[i]=vtkId;
    }
  return ret;
}

///////////////////

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

DataArrayIdType *ConvertVTKArrayToMCArrayInt(vtkDataArray *data)
{
  if(!data)
    throw INTERP_KERNEL::Exception("ConvertVTKArrayToMCArrayInt : internal error !");
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
  vtkIdTypeArray *d2(vtkIdTypeArray::SafeDownCast(data));
  if(d2)
    {
      const vtkIdType *pt(d2->GetPointer(0));
      std::copy(pt,pt+nbElts,ptOut);
      return ret.retn();
    }
  std::ostringstream oss;
  oss << "ConvertVTKArrayToMCArrayInt : unrecognized array \"" << typeid(*data).name() << "\" type !";
  throw INTERP_KERNEL::Exception(oss.str());
}

DataArrayDouble *ConvertVTKArrayToMCArrayDouble(vtkDataArray *data)
{
  if(!data)
    throw INTERP_KERNEL::Exception("ConvertVTKArrayToMCArrayDouble : internal error !");
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
  throw INTERP_KERNEL::Exception(oss.str());
}

DataArray *ConvertVTKArrayToMCArray(vtkDataArray *data)
{
  if(!data)
    throw INTERP_KERNEL::Exception("ConvertVTKArrayToMCArray : internal error !");
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
  throw INTERP_KERNEL::Exception(oss.str());
}

DataArrayDouble *BuildCoordsFrom(vtkPointSet *ds)
{
  if(!ds)
    throw INTERP_KERNEL::Exception("BuildCoordsFrom : internal error !");
  vtkPoints *pts(ds->GetPoints());
  if(!pts)
    throw INTERP_KERNEL::Exception("BuildCoordsFrom : internal error 2 !");
  vtkDataArray *data(pts->GetData());
  if(!data)
    throw INTERP_KERNEL::Exception("BuildCoordsFrom : internal error 3 !");
  MCAuto<DataArrayDouble> coords(ConvertVTKArrayToMCArrayDouble(data));
  return coords.retn();
}

void ConvertFromUnstructuredGrid(vtkUnstructuredGrid *ds, std::vector< MCAuto<MEDCouplingUMesh> >& ms, std::vector< MCAuto<DataArrayIdType> >& ids)
{
  MCAuto<DataArrayDouble> coords(BuildCoordsFrom(ds));
  vtkIdType nbCells(ds->GetNumberOfCells());
  vtkCellArray *ca(ds->GetCells());
  if(!ca)
    return ;
  //vtkIdType nbEnt(ca->GetNumberOfConnectivityEntries()); // todo: unused
  //vtkIdType *caPtr(ca->GetData()->GetPointer(0)); // todo: unused
  vtkUnsignedCharArray *ct(ds->GetCellTypesArray());
  if(!ct)
    throw INTERP_KERNEL::Exception("ConvertFromUnstructuredGrid : internal error");
  vtkIdTypeArray *cla(ds->GetCellLocationsArray());
  //const vtkIdType *claPtr(cla->GetPointer(0)); // todo: unused
  if(!cla)
    throw INTERP_KERNEL::Exception("ConvertFromUnstructuredGrid : internal error 2");
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
          throw INTERP_KERNEL::Exception(oss.str());
        }
    }
  MCAuto<DataArrayInt> levs(lev->getDifferentValues());
  //vtkIdTypeArray *faces(ds->GetFaces()),*faceLoc(ds->GetFaceLocations()); // todo: unused
  for(const int *curLev=levs->begin();curLev!=levs->end();curLev++)
    {
      MCAuto<MEDCouplingUMesh> m0(MEDCouplingUMesh::New("",*curLev));
      m0->setCoords(coords); m0->allocateCells();
      MCAuto<DataArrayIdType> cellIdsCurLev(lev->findIdsEqual(*curLev));
      for(const mcIdType *cellId=cellIdsCurLev->begin();cellId!=cellIdsCurLev->end();cellId++)
        {
          std::map<int,int>::iterator it(m.find(ds->GetCellType(*cellId)));
          INTERP_KERNEL::NormalizedCellType ct((INTERP_KERNEL::NormalizedCellType)(*it).second);
          if(ct!=INTERP_KERNEL::NORM_POLYHED)
            {
              vtkIdType szzz(0);
              const vtkIdType *ptszz(nullptr);
              ds->GetCellPoints(*cellId, szzz, ptszz);
              std::vector<mcIdType> conn(ptszz,ptszz+szzz);
              m0->insertNextCell(ct,szzz,conn.data());
            }
          else
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
        }
      ms.push_back(m0); ids.push_back(cellIdsCurLev);
    }
}

vtkSmartPointer<vtkUnstructuredGrid> ConvertUMeshFromMCToVTK(const MEDCouplingUMesh *mVor)
{
  std::map<int,int> zeMapRev(ComputeRevMapOfType());
  int nbCells(mVor->getNumberOfCells());
  vtkSmartPointer<vtkUnstructuredGrid> ret(vtkSmartPointer<vtkUnstructuredGrid>::New());
  ret->Initialize();
  ret->Allocate();
  vtkSmartPointer<vtkPoints> points(vtkSmartPointer<vtkPoints>::New());
  {
    const DataArrayDouble *vorCoords(mVor->getCoords());
    vtkSmartPointer<vtkDoubleArray> da(vtkSmartPointer<vtkDoubleArray>::New());
    da->SetNumberOfComponents((vtkIdType)vorCoords->getNumberOfComponents());
    da->SetNumberOfTuples((vtkIdType)vorCoords->getNumberOfTuples());
    std::copy(vorCoords->begin(),vorCoords->end(),da->GetPointer(0));
    points->SetData(da);
  }
  mVor->checkConsistencyLight();
  switch(mVor->getMeshDimension())
    {
    case 3:
      {
        vtkIdType *cPtr(nullptr),*dPtr(nullptr);
        unsigned char *aPtr(nullptr);
        vtkSmartPointer<vtkUnsignedCharArray> cellTypes(vtkSmartPointer<vtkUnsignedCharArray>::New());
        {
          cellTypes->SetNumberOfComponents(1);
          cellTypes->SetNumberOfTuples(nbCells);
          aPtr=cellTypes->GetPointer(0);
        }
        vtkSmartPointer<vtkIdTypeArray> cellLocations(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          cellLocations->SetNumberOfComponents(1);
          cellLocations->SetNumberOfTuples(nbCells);
          cPtr=cellLocations->GetPointer(0);
        }
        vtkSmartPointer<vtkIdTypeArray> cells(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          MCAuto<DataArrayIdType> tmp2(mVor->computeEffectiveNbOfNodesPerCell());
          cells->SetNumberOfComponents(1);
          cells->SetNumberOfTuples(tmp2->accumulate((std::size_t)0)+nbCells);
          dPtr=cells->GetPointer(0);
        }
        const mcIdType *connPtr(mVor->getNodalConnectivity()->begin()),*connIPtr(mVor->getNodalConnectivityIndex()->begin());
        int k(0),kk(0);
        std::vector<vtkIdType> ee,ff;
        for(int i=0;i<nbCells;i++,connIPtr++)
          {
            INTERP_KERNEL::NormalizedCellType ct(static_cast<INTERP_KERNEL::NormalizedCellType>(connPtr[connIPtr[0]]));
            *aPtr++=(unsigned char)zeMapRev[connPtr[connIPtr[0]]];
            if(ct!=INTERP_KERNEL::NORM_POLYHED)
              {
                int sz(connIPtr[1]-connIPtr[0]-1);
                *dPtr++=sz;
                dPtr=std::copy(connPtr+connIPtr[0]+1,connPtr+connIPtr[1],dPtr);
                *cPtr++=k; k+=sz+1;
                ee.push_back(kk);
              }
            else
              {
                std::set<int> s(connPtr+connIPtr[0]+1,connPtr+connIPtr[1]); s.erase(-1);
                vtkIdType nbFace((vtkIdType)(std::count(connPtr+connIPtr[0]+1,connPtr+connIPtr[1],-1)+1));
                ff.push_back(nbFace);
                const mcIdType *work(connPtr+connIPtr[0]+1);
                for(int j=0;j<nbFace;j++)
                  {
                    const mcIdType *work2=std::find(work,connPtr+connIPtr[1],-1);
                    ff.push_back((vtkIdType)std::distance(work,work2));
                    ff.insert(ff.end(),work,work2);
                    work=work2+1;
                  }
                *dPtr++=(int)s.size();
                dPtr=std::copy(s.begin(),s.end(),dPtr);
                *cPtr++=k; k+=(int)s.size()+1;
                ee.push_back(kk); kk+=connIPtr[1]-connIPtr[0]+1;
              }
          }
        //
        vtkSmartPointer<vtkIdTypeArray> faceLocations(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          faceLocations->SetNumberOfComponents(1);
          faceLocations->SetNumberOfTuples((vtkIdType)ee.size());
          std::copy(ee.begin(),ee.end(),faceLocations->GetPointer(0));
        }
        vtkSmartPointer<vtkIdTypeArray> faces(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          faces->SetNumberOfComponents(1);
          faces->SetNumberOfTuples((vtkIdType)ff.size());
          std::copy(ff.begin(),ff.end(),faces->GetPointer(0));
        }
        vtkSmartPointer<vtkCellArray> cells2(vtkSmartPointer<vtkCellArray>::New());
        cells2->SetCells(nbCells,cells);
        ret->SetCells(cellTypes,cellLocations,cells2,faceLocations,faces);
        break;
      }
    case 2:
      {
        vtkSmartPointer<vtkUnsignedCharArray> cellTypes(vtkSmartPointer<vtkUnsignedCharArray>::New());
        {
          cellTypes->SetNumberOfComponents(1);
          cellTypes->SetNumberOfTuples(nbCells);
          unsigned char *ptr(cellTypes->GetPointer(0));
          std::fill(ptr,ptr+nbCells,zeMapRev[(int)INTERP_KERNEL::NORM_POLYGON]);
        }
        vtkIdType *cPtr(nullptr),*dPtr(nullptr);
        vtkSmartPointer<vtkIdTypeArray> cellLocations(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          cellLocations->SetNumberOfComponents(1);
          cellLocations->SetNumberOfTuples(nbCells);
          cPtr=cellLocations->GetPointer(0);
        }
        vtkSmartPointer<vtkIdTypeArray> cells(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          cells->SetNumberOfComponents(1);
          cells->SetNumberOfTuples(mVor->getNodalConnectivity()->getNumberOfTuples());
          dPtr=cells->GetPointer(0);
        }
        const mcIdType *connPtr(mVor->getNodalConnectivity()->begin()),*connIPtr(mVor->getNodalConnectivityIndex()->begin());
        int k(0);
        for(int i=0;i<nbCells;i++,connIPtr++)
          {
            *dPtr++=connIPtr[1]-connIPtr[0]-1;
            dPtr=std::copy(connPtr+connIPtr[0]+1,connPtr+connIPtr[1],dPtr);
            *cPtr++=k; k+=connIPtr[1]-connIPtr[0];
          }
        vtkSmartPointer<vtkCellArray> cells2(vtkSmartPointer<vtkCellArray>::New());
        cells2->SetCells(nbCells,cells);
        ret->SetCells(cellTypes,cellLocations,cells2);
        break;
      }
    case 1:
      {
        vtkSmartPointer<vtkUnsignedCharArray> cellTypes(vtkSmartPointer<vtkUnsignedCharArray>::New());
        {
          cellTypes->SetNumberOfComponents(1);
          cellTypes->SetNumberOfTuples(nbCells);
          unsigned char *ptr(cellTypes->GetPointer(0));
          std::fill(ptr,ptr+nbCells,zeMapRev[(int)INTERP_KERNEL::NORM_SEG2]);
        }
        vtkIdType *cPtr(nullptr),*dPtr(nullptr);
        vtkSmartPointer<vtkIdTypeArray> cellLocations(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          cellLocations->SetNumberOfComponents(1);
          cellLocations->SetNumberOfTuples(nbCells);
          cPtr=cellLocations->GetPointer(0);
        }
        vtkSmartPointer<vtkIdTypeArray> cells(vtkSmartPointer<vtkIdTypeArray>::New());
        {
          cells->SetNumberOfComponents(1);
          cells->SetNumberOfTuples(mVor->getNodalConnectivity()->getNumberOfTuples());
          dPtr=cells->GetPointer(0);
        }
        const mcIdType *connPtr(mVor->getNodalConnectivity()->begin()),*connIPtr(mVor->getNodalConnectivityIndex()->begin());
        for(int i=0;i<nbCells;i++,connIPtr++)
          {
            *dPtr++=2;
            dPtr=std::copy(connPtr+connIPtr[0]+1,connPtr+connIPtr[1],dPtr);
            *cPtr++=3*i;
          }
        vtkSmartPointer<vtkCellArray> cells2(vtkSmartPointer<vtkCellArray>::New());
        cells2->SetCells(nbCells,cells);
        ret->SetCells(cellTypes,cellLocations,cells2);
        break;
      }
    default:
      throw INTERP_KERNEL::Exception("Not implemented yet !");
    }
  ret->SetPoints(points);
  return ret;
}

class OffsetKeeper
{
public:
  OffsetKeeper():_vtk_arr(0) { }
  void pushBack(vtkDataArray *da) { _da_on.push_back(da); }
  void setVTKArray(vtkIdTypeArray *arr) { 
    MCAuto<DataArrayIdType> offmc(ConvertVTKArrayToMCArrayInt(arr));
    _off_arr=offmc; _vtk_arr=arr; }
  const std::vector<vtkDataArray *>& getArrayGauss() const { return _da_on; }
  const DataArrayIdType *getOffsets() const { return _off_arr; }
  vtkIdTypeArray *getVTKOffsets() const { return _vtk_arr; }
private:
  std::vector<vtkDataArray *> _da_on;
  MCAuto<DataArrayIdType> _off_arr;
  vtkIdTypeArray *_vtk_arr;
};

void FillAdvInfoFrom(int vtkCT, const std::vector<double>& GaussAdvData, int nbGaussPt, int nbNodesPerCell, std::vector<double>& refCoo,std::vector<double>& posInRefCoo)
{
  int nbOfCTS((int)GaussAdvData[0]),pos(1);
  for(int i=0;i<nbOfCTS;i++)
    {
      int lgth((int)GaussAdvData[pos]);
      int curCT((int)GaussAdvData[pos+1]),dim((int)GaussAdvData[pos+2]);
      if(curCT!=vtkCT)
        {
          pos+=lgth+1;
          continue;
        }
      int lgthExp(nbNodesPerCell*dim+nbGaussPt*dim);
      if(lgth!=lgthExp+2)//+2 for cell type and dimension !
        {
          std::ostringstream oss; oss << "FillAdvInfoFrom : Internal error. Unmatch with MEDReader version ? Expect size " << lgthExp << " and have " << lgth << " !";
          throw INTERP_KERNEL::Exception(oss.str());
        }
      refCoo.insert(refCoo.end(),GaussAdvData.begin()+pos+3,GaussAdvData.begin()+pos+3+nbNodesPerCell*dim);
      posInRefCoo.insert(posInRefCoo.end(),GaussAdvData.begin()+pos+3+nbNodesPerCell*dim,GaussAdvData.begin()+pos+3+nbNodesPerCell*dim+nbGaussPt*dim);
      //std::copy(refCoo.begin(),refCoo.end(),std::ostream_iterator<double>(std::cerr," ")); std::cerr << std::endl;
      //std::copy(posInRefCoo.begin(),posInRefCoo.end(),std::ostream_iterator<double>(std::cerr," ")); std::cerr << std::endl;
      return ;
    }
  std::ostringstream oss; oss << "FillAdvInfoFrom : Internal error ! Not found cell type " << vtkCT << " in advanced Gauss info !";
  throw INTERP_KERNEL::Exception(oss.str());
}

template<class T, class U>
vtkSmartPointer<T> ExtractFieldFieldArr(T *elt2, int sizeOfOutArr, int nbOfCellsOfInput, const mcIdType *offsetsPtr, const mcIdType *nbPtsPerCellPtr)
{
  vtkSmartPointer<T> elt3(vtkSmartPointer<T>::New());
  int nbc(elt2->GetNumberOfComponents());
  elt3->SetNumberOfComponents(nbc);
  elt3->SetNumberOfTuples(sizeOfOutArr);
  for(int i=0;i<nbc;i++)
    {
      const char *name(elt2->GetComponentName(i));
      if(name)
        elt3->SetComponentName(i,name);
    }
  elt3->SetName(elt2->GetName());
  //
  U *ptr(elt3->GetPointer(0));
  const U *srcPtr(elt2->GetPointer(0));
  for(int i=0;i<nbOfCellsOfInput;i++)
    ptr=std::copy(srcPtr+nbc*offsetsPtr[i],srcPtr+nbc*(offsetsPtr[i]+nbPtsPerCellPtr[i]),ptr);
  return elt3;
}

template<class T, class U>
vtkSmartPointer<T> ExtractCellFieldArr(T *elt2, int sizeOfOutArr, int nbOfCellsOfInput, const mcIdType *idsPtr, const mcIdType *nbPtsPerCellPtr)
{
  vtkSmartPointer<T> elt3(vtkSmartPointer<T>::New());
  int nbc(elt2->GetNumberOfComponents());
  elt3->SetNumberOfComponents(nbc);
  elt3->SetNumberOfTuples(sizeOfOutArr);
  for(int i=0;i<nbc;i++)
    {
      const char *name(elt2->GetComponentName(i));
      if(name)
        elt3->SetComponentName(i,name);
    }
  elt3->SetName(elt2->GetName());
  //
  U *ptr(elt3->GetPointer(0));
  const U *srcPtr(elt2->GetPointer(0));
  for(int i=0;i<nbOfCellsOfInput;i++)
    for(int j=0;j<nbPtsPerCellPtr[i];j++)
      ptr=std::copy(srcPtr+nbc*idsPtr[i],srcPtr+nbc*(idsPtr[i]+1),ptr);
  return elt3;
}

vtkSmartPointer<vtkUnstructuredGrid> Voronize(const MEDCouplingUMesh *m, const DataArrayIdType *ids, vtkIdTypeArray *vtkOff, const DataArrayIdType *offsetsBase, const std::vector<vtkDataArray *>& arrGauss, const std::vector<double>& GaussAdvData, const std::vector<vtkDataArray *>& arrsOnCells)
{
  if(arrGauss.empty())
    throw INTERP_KERNEL::Exception("Voronize : no Gauss array !");
  int nbTuples(arrGauss[0]->GetNumberOfTuples());
  for(std::vector<vtkDataArray *>::const_iterator it=arrGauss.begin();it!=arrGauss.end();it++)
    {
      if((*it)->GetNumberOfTuples()!=nbTuples)
        {
          std::ostringstream oss; oss << "Mismatch of number of tuples in Gauss arrays for array \"" << (*it)->GetName() << "\"";
          throw INTERP_KERNEL::Exception(oss.str());
        }
    }
  // Look at vtkOff has in the stomac
  vtkInformation *info(vtkOff->GetInformation());
  if(!info)
    throw INTERP_KERNEL::Exception("info is null ! Internal error ! Looks bad !");
  vtkInformationQuadratureSchemeDefinitionVectorKey *key(vtkQuadratureSchemeDefinition::DICTIONARY());
  if(!key->Has(info))
    throw INTERP_KERNEL::Exception("No quadrature key in info included in offets array ! Internal error ! Looks bad !");
  int dictSize(key->Size(info));
  INTERP_KERNEL::AutoPtr<vtkQuadratureSchemeDefinition *> dict(new vtkQuadratureSchemeDefinition *[dictSize]);
  key->GetRange(info,dict,0,0,dictSize);
  // Voronoize
  MCAuto<MEDCouplingFieldDouble> field(MEDCouplingFieldDouble::New(ON_GAUSS_PT));
  field->setMesh(m);
  // Gauss Part
  int nbOfCellsOfInput(m->getNumberOfCells());
  MCAuto<DataArrayIdType> nbPtsPerCellArr(DataArrayIdType::New()); nbPtsPerCellArr->alloc(nbOfCellsOfInput,1);
  std::map<int,int> zeMapRev(ComputeRevMapOfType()),zeMap(ComputeMapOfType());
  std::set<INTERP_KERNEL::NormalizedCellType> agt(m->getAllGeoTypes());
  for(std::set<INTERP_KERNEL::NormalizedCellType>::const_iterator it=agt.begin();it!=agt.end();it++)
    {
      const INTERP_KERNEL::CellModel& cm(INTERP_KERNEL::CellModel::GetCellModel(*it));
      std::map<int,int>::const_iterator it2(zeMapRev.find((int)*it));
      if(it2==zeMapRev.end())
        throw INTERP_KERNEL::Exception("Internal error ! no type conversion available !");
      vtkQuadratureSchemeDefinition *gaussLoc(dict[(*it2).second]);
      if(!gaussLoc)
        {
          std::ostringstream oss; oss << "For cell type " << cm.getRepr() << " no Gauss info !";
          throw INTERP_KERNEL::Exception(oss.str());
        }
      int np(gaussLoc->GetNumberOfQuadraturePoints()),nbPtsPerCell((int)cm.getNumberOfNodes());
      const double /**sfw(gaussLoc->GetShapeFunctionWeights()),*/*w(gaussLoc->GetQuadratureWeights());; // todo: sfw is unused
      std::vector<double> refCoo,posInRefCoo,wCpp(w,w+np);
      FillAdvInfoFrom((*it2).second,GaussAdvData,np,nbPtsPerCell,refCoo,posInRefCoo);
      field->setGaussLocalizationOnType(*it,refCoo,posInRefCoo,wCpp);
      MCAuto<DataArrayIdType> ids2(m->giveCellsWithType(*it));
      nbPtsPerCellArr->setPartOfValuesSimple3(np,ids2->begin(),ids2->end(),0,1,1);
    }
  int zeSizeOfOutCellArr(nbPtsPerCellArr->accumulate((std::size_t)0));
  { MCAuto<DataArrayDouble> fakeArray(DataArrayDouble::New()); fakeArray->alloc(zeSizeOfOutCellArr,1); field->setArray(fakeArray); }
  field->checkConsistencyLight();
  MCAuto<MEDCouplingFieldDouble> vor(field->voronoize(1e-12));// The key is here !
  MEDCouplingUMesh *mVor(dynamic_cast<MEDCouplingUMesh *>(vor->getMesh()));
  //
  vtkSmartPointer<vtkUnstructuredGrid> ret(ConvertUMeshFromMCToVTK(mVor));
  // now fields...
  MCAuto<DataArrayIdType> myOffsets(offsetsBase->selectByTupleIdSafe(ids->begin(),ids->end()));
  const mcIdType *myOffsetsPtr(myOffsets->begin()),*nbPtsPerCellArrPtr(nbPtsPerCellArr->begin());
  for(std::vector<vtkDataArray *>::const_iterator it=arrGauss.begin();it!=arrGauss.end();it++)
    {
      vtkDataArray *elt(*it);
      vtkDoubleArray *elt2(vtkDoubleArray::SafeDownCast(elt));
      vtkIntArray *elt3(vtkIntArray::SafeDownCast(elt));
      vtkIdTypeArray *elt4(vtkIdTypeArray::SafeDownCast(elt));
      if(elt2)
        {
          vtkSmartPointer<vtkDoubleArray> arr(ExtractFieldFieldArr<vtkDoubleArray,double>(elt2,zeSizeOfOutCellArr,nbOfCellsOfInput,myOffsetsPtr,nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
      if(elt3)
        {
          vtkSmartPointer<vtkIntArray> arr(ExtractFieldFieldArr<vtkIntArray,int>(elt3,zeSizeOfOutCellArr,nbOfCellsOfInput,myOffsetsPtr,nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
      if(elt4)
        {
          vtkSmartPointer<vtkIdTypeArray> arr(ExtractFieldFieldArr<vtkIdTypeArray,vtkIdType>(elt4,zeSizeOfOutCellArr,nbOfCellsOfInput,myOffsetsPtr,nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
    }
  for(std::vector<vtkDataArray *>::const_iterator it=arrsOnCells.begin();it!=arrsOnCells.end();it++)
    {
      vtkDataArray *elt(*it);
      vtkDoubleArray *elt2(vtkDoubleArray::SafeDownCast(elt));
      vtkIntArray *elt3(vtkIntArray::SafeDownCast(elt));
      vtkIdTypeArray *elt4(vtkIdTypeArray::SafeDownCast(elt));
      if(elt2)
        {
          vtkSmartPointer<vtkDoubleArray> arr(ExtractCellFieldArr<vtkDoubleArray,double>(elt2,zeSizeOfOutCellArr,nbOfCellsOfInput,ids->begin(),nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
      if(elt3)
        {
          vtkSmartPointer<vtkIntArray> arr(ExtractCellFieldArr<vtkIntArray,int>(elt3,zeSizeOfOutCellArr,nbOfCellsOfInput,ids->begin(),nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
      if(elt4)
        {
          vtkSmartPointer<vtkIdTypeArray> arr(ExtractCellFieldArr<vtkIdTypeArray,vtkIdType>(elt4,zeSizeOfOutCellArr,nbOfCellsOfInput,ids->begin(),nbPtsPerCellArrPtr));
          ret->GetCellData()->AddArray(arr);
          continue;
        }
    }
  return ret;
}

vtkSmartPointer<vtkUnstructuredGrid> ComputeVoroGauss(vtkUnstructuredGrid *usgIn, const std::vector<double>& GaussAdvData)
{
  OffsetKeeper zeOffsets;
  std::string zeArrOffset;
  std::vector<std::string> cellFieldNamesToDestroy;
  {
    int nArrays(usgIn->GetFieldData()->GetNumberOfArrays());
    for(int i=0;i<nArrays;i++)
      {
        vtkDataArray *array(usgIn->GetFieldData()->GetArray(i));
        if(!array)
          continue;
        const char* arrayOffsetName(array->GetInformation()->Get(vtkQuadratureSchemeDefinition::QUADRATURE_OFFSET_ARRAY_NAME()));
        if(!arrayOffsetName)
          continue;
        std::string arrOffsetNameCpp(arrayOffsetName);
        cellFieldNamesToDestroy.push_back(arrOffsetNameCpp);
        if(arrOffsetNameCpp.find("ELGA@")==std::string::npos)
          continue;
        if(zeArrOffset.empty())
          zeArrOffset=arrOffsetNameCpp;
        else
          if(zeArrOffset!=arrOffsetNameCpp)
            {
              throw INTERP_KERNEL::Exception("ComputeVoroGauss : error in QUADRATURE_OFFSET_ARRAY_NAME for Gauss fields array !");
            }
        zeOffsets.pushBack(array);
      }
    if(zeArrOffset.empty())
      throw INTERP_KERNEL::Exception("ComputeVoroGauss : no Gauss points fields in DataSet !");
  }
  std::vector< MCAuto<MEDCouplingUMesh> > ms;
  std::vector< MCAuto<DataArrayIdType> > ids;
  ConvertFromUnstructuredGrid(usgIn,ms,ids);
  {
    vtkDataArray *offTmp(usgIn->GetCellData()->GetArray(zeArrOffset.c_str()));
    if(!offTmp)
      {
        std::ostringstream oss; oss << "ComputeVoroGauss : cell field " << zeArrOffset << " not found !";
        throw INTERP_KERNEL::Exception(oss.str());
      }
    vtkIdTypeArray *offsets(vtkIdTypeArray::SafeDownCast(offTmp));
    if(!offsets)
      {
        std::ostringstream oss; oss << "ComputeVoroGauss : cell field " << zeArrOffset << " exists but not with the right type of data !";
        throw INTERP_KERNEL::Exception(oss.str());
      }
    ///
    zeOffsets.setVTKArray(offsets);
  }
  //
  std::vector<vtkDataArray *> arrsOnCells;
  {
    int nArrays(usgIn->GetCellData()->GetNumberOfArrays());
    for(int i=0;i<nArrays;i++)
      {
        vtkDataArray *array(usgIn->GetCellData()->GetArray(i));
        if(!array)
          continue;
        std::string name(array->GetName());
        if(std::find(cellFieldNamesToDestroy.begin(),cellFieldNamesToDestroy.end(),name)==cellFieldNamesToDestroy.end())
          {
            arrsOnCells.push_back(array);
          }
      }
    {
      vtkDataArray *doNotKeepThis(zeOffsets.getVTKOffsets());
      std::vector<vtkDataArray *>::iterator it2(std::find(arrsOnCells.begin(),arrsOnCells.end(),doNotKeepThis));
      if(it2!=arrsOnCells.end())
        arrsOnCells.erase(it2);
    }
  }
  //
  std::size_t sz(ms.size());
  std::vector< vtkSmartPointer<vtkUnstructuredGrid> > res;
  for(std::size_t i=0;i<sz;i++)
    {
      MCAuto<MEDCouplingUMesh> mmc(ms[i]);
      MCAuto<DataArrayIdType> myIds(ids[i]);
      vtkSmartPointer<vtkUnstructuredGrid> vor(Voronize(mmc,myIds,zeOffsets.getVTKOffsets(),zeOffsets.getOffsets(),zeOffsets.getArrayGauss(),GaussAdvData,arrsOnCells));
      res.push_back(vor);
    }
  if(res.empty())
    throw INTERP_KERNEL::Exception("Dataset is empty !");
  vtkSmartPointer<vtkMultiBlockDataGroupFilter> mb(vtkSmartPointer<vtkMultiBlockDataGroupFilter>::New());
  vtkSmartPointer<vtkCompositeDataToUnstructuredGridFilter> cd(vtkSmartPointer<vtkCompositeDataToUnstructuredGridFilter>::New());
  for(std::vector< vtkSmartPointer<vtkUnstructuredGrid> >::const_iterator it=res.begin();it!=res.end();it++)
    mb->AddInputData(*it);
  cd->SetInputConnection(mb->GetOutputPort());
  cd->SetMergePoints(0);
  cd->Update();
  vtkSmartPointer<vtkUnstructuredGrid> ret;
  ret=cd->GetOutput();
  return ret;
}

////////////////////

vtkVoroGauss::vtkVoroGauss()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkVoroGauss::~vtkVoroGauss()
{
}

int vtkVoroGauss::RequestInformation(vtkInformation * /*request*/, vtkInformationVector **inputVector, vtkInformationVector * /*outputVector*/)
{ 
  //std::cerr << "########################################## vtkVoroGauss::RequestInformation ##########################################" << std::endl;
  try
    {
      vtkUnstructuredGrid *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkVoroGauss::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

int vtkVoroGauss::RequestData(vtkInformation * /*request*/, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkVoroGauss::RequestData        ##########################################" << std::endl;
  try
    {
      
      std::vector<double> GaussAdvData;
      bool isOK(IsInformationOK(inputVector[0]->GetInformationObject(0),GaussAdvData));
      if(!isOK)
        throw INTERP_KERNEL::Exception("Sorry but no advanced gauss info found ! Expect to be called right after a MEDReader containing Gauss Points !");
      vtkUnstructuredGrid *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
      //
      vtkSmartPointer<vtkUnstructuredGrid> ret(ComputeVoroGauss(usgIn,GaussAdvData));
      //vtkInformation *inInfo(inputVector[0]->GetInformationObject(0)); // todo: unused
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkUnstructuredGrid *output(vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      output->ShallowCopy(ret);
    }
  catch(INTERP_KERNEL::Exception& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkVoroGauss::RequestData : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

void vtkVoroGauss::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
