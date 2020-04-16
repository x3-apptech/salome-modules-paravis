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

#include "vtkDevelopedSurface.h"
#include "VTKToMEDMem.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkIntArray.h"
#include "vtkLongArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCylinder.h"
#include "vtkNew.h"
#include "vtkCutter.h"
#include "vtkTransform.h"

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
#include "vtkFloatArray.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkWarpScalar.h"

#include "MEDCouplingMemArray.hxx"

#include "VTKMEDTraits.hxx"

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#include <map>
#include <deque>
#include <sstream>
#include <algorithm>

vtkStandardNewMacro(vtkDevelopedSurface);

///////////////////

template<class T>
struct VTKTraits
{
};

template<>
struct VTKTraits<double>
{
  typedef vtkDoubleArray ArrayType;
};

template<>
struct VTKTraits<float>
{
  typedef vtkFloatArray ArrayType;
};

void ExtractInfo(vtkInformationVector *inputVector, vtkDataSet *& usgIn)
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
        throw MZCException("Input dataSet must be a DataSet or single elt multi block dataset expected !");
      if(input1->GetNumberOfBlocks()!=1)
        throw MZCException("Input dataSet is a multiblock dataset with not exactly one block ! Use MergeBlocks or ExtractBlocks filter before calling this filter !");
      vtkDataObject *input2(input1->GetBlock(0));
      if(!input2)
        throw MZCException("Input dataSet is a multiblock dataset with exactly one block but this single element is NULL !");
      vtkDataSet *input2c(vtkDataSet::SafeDownCast(input2));
      if(!input2c)
        throw MZCException("Input dataSet is a multiblock dataset with exactly one block but this single element is not a dataset ! Use MergeBlocks or ExtractBlocks filter before calling this filter !");
      input=input2c;
    }
  if(!input)
    throw MZCException("Input data set is NULL !");
  vtkPointData *att(input->GetPointData());
  if(!att)
    throw MZCException("Input dataset has no point data attribute ! Impossible to deduce a developed surface on it !");
  usgIn=input;
}

class vtkDevelopedSurface::vtkInternals
{
public:
  vtkNew<vtkCutter> Cutter;
};

////////////////////

vtkDevelopedSurface::vtkDevelopedSurface():_cyl(nullptr),Internal(new vtkInternals),InvertStatus(false),OffsetInRad(0.)
{
  //this->RegisterFilter(this->Internal->Cutter.GetPointer());
}

vtkDevelopedSurface::~vtkDevelopedSurface()
{
  delete this->Internal;
}

void vtkDevelopedSurface::SetInvertWay(bool invertStatus)
{
  this->InvertStatus=invertStatus;
  this->Modified();
}

void vtkDevelopedSurface::SetThetaOffset(double offsetInDegrees)
{
  double tmp(std::min(offsetInDegrees,180.));
  tmp=std::max(tmp,-180.);
  this->OffsetInRad=tmp/180.*M_PI;
  this->Modified();
}

int vtkDevelopedSurface::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{ 
  //std::cerr << "########################################## vtkDevelopedSurface::RequestInformation ##########################################" << std::endl;
  try
    {
      vtkDataSet *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
    }
  catch(MZCException& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkDevelopedSurface::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

std::vector<mcIdType> UnWrapByDuplicatingNodes(vtkCellArray *ca, vtkIdType& offset, const MEDCoupling::DataArrayDouble *thetas)
{
  std::vector<mcIdType> ret;
  vtkIdType nbCells(ca->GetNumberOfCells());
  vtkIdType *conn(ca->GetData()->GetPointer(0));
  const double *tptr(thetas->begin());
  for(vtkIdType i=0;i<nbCells;i++)
    {
      vtkIdType sz(*conn++);
      double ma(-std::numeric_limits<double>::max()),mi(std::numeric_limits<double>::max());
      for(vtkIdType j=0;j<sz;j++)
        {
          double tmp_theta(tptr[conn[j]]);
          ma=std::max(ma,tmp_theta);
          mi=std::min(mi,tmp_theta);
        }
      if(ma-mi>M_PI)
        {
          for(vtkIdType j=0;j<sz;j++)
            {
              double tmp_theta(tptr[conn[j]]);
              if(tmp_theta<=M_PI)
                {
                  ret.push_back(conn[j]);
                  conn[j]=offset++;
                }
            }
        }
      conn+=sz;
    }
  return ret;
}

template<class T>
void DealArray(vtkDataSetAttributes *pd, int pos, typename MEDFileVTKTraits<T>::VtkType *arr, std::vector<int>& nodeSel)
{
  int nbc(arr->GetNumberOfComponents());
  std::size_t nbt(nodeSel.size());
  vtkSmartPointer< typename MEDFileVTKTraits<T>::VtkType > newArr;
  newArr.TakeReference(MEDFileVTKTraits<T>::VtkType::New());
  newArr->SetNumberOfComponents(nbc);
  newArr->SetNumberOfTuples(nbt);
  T *ptr(newArr->GetPointer(0));
  const T *inPtr(arr->GetPointer(0));
  for(std::size_t i=0;i<nbt;i++)
    {
      ptr=std::copy(inPtr+nbc*nodeSel[i],inPtr+nbc*(nodeSel[i]+1),ptr);
    }
  newArr->SetName(arr->GetName());
  arr->DeepCopy(newArr);
}

void ToDouble(vtkDataArray *coords, vtkSmartPointer<vtkDoubleArray>& coordsOut)
{
  vtkDoubleArray *coords2(vtkDoubleArray::SafeDownCast(coords));
  vtkFloatArray *coords3(vtkFloatArray::SafeDownCast(coords));
  if(!coords2 && !coords3)
    throw MZCException("Input coordinates are neither float64 or float32 !");
  //
  if(coords2)
    {
      coordsOut.TakeReference(coords2);
      coords2->Register(0);
    }
  else
    {
      coordsOut.TakeReference(vtkDoubleArray::New());
      coordsOut->SetNumberOfComponents(3);
      vtkIdType nbTuples(coords3->GetNumberOfTuples());
      coordsOut->SetNumberOfTuples(nbTuples);
      std::copy(coords3->GetPointer(0),coords3->GetPointer(0)+3*nbTuples,coordsOut->GetPointer(0));
    }
}

void dealWith(vtkPolyData *outdata, const double center[3], const double axis[3], double radius, double eps, bool invertThetaInc, double offsetInRad)
{
  vtkDataArray *coords(outdata->GetPoints()->GetData());
  if(coords->GetNumberOfComponents()!=3)
    throw MZCException("Input coordinates are expected to have 3 components !");
  //
  vtkIdType nbNodes(coords->GetNumberOfTuples());
  if(nbNodes==0)
    throw MZCException("No points -> impossible to develop anything !");
  //
  vtkSmartPointer<vtkDoubleArray> zeCoords;
  ToDouble(coords,zeCoords);
  //
  double axis_cross_Z[3]={axis[1],-axis[0],0.};
  double n_axis(sqrt(axis_cross_Z[0]*axis_cross_Z[0]+axis_cross_Z[1]*axis_cross_Z[1]));
  if(n_axis>eps)
    {
      double ang(asin(n_axis));
      if(axis[2]<0.)
        ang=M_PI-ang;
      MEDCoupling::DataArrayDouble::Rotate3DAlg(center,axis_cross_Z,ang,nbNodes,zeCoords->GetPointer(0),zeCoords->GetPointer(0));
    }
  //
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> c_cyl;
  {
    MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> cc(MEDCoupling::DataArrayDouble::New()); cc->alloc(nbNodes,3);
    double *ccPtr(cc->getPointer());
    const double *zeCoordsPtr(zeCoords->GetPointer(0));
    for(vtkIdType i=0;i<nbNodes;i++,ccPtr+=3,zeCoordsPtr+=3)
      {
        std::transform(zeCoordsPtr,zeCoordsPtr+3,center,ccPtr,std::minus<double>());
      }
    c_cyl=cc->fromCartToCyl();
  }
  MEDCoupling::MCAuto<MEDCoupling::MEDFileData> mfd(MEDCoupling::MEDFileData::New());
  WriteMEDFileFromVTKDataSet(mfd,outdata,{},0.,0);
  bool a;
  {
    MEDCoupling::MEDFileMeshes *ms(mfd->getMeshes());
    if(ms->getNumberOfMeshes()!=1)
      throw MZCException("Unexpected number of meshes !");
    MEDCoupling::MEDFileMesh *mm(ms->getMeshAtPos(0));
    MEDCoupling::MEDFileUMesh *mmu(dynamic_cast<MEDCoupling::MEDFileUMesh *>(mm));
    if(!mmu)
      throw MZCException("Expecting unstructured one !");
    MEDCoupling::MCAuto<MEDCoupling::MEDCouplingUMesh> m0(mmu->getMeshAtLevel(0));
    {
      mcIdType v(0);
      MEDCoupling::MCAuto<MEDCoupling::DataArrayIdType> c0s(m0->getCellIdsLyingOnNodes(&v,&v+1,false));
      if(c0s->empty())
        throw MZCException("Orphan node 0 !");
      std::vector<mcIdType> nodes0;
      m0->getNodeIdsOfCell(c0s->getIJ(0,0),nodes0);
      MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> tmp0(c_cyl->selectByTupleIdSafe(nodes0.data(),nodes0.data()+nodes0.size()));
      tmp0=tmp0->keepSelectedComponents({1});
      double tmp(tmp0->getMaxAbsValueInArray());
      a=tmp>0.;
    }
  }
  //
  constexpr double EPS_FOR_RADIUS=1e-2;
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> rs(c_cyl->keepSelectedComponents({0}));
  if(!rs->isUniform(radius,radius*EPS_FOR_RADIUS))
    {
      double mi(rs->getMinValueInArray()),ma(rs->getMaxValueInArray());
      std::ostringstream oss; oss << "Looks not really a cylinder within given precision ! Range is [" << mi << "," << ma << "] expecting " << radius << " within precision of " << radius*EPS_FOR_RADIUS << " !";
      throw MZCException(oss.str());
    }
  double tetha0(c_cyl->getIJ(0,1));
  {
    double *ccylptr(c_cyl->getPointer()+1);
    double mi02(std::numeric_limits<double>::max());
    for(vtkIdType i=0;i<nbNodes;i++,ccylptr+=3)
      {
        *ccylptr-=tetha0+offsetInRad;
        mi02=std::min(mi02,*ccylptr);
      }
    if(mi02>0.)
      {
        ccylptr=c_cyl->getPointer()+1;
        for(vtkIdType i=0;i<nbNodes;i++,ccylptr+=3)
          {
            if(*ccylptr>=2*M_PI)
              {
                *ccylptr+=-2*M_PI;
              }
          }
      }
  }
  {
    MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> c_cyl_2(c_cyl->keepSelectedComponents({1}));
    c_cyl_2->abs();
    MEDCoupling::MCAuto<MEDCoupling::DataArrayIdType> poses(c_cyl_2->findIdsInRange(0.,eps));
    c_cyl->setPartOfValuesSimple3(0.,poses->begin(),poses->end(),1,2,1);
  }
  //
  if(a ^ (!invertThetaInc))
    {
      MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> tmp(c_cyl->keepSelectedComponents({1}));
      tmp=tmp->negate();
      std::for_each(tmp->getPointer(),tmp->getPointer()+tmp->getNumberOfTuples(),[](double& v) { if(v==-0.) v=0.; });
      c_cyl->setPartOfValues1(tmp,0,nbNodes,1,1,2,1);
    }
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> c_cyl_post(c_cyl->keepSelectedComponents({1}));
  {
    double *c_cyl_post_ptr(c_cyl_post->getPointer());
    for(vtkIdType i=0;i<nbNodes;i++,c_cyl_post_ptr++)
      {
        if(*c_cyl_post_ptr<0.)
          *c_cyl_post_ptr+=2*M_PI;
      }
  }
  vtkCellArray *cb(outdata->GetPolys());
  vtkIdType offset(nbNodes);
  std::vector<mcIdType> dupNodes(UnWrapByDuplicatingNodes(cb,offset,c_cyl_post));
  //
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> c_cyl_post2(c_cyl_post->selectByTupleId(dupNodes.data(),dupNodes.data()+dupNodes.size()));
  c_cyl_post2->applyLin(1.,2*M_PI);
  c_cyl_post=MEDCoupling::DataArrayDouble::Aggregate(c_cyl_post,c_cyl_post2);
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> z0(c_cyl->keepSelectedComponents({2}));
  MEDCoupling::MCAuto<MEDCoupling::DataArrayDouble> z1(z0->selectByTupleId(dupNodes.data(),dupNodes.data()+dupNodes.size()));
  z0=MEDCoupling::DataArrayDouble::Aggregate(z0,z1);
  //
  std::size_t outNbNodes(z0->getNumberOfTuples());
  vtkSmartPointer<vtkDoubleArray> zeCoords2;
  zeCoords2.TakeReference(vtkDoubleArray::New());
  zeCoords2->SetNumberOfComponents(3);
  zeCoords2->SetNumberOfTuples(outNbNodes);
  {
    const double *tptr(c_cyl_post->begin()),*zptr(z0->begin());
    double *outPtr(zeCoords2->GetPointer(0));
    for(std::size_t i=0;i<outNbNodes;i++,tptr++,zptr++,outPtr+=3)
      {
        outPtr[0]=radius*(*tptr);
        outPtr[1]=(*zptr);
        outPtr[2]=0.;
      }
  }
  //
  outdata->GetPoints()->SetData(zeCoords2);
  // now post process nodes
  std::vector<int> nodeSel(nbNodes+dupNodes.size());
  {
    int cnt(0);
    std::for_each(nodeSel.begin(),nodeSel.begin()+nbNodes,[&cnt](int& v){ v=cnt++; });
    std::copy(dupNodes.begin(),dupNodes.end(),nodeSel.begin()+nbNodes);
  }
  vtkDataSetAttributes *pd(outdata->GetPointData());
  int nba(pd->GetNumberOfArrays());
  for(int i=0;i<nba;i++)
    {
      vtkDataArray *arr(pd->GetArray(i));
      {
        vtkIntArray *arr0(vtkIntArray::SafeDownCast(arr));
        if(arr0)
          {
            DealArray<int>(pd,i,arr0,nodeSel);
            continue;
          }
      }
      {
        vtkFloatArray *arr0(vtkFloatArray::SafeDownCast(arr));
        if(arr0)
          {
            DealArray<float>(pd,i,arr0,nodeSel);
            continue;
          }
      }
      {
        vtkDoubleArray *arr0(vtkDoubleArray::SafeDownCast(arr));
        if(arr0)
          {
            DealArray<double>(pd,i,arr0,nodeSel);
            continue;
          }
      }
    }
}

int vtkDevelopedSurface::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkDevelopedSurface::RequestData        ##########################################" << std::endl;
  try
    {
      if(!_cyl)
        throw MZCException("No cylinder object as cut function !");
      double center[3],axis[3],radius;
      vtkAbstractTransform* trf(_cyl->GetTransform());
      {
        _cyl->GetCenter(center);
        _cyl->GetAxis(axis[0],axis[1],axis[2]);
        radius=_cyl->GetRadius();
      }
      if(trf)
        {
          double axis3[3]={center[0]+0.,center[1]+1.,center[2]+0.},axis4[3];
          trf->TransformPoint(axis3,axis4);
          std::transform(axis4,axis4+3,center,axis,[](double a, double b) { return b-a; });
          axis[1]=-axis[1];
          if(std::isnan(axis[0]) && std::isnan(axis[1]) && std::isnan(axis[2]))
            { axis[0]=0.; axis[1]=-1.; axis[2]=0.; }
        }
      //std::cerr << trf << " jjj " << axis[0] << " " << axis[1] << " " << axis[2]  << " : " << center[0] <<  " " << center[1] << " " << center[2] <<  " " " " << " -> " << radius << std::endl;
      vtkDataSet *usgIn(0);
      ExtractInfo(inputVector[0],usgIn);
      vtkSmartPointer<vtkPolyData> outData;
      {
        vtkNew<vtkCutter> Cutter;
        Cutter->SetInputData(usgIn);
        Cutter->SetCutFunction(_cyl);
        Cutter->Update();
        vtkDataSet *zeComputedOutput(Cutter->GetOutput());
        vtkPolyData *zeComputedOutput2(vtkPolyData::SafeDownCast(zeComputedOutput));
        if(!zeComputedOutput2)
          throw MZCException("Unexpected output of cutter !");
        outData.TakeReference(zeComputedOutput2);
        zeComputedOutput2->Register(0);
      }
      if(outData->GetNumberOfCells()==0)
        return 1;// no cells -> nothing to do
      //
      dealWith(outData,center,axis,radius,1e-7,this->InvertStatus,this->OffsetInRad);
      //finish
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkPolyData *output(vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      output->ShallowCopy(outData);
    }
  catch(MZCException& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkDevelopedSurface::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

int vtkDevelopedSurface::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}


void vtkDevelopedSurface::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkDevelopedSurface::SetCutFunction(vtkImplicitFunction* func)
{
  vtkCylinder *cyl(vtkCylinder::SafeDownCast(func));
  if(cyl)
    {
      _cyl=cyl;
      this->Modified();
    }
}

vtkMTimeType vtkDevelopedSurface::GetMTime()
{
  vtkMTimeType maxMTime = this->Superclass::GetMTime(); // My MTime
  if(_cyl)
    {
      maxMTime=std::max(maxMTime,_cyl->GetMTime());
    }
  return maxMTime;
}
