// Copyright (C) 2017  CEA/DEN, EDF R&D
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

#include "vtkSimpleMode.h"

#include "vtkAdjacentVertexIterator.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkIntArray.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSet.h"
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
#include "vtkWarpScalar.h"
#include "vtkWarpVector.h"
#include "vtkMultiBlockDataGroupFilter.h"
#include "vtkCompositeDataToUnstructuredGridFilter.h"

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include <map>
#include <deque>
#include <sstream>

vtkStandardNewMacro(vtkSimpleMode);

static const char ZE_DISPLACEMENT_NAME1[]="@@ForReal?@@";

static const char ZE_DISPLACEMENT_NAME2[]="@@ForImag?@@";

static const char ZE_DISPLACEMENT_NAME3[]="MagnitudeOfCpxDisp";

static const double EPS=1e-12;

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

vtkSmartPointer<vtkDoubleArray> ForceTo3Compo(vtkDoubleArray *arr)
{
  if(!arr)
    return vtkSmartPointer<vtkDoubleArray>();
  int nbCompo(arr->GetNumberOfComponents()),nbTuples(arr->GetNumberOfTuples());
  if(nbCompo==3)
    {
      vtkSmartPointer<vtkDoubleArray> ret(arr);
      return ret;
    }
  if(nbCompo==6)
    {
      vtkSmartPointer<vtkDoubleArray> ret(vtkSmartPointer<vtkDoubleArray>::New());
      ret->SetNumberOfComponents(3);
      ret->SetNumberOfTuples(nbTuples);
      const double *srcPt(arr->Begin());
      double *destPt(ret->Begin());
      for(int i=0;i<nbTuples;i++,destPt+=3,srcPt+=6)
        std::copy(srcPt,srcPt+3,destPt);
      return ret;
    }
  throw MZCException("ForceTo3Compo : internal error ! 6 or 3 compo arrays expected !");
}

std::vector< std::string > GetPossibleArrayNames(vtkDataSet *dataset)
{
  if(!dataset)
    throw MZCException("The input dataset is null !");
  std::vector< std::string > ret;
  vtkPointData *att(dataset->GetPointData());
  for(int i=0;i<att->GetNumberOfArrays();i++)
    {
      vtkDataArray *locArr(att->GetArray(i));
      int nbComp(locArr->GetNumberOfComponents());
      if(nbComp!=3 && nbComp!=6)
        continue;
      std::string s(locArr->GetName());
      ret.push_back(s);
    }
  return ret;
}

std::string FindTheBest(const std::vector<std::string>& arrNames, const std::string& key0, const std::string& key1)
{
  std::string ret;
  char points(0);
  if(arrNames.empty())
    return ret;
  for(std::vector<std::string>::const_iterator it=arrNames.begin();it!=arrNames.end();it++)
    {
      char curNbPts(1);
      if((*it).find(key0,0)!=std::string::npos)
        curNbPts++;
      if((*it).find(key1,0)!=std::string::npos)
        curNbPts++;
      if(curNbPts>points)
        {
          points=curNbPts;
          ret=*it;
        }
    }
  return ret;
}

std::string FindBestRealAmong(const std::vector<std::string>& arrNames)
{
  static const char KEY1[]="DEPL";
  static const char KEY2[]="REEL";
  return FindTheBest(arrNames,KEY1,KEY2);
}

std::string FindBestImagAmong(const std::vector<std::string>& arrNames)
{
  static const char KEY1[]="DEPL";
  static const char KEY2[]="IMAG";
  return FindTheBest(arrNames,KEY1,KEY2);
}

vtkUnstructuredGrid *ExtractInfo1(vtkInformationVector *inputVector)
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
  vtkUnstructuredGrid *usgIn(vtkUnstructuredGrid::SafeDownCast(input));
  if(!usgIn)
    throw MZCException("Input data set is not an unstructured mesh ! This filter works only on unstructured meshes !");
  return usgIn;
}

void ExtractInfo3(vtkDataSet *ds, const std::string& arrName, vtkDataArray *& arr, int& idx)
{
  vtkPointData *att(ds->GetPointData());
  if(!att)
    throw MZCException("Input dataset has no point data attribute ! Impossible to move mesh !");
  vtkDataArray *zeArr(0);
  int i(0);
  for(;i<att->GetNumberOfArrays();i++)
    {
      vtkDataArray *locArr(att->GetArray(i));
      std::string s(locArr->GetName());
      if(s==arrName)
        {
          zeArr=locArr;
          break;
        }
    }
  if(!zeArr)
    {
      std::ostringstream oss;
      oss << "Impossible to locate the array called \"" << arrName << "\" used to move mesh !";
      throw MZCException(oss.str());
    }
  arr=zeArr;
  idx=i;
}

  
void ExtractInfo2(vtkDataSet *ds, const std::string& arrName, vtkDoubleArray *& arr)
{
  vtkDataArray *zeArr(0);
  int dummy;
  ExtractInfo3(ds,arrName,zeArr,dummy);
  arr=vtkDoubleArray::SafeDownCast(zeArr);
  if(!arr)
    {
      std::ostringstream oss;
      oss << "Array called \"" << arrName << "\" has been located but this is NOT a float64 array !";
      throw MZCException(oss.str());
    }
  if(arr->GetNumberOfComponents()!=3 && arr->GetNumberOfComponents()!=6)
    {
      std::ostringstream oss;
      oss << "Float64 array called \"" << arrName << "\" has been located but this array has not exactly 3 or 6 components as it should !";
      throw MZCException(oss.str());
    }
  if(arr->GetNumberOfTuples()!=ds->GetNumberOfPoints())
    {
      std::ostringstream oss;
      oss << "Float64-1 components array called \"" << arrName << "\" has been located but the number of tuples is invalid ! Should be " << ds->GetNumberOfPoints() << " instead of " << arr->GetNumberOfTuples() << " !";
      throw MZCException(oss.str());
    }
}

////////////////////

class vtkSimpleMode::vtkSimpleModeInternal
{
public:
  vtkSimpleModeInternal():_surface(0) { }
  vtkPolyData *performConnection(vtkDataSet *ds);
  void setFieldForReal(const std::string& st) { _real=st; }
  std::string getFieldForReal() const { return _real; }
  ~vtkSimpleModeInternal();
private:
  vtkDataSetSurfaceFilter *_surface;
private:
  std::string _real;
};

vtkPolyData *vtkSimpleMode::vtkSimpleModeInternal::performConnection(vtkDataSet *ds)
{
  if(!_surface)
    {
      _surface=vtkDataSetSurfaceFilter::New();
      _surface->SetInputData(ds);
    }
  _surface->Update();
  return _surface->GetOutput();
}

vtkSimpleMode::vtkSimpleModeInternal::~vtkSimpleModeInternal()
{
  if(_surface)
    _surface->Delete();
}

vtkSimpleMode::vtkSimpleMode():Factor(1.),AnimationTime(0.),Internal(new vtkSimpleMode::vtkSimpleModeInternal)
{
  //this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,vtkDataSetAttributes::VECTORS);
}

vtkSimpleMode::~vtkSimpleMode()
{
  delete this->Internal;
}

void vtkSimpleMode::SetInputArrayToProcess(int idx, int port, int connection, int ff, const char *name)
{
  if(idx==0)
    this->Internal->setFieldForReal(name);
  vtkDataSetAlgorithm::SetInputArrayToProcess(idx,port,connection,ff,name);
}

double GetOptimalRatioFrom(vtkUnstructuredGrid *dataset, vtkDoubleArray *array)
{
  if(!dataset || !array)
    throw MZCException("The input dataset and or array is null !");
  vtkDataArray *coords(dataset->GetPoints()->GetData());
  vtkDoubleArray *coords2(vtkDoubleArray::SafeDownCast(coords));
  if(!coords2)
    throw MZCException("Input coordinates are not float64 !");
  int nbCompo(array->GetNumberOfComponents());
  if(coords2->GetNumberOfComponents()!=3 || (nbCompo!=3 && nbCompo!=6))
    throw MZCException("Input coordinates do not have 3 components as it should !");
  int nbPts(dataset->GetNumberOfPoints());
  const double *srcPt1(array->Begin());
  dataset->ComputeBounds();
  double *minmax1(dataset->GetBounds());
  double minmax2[3]={0.,0.,0.};
  for(int i=0;i<nbPts;i++,srcPt1+=nbCompo)
    {
      minmax2[0]=std::max(fabs(srcPt1[0]),minmax2[0]);
      minmax2[1]=std::max(fabs(srcPt1[1]),minmax2[1]);
      minmax2[2]=std::max(fabs(srcPt1[2]),minmax2[2]);
    }
  double maxDispDelta(*std::max_element(minmax2,minmax2+3));
  if(maxDispDelta<EPS)
    maxDispDelta=1.;
  for(int i=0;i<3;i++)
    minmax2[i]=minmax1[2*i+1]-minmax1[2*i];
  double maxGeoDelta(*std::max_element(minmax2,minmax2+3));
  if(maxDispDelta<EPS)
    maxDispDelta=1.;
  return maxGeoDelta/maxDispDelta;
}

int vtkSimpleMode::FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

int vtkSimpleMode::RequestInformation(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{ 
  //std::cerr << "########################################## vtkSimpleMode::RequestInformation ##########################################" << std::endl;
  try
    {
      if(this->Internal->getFieldForReal().empty())
        return 1;
      /*vtkUnstructuredGrid *usgIn(0);
      vtkDoubleArray *arr(0);
      ExtractInfo(inputVector[0],usgIn,this->Internal->getFieldForReal(),arr);
      std::vector<std::string> candidatesArrName(GetPossibleArrayNames(usgIn));
      //
      double ratio(GetOptimalRatioFrom(usgIn,arr));
      std::string optArrNameForReal(FindBestRealAmong(candidatesArrName));
      std::string optArrNameForImag(FindBestImagAmong(candidatesArrName));*/
      //std::cerr << ratio << std::endl;
      //std::cerr << optArrNameForReal << " * " << optArrNameForImag << std::endl;
    }
  catch(MZCException& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkSimpleMode::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

int vtkSimpleMode::RequestData(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  //std::cerr << "########################################## vtkSimpleMode::RequestData        ##########################################" << std::endl;
  try
    {
      vtkUnstructuredGrid *usgIn(0);
      usgIn=ExtractInfo1(inputVector[0]);
      //
      int nbPts(usgIn->GetNumberOfPoints());
      vtkPolyData *outSurface(this->Internal->performConnection(usgIn));
      vtkDoubleArray *arrRealBase(0);
      ExtractInfo2(outSurface,this->Internal->getFieldForReal(),arrRealBase);
      vtkSmartPointer<vtkDoubleArray> arrReal(ForceTo3Compo(arrRealBase));
      vtkSmartPointer<vtkDoubleArray> arr1(vtkSmartPointer<vtkDoubleArray>::New());
      arr1->SetName(ZE_DISPLACEMENT_NAME1);
      arr1->SetNumberOfComponents(3);
      arr1->SetNumberOfTuples(nbPts);
      double *ptToFeed1(arr1->Begin());
      const double *srcPt1(arrReal->Begin());
      double cst1(Factor*sin(AnimationTime*2*M_PI));
      std::transform(srcPt1,srcPt1+3*nbPts,ptToFeed1,std::bind2nd(std::multiplies<double>(),cst1));
      int idx1(outSurface->GetPointData()->AddArray(arr1));
      outSurface->GetPointData()->SetActiveAttribute(idx1,vtkDataSetAttributes::VECTORS);
      //
      //
      vtkSmartPointer<vtkWarpVector> ws(vtkSmartPointer<vtkWarpVector>::New());//vtkNew
      ws->SetInputData(outSurface);
      ws->SetScaleFactor(1.);
      ws->SetInputArrayToProcess(idx1,0,0,"vtkDataObject::FIELD_ASSOCIATION_POINTS",ZE_DISPLACEMENT_NAME1);
      ws->Update();
      vtkSmartPointer<vtkDataSet> ds(ws->GetOutput());
      ds->GetPointData()->RemoveArray(idx1);
      //
      int idx2(0);
      {
        vtkDataArray *dummy(0);
        ExtractInfo3(ds,this->Internal->getFieldForReal(),dummy,idx2);
      }
      //
      vtkInformation *outInfo(outputVector->GetInformationObject(0));
      vtkPolyData *output(vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
      output->ShallowCopy(ds);

      vtkDataArray* array = ds->GetPointData()->GetArray(idx2);
      vtkSmartPointer<vtkDataArray> result = vtkSmartPointer<vtkDataArray>::Take(vtkDataArray::CreateDataArray(array->GetDataType()));
      result->ShallowCopy(array);
      result->SetName("Result");
      output->GetPointData()->SetScalars(result);            
    }
  catch(MZCException& e)
    {
      std::ostringstream oss;
      oss << "Exception has been thrown in vtkSimpleMode::RequestInformation : " << e.what() << std::endl;
      if(this->HasObserver("ErrorEvent") )
        this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      else
        vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str()));
      vtkObject::BreakOnError();
      return 0;
    }
  return 1;
}

void vtkSimpleMode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
