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

#include "vtkSimpleMode.h"

#include <vtkAdjacentVertexIterator.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkCompositeDataToUnstructuredGridFilter.h>
#include <vtkDataArraySelection.h>
#include <vtkDataObjectTreeIterator.h>
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkInEdgeIterator.h>
#include <vtkInformation.h>
#include <vtkInformationDataObjectKey.h>
#include <vtkInformationStringKey.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkMultiBlockDataGroupFilter.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMutableDirectedGraph.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkTimeStamp.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVariantArray.h>
#include <vtkWarpScalar.h>
#include <vtkWarpVector.h>

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <functional>
#include <math.h>
#endif

#include <deque>
#include <map>
#include <sstream>

vtkStandardNewMacro(vtkSimpleMode)

static const char ZE_DISPLACEMENT_NAME1[] = "@@ForReal?@@";

static const char ZE_DISPLACEMENT_NAME2[] = "@@ForImag?@@";

static const char ZE_DISPLACEMENT_NAME3[] = "MagnitudeOfCpxDisp";

static const double EPS = 1e-12;

///////////////////

class MZCException : public std::exception
{
public:
  MZCException(const std::string& s)
    : _reason(s)
  {
  }
  virtual const char* what() const noexcept { return _reason.c_str(); }
  virtual ~MZCException() noexcept {}

private:
  std::string _reason;
};

// ValueTypeT

/*template<class T>
struct ArrayTraits
{
  typedef T EltType;
  };*/

template <class VTK_ARRAY_T>
vtkSmartPointer<VTK_ARRAY_T> ForceTo3CompoImpl(VTK_ARRAY_T* arr)
{
  using ELT_TYPE = typename VTK_ARRAY_T::ValueType;
  if (!arr)
    return vtkSmartPointer<VTK_ARRAY_T>();
  vtkIdType nbCompo(arr->GetNumberOfComponents()), nbTuples(arr->GetNumberOfTuples());
  if (nbCompo == 3)
  {
    vtkSmartPointer<VTK_ARRAY_T> ret(arr);
    return ret;
  }
  if (nbCompo == 6)
  {
    vtkSmartPointer<VTK_ARRAY_T> ret(vtkSmartPointer<VTK_ARRAY_T>::New());
    ret->SetNumberOfComponents(3);
    ret->SetNumberOfTuples(nbTuples);
    const ELT_TYPE* srcPt(arr->Begin());
    ELT_TYPE* destPt(ret->Begin());
    for (vtkIdType i = 0; i < nbTuples; i++, destPt += 3, srcPt += 6)
      std::copy(srcPt, srcPt + 3, destPt);
    return ret;
  }
  throw MZCException("ForceTo3CompoImpl : internal error ! 6 or 3 compo arrays expected !");
}

vtkSmartPointer<vtkDataArray> ForceTo3Compo(vtkDataArray* arr)
{
  vtkDoubleArray* arr0(vtkDoubleArray::SafeDownCast(arr));
  if (arr0)
    return ForceTo3CompoImpl<vtkDoubleArray>(arr0);
  vtkFloatArray* arr1(vtkFloatArray::SafeDownCast(arr));
  if (arr1)
    return ForceTo3CompoImpl<vtkFloatArray>(arr1);
  throw MZCException("ForceTo3Compo : array is NEITHER float64 NOR float32 array !");
}

template <class VTK_ARRAY_T>
void FeedDataInternal(VTK_ARRAY_T* arrReal, double cst1, double* ptToFeed1)
{
  vtkIdType nbTuples(arrReal->GetNumberOfTuples());
  using ELT_TYPE = typename VTK_ARRAY_T::ValueType;
  const ELT_TYPE* srcPt1(arrReal->Begin());
  std::for_each(srcPt1, srcPt1 + 3 * nbTuples, [&ptToFeed1, cst1](const ELT_TYPE& elt) {
    *ptToFeed1 = (double)elt * cst1;
    ptToFeed1++;
  });
}

void FeedData(vtkDataArray* arr, double cst1, double* ptToFeed1)
{
  vtkDoubleArray* arr0(vtkDoubleArray::SafeDownCast(arr));
  if (arr0)
    return FeedDataInternal<vtkDoubleArray>(arr0, cst1, ptToFeed1);
  vtkFloatArray* arr1(vtkFloatArray::SafeDownCast(arr));
  if (arr1)
    return FeedDataInternal<vtkFloatArray>(arr1, cst1, ptToFeed1);
  throw MZCException("FeedData : array is NEITHER float64 NOR float32 array !");
}

std::vector<std::string> GetPossibleArrayNames(vtkDataSet* dataset)
{
  if (!dataset)
    throw MZCException("The input dataset is null !");
  std::vector<std::string> ret;
  vtkPointData* att(dataset->GetPointData());
  for (int i = 0; i < att->GetNumberOfArrays(); i++)
  {
    vtkDataArray* locArr(att->GetArray(i));
    int nbComp(locArr->GetNumberOfComponents());
    if (nbComp != 3 && nbComp != 6)
      continue;
    std::string s(locArr->GetName());
    ret.push_back(s);
  }
  return ret;
}

std::string FindTheBest(
  const std::vector<std::string>& arrNames, const std::string& key0, const std::string& key1)
{
  std::string ret;
  char points(0);
  if (arrNames.empty())
    return ret;
  for (std::vector<std::string>::const_iterator it = arrNames.begin(); it != arrNames.end(); it++)
  {
    char curNbPts(1);
    if ((*it).find(key0, 0) != std::string::npos)
      curNbPts++;
    if ((*it).find(key1, 0) != std::string::npos)
      curNbPts++;
    if (curNbPts > points)
    {
      points = curNbPts;
      ret = *it;
    }
  }
  return ret;
}

std::string FindBestRealAmong(const std::vector<std::string>& arrNames)
{
  static const char KEY1[] = "DEPL";
  static const char KEY2[] = "REEL";
  return FindTheBest(arrNames, KEY1, KEY2);
}

std::string FindBestImagAmong(const std::vector<std::string>& arrNames)
{
  static const char KEY1[] = "DEPL";
  static const char KEY2[] = "IMAG";
  return FindTheBest(arrNames, KEY1, KEY2);
}

vtkUnstructuredGrid* ExtractInfo1(vtkInformationVector* inputVector)
{
  vtkInformation* inputInfo(inputVector->GetInformationObject(0));
  vtkDataSet* input(0);
  vtkDataSet* input0(vtkDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  vtkMultiBlockDataSet* input1(
    vtkMultiBlockDataSet::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  if (input0)
    input = input0;
  else
  {
    if (!input1)
      throw MZCException(
        "Input dataSet must be a DataSet or single elt multi block dataset expected !");
    if (input1->GetNumberOfBlocks() != 1)
      throw MZCException("Input dataSet is a multiblock dataset with not exactly one block ! Use "
                         "MergeBlocks or ExtractBlocks filter before calling this filter !");
    vtkDataObject* input2(input1->GetBlock(0));
    if (!input2)
      throw MZCException("Input dataSet is a multiblock dataset with exactly one block but this "
                         "single element is NULL !");
    vtkDataSet* input2c(vtkDataSet::SafeDownCast(input2));
    if (!input2c)
      throw MZCException(
        "Input dataSet is a multiblock dataset with exactly one block but this single element is "
        "not a dataset ! Use MergeBlocks or ExtractBlocks filter before calling this filter !");
    input = input2c;
  }
  if (!input)
    throw MZCException("Input data set is NULL !");
  vtkUnstructuredGrid* usgIn(vtkUnstructuredGrid::SafeDownCast(input));
  if (!usgIn)
    throw MZCException("Input data set is not an unstructured mesh ! This filter works only on "
                       "unstructured meshes !");
  return usgIn;
}

void ExtractInfo3(vtkDataSet* ds, const std::string& arrName, vtkDataArray*& arr, int& idx)
{
  vtkPointData* att(ds->GetPointData());
  if (!att)
    throw MZCException("Input dataset has no point data attribute ! Impossible to move mesh !");
  vtkDataArray* zeArr(0);
  int i(0);
  for (; i < att->GetNumberOfArrays(); i++)
  {
    vtkDataArray* locArr(att->GetArray(i));
    std::string s(locArr->GetName());
    if (s == arrName)
    {
      zeArr = locArr;
      break;
    }
  }
  if (!zeArr)
  {
    std::ostringstream oss;
    oss << "Impossible to locate the array called \"" << arrName << "\" used to move mesh !";
    throw MZCException(oss.str());
  }
  arr = zeArr;
  idx = i;
}

void ExtractInfo2(vtkDataSet* ds, const std::string& arrName, vtkDataArray*& arr)
{
  int dummy;
  ExtractInfo3(ds, arrName, arr, dummy);
  vtkDoubleArray* arr1(vtkDoubleArray::SafeDownCast(arr));
  vtkFloatArray* arr2(vtkFloatArray::SafeDownCast(arr));
  if (!arr1 && !arr2)
  {
    std::ostringstream oss;
    oss << "Array called \"" << arrName
        << "\" has been located but this is NEITHER float64 NOR float32 array !";
    throw MZCException(oss.str());
  }
  if (arr->GetNumberOfComponents() != 3 && arr->GetNumberOfComponents() != 6)
  {
    std::ostringstream oss;
    oss << "Float64 array called \"" << arrName
        << "\" has been located but this array has not exactly 3 or 6 components as it should !";
    throw MZCException(oss.str());
  }
  if (arr->GetNumberOfTuples() != ds->GetNumberOfPoints())
  {
    std::ostringstream oss;
    oss << "Float64-1 components array called \"" << arrName
        << "\" has been located but the number of tuples is invalid ! Should be "
        << ds->GetNumberOfPoints() << " instead of " << arr->GetNumberOfTuples() << " !";
    throw MZCException(oss.str());
  }
}

////////////////////

class vtkSimpleMode::vtkSimpleModeInternal
{
public:
  vtkSimpleModeInternal()
    : _surface(0)
  {
  }
  vtkPolyData* performConnection(vtkDataSet* ds);
  void setFieldForReal(const std::string& st) { _real = st; }
  std::string getFieldForReal() const { return _real; }
  ~vtkSimpleModeInternal();

private:
  vtkDataSetSurfaceFilter* _surface;

private:
  std::string _real;
};

vtkPolyData* vtkSimpleMode::vtkSimpleModeInternal::performConnection(vtkDataSet* ds)
{
  if (!_surface)
  {
    _surface = vtkDataSetSurfaceFilter::New();
    _surface->SetInputData(ds);
  }
  _surface->Update();
  return _surface->GetOutput();
}

vtkSimpleMode::vtkSimpleModeInternal::~vtkSimpleModeInternal()
{
  if (_surface)
    _surface->Delete();
}

vtkSimpleMode::vtkSimpleMode()
  : Factor(1.)
  , AnimationTime(0.)
  , Internal(new vtkSimpleMode::vtkSimpleModeInternal)
{
  // this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,vtkDataSetAttributes::VECTORS);
}

vtkSimpleMode::~vtkSimpleMode()
{
  delete this->Internal;
}

void vtkSimpleMode::SetInputArrayToProcess(
  int idx, int port, int connection, int ff, const char* name)
{
  if (idx == 0)
    this->Internal->setFieldForReal(name);
  vtkDataSetAlgorithm::SetInputArrayToProcess(idx, port, connection, ff, name);
}

double GetOptimalRatioFrom(vtkUnstructuredGrid* dataset, vtkDoubleArray* array)
{
  if (!dataset || !array)
    throw MZCException("The input dataset and or array is null !");
  vtkDataArray* coords(dataset->GetPoints()->GetData());
  vtkDoubleArray* coords2(vtkDoubleArray::SafeDownCast(coords));
  if (!coords2)
    throw MZCException("Input coordinates are not float64 !");
  int nbCompo(array->GetNumberOfComponents());
  if (coords2->GetNumberOfComponents() != 3 || (nbCompo != 3 && nbCompo != 6))
    throw MZCException("Input coordinates do not have 3 components as it should !");
  int nbPts(dataset->GetNumberOfPoints());
  const double* srcPt1(array->Begin());
  dataset->ComputeBounds();
  double* minmax1(dataset->GetBounds());
  double minmax2[3] = { 0., 0., 0. };
  for (int i = 0; i < nbPts; i++, srcPt1 += nbCompo)
  {
    minmax2[0] = std::max(fabs(srcPt1[0]), minmax2[0]);
    minmax2[1] = std::max(fabs(srcPt1[1]), minmax2[1]);
    minmax2[2] = std::max(fabs(srcPt1[2]), minmax2[2]);
  }
  double maxDispDelta(*std::max_element(minmax2, minmax2 + 3));
  if (maxDispDelta < EPS)
    maxDispDelta = 1.;
  for (int i = 0; i < 3; i++)
    minmax2[i] = minmax1[2 * i + 1] - minmax1[2 * i];
  double maxGeoDelta(*std::max_element(minmax2, minmax2 + 3));
  if (maxDispDelta < EPS)
    maxDispDelta = 1.;
  return maxGeoDelta / maxDispDelta;
}

int vtkSimpleMode::FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
  return 1;
}

int vtkSimpleMode::RequestInformation(
      vtkInformation* /*request*/, vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/)
{
  // std::cerr << "########################################## vtkSimpleMode::RequestInformation
  // ##########################################" << std::endl;
  try
  {
    if (this->Internal->getFieldForReal().empty())
      return 1;
    /*vtkUnstructuredGrid *usgIn(0);
    vtkDoubleArray *arr(0);
    ExtractInfo(inputVector[0],usgIn,this->Internal->getFieldForReal(),arr);
    std::vector<std::string> candidatesArrName(GetPossibleArrayNames(usgIn));
    //
    double ratio(GetOptimalRatioFrom(usgIn,arr));
    std::string optArrNameForReal(FindBestRealAmong(candidatesArrName));
    std::string optArrNameForImag(FindBestImagAmong(candidatesArrName));*/
    // std::cerr << ratio << std::endl;
    // std::cerr << optArrNameForReal << " * " << optArrNameForImag << std::endl;
  }
  catch (MZCException& e)
  {
    std::ostringstream oss;
    oss << "Exception has been thrown in vtkSimpleMode::RequestInformation : " << e.what()
        << std::endl;
    if (this->HasObserver("ErrorEvent"))
      this->InvokeEvent("ErrorEvent", const_cast<char*>(oss.str().c_str()));
    else
      vtkOutputWindowDisplayErrorText(const_cast<char*>(oss.str().c_str()));
    vtkObject::BreakOnError();
    return 0;
  }
  return 1;
}

int vtkSimpleMode::RequestData(
    vtkInformation* /*request*/, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // std::cerr << "########################################## vtkSimpleMode::RequestData
  // ##########################################" << std::endl;
  try
  {
    vtkUnstructuredGrid* usgIn(0);
    usgIn = ExtractInfo1(inputVector[0]);
    //
    int nbPts(usgIn->GetNumberOfPoints());
    vtkPolyData* outSurface(this->Internal->performConnection(usgIn));
    vtkDataArray* arrRealBase(nullptr);
    ExtractInfo2(outSurface, this->Internal->getFieldForReal(), arrRealBase);
    vtkSmartPointer<vtkDataArray> arrReal(ForceTo3Compo(arrRealBase));
    vtkSmartPointer<vtkDoubleArray> arr1(vtkSmartPointer<vtkDoubleArray>::New());
    arr1->SetName(ZE_DISPLACEMENT_NAME1);
    arr1->SetNumberOfComponents(3);
    arr1->SetNumberOfTuples(nbPts);
    double* ptToFeed1(arr1->Begin());
    double cst1(Factor * cos(AnimationTime * 2 * M_PI));
    FeedData(arrReal, cst1, ptToFeed1);
    int idx1(outSurface->GetPointData()->AddArray(arr1));
    outSurface->GetPointData()->SetActiveAttribute(idx1, vtkDataSetAttributes::VECTORS);
    //
    //
    vtkSmartPointer<vtkWarpVector> ws(vtkSmartPointer<vtkWarpVector>::New()); // vtkNew
    ws->SetInputData(outSurface);
    ws->SetScaleFactor(1.);
    ws->SetInputArrayToProcess(
      idx1, 0, 0, "vtkDataObject::FIELD_ASSOCIATION_POINTS", ZE_DISPLACEMENT_NAME1);
    ws->Update();
    vtkSmartPointer<vtkDataSet> ds(ws->GetOutput());
    ds->GetPointData()->RemoveArray(idx1);
    //
    int idx2(0);
    {
      vtkDataArray* dummy(0);
      ExtractInfo3(ds, this->Internal->getFieldForReal(), dummy, idx2);
    }
    //
    vtkInformation* outInfo(outputVector->GetInformationObject(0));
    vtkPolyData* output(vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
    output->ShallowCopy(ds);
    output->GetPointData()->DeepCopy(ds->GetPointData());
    //
    for (int i = 0; i < output->GetPointData()->GetNumberOfArrays(); i++)
    {
      vtkDataArray* arr(output->GetPointData()->GetArray(i));
      vtkDoubleArray* arr2(vtkDoubleArray::SafeDownCast(arr));
      if (!arr2)
        continue;
      vtkIdType nbCompo(arr2->GetNumberOfComponents()), nbTuples(arr2->GetNumberOfTuples());
      if (nbCompo != 3 && nbCompo != 2)
        continue;
      double* arrPtr(arr2->GetPointer(0));
      std::transform(arrPtr, arrPtr + nbCompo * nbTuples, arrPtr,
        std::bind(std::multiplies<double>(),std::placeholders::_1, cos(AnimationTime * 2 * M_PI)));
    }
    //
    vtkDataArray* array = output->GetPointData()->GetArray(idx2);
    vtkSmartPointer<vtkDataArray> result =
      vtkSmartPointer<vtkDataArray>::Take(vtkDataArray::CreateDataArray(array->GetDataType()));
    result->ShallowCopy(array);
    result->SetName("__NormalModesAnimation__");
    output->GetPointData()->SetScalars(result);
  }
  catch (MZCException& e)
  {
    std::ostringstream oss;
    oss << "Exception has been thrown in vtkSimpleMode::RequestInformation : " << e.what()
        << std::endl;
    if (this->HasObserver("ErrorEvent"))
      this->InvokeEvent("ErrorEvent", const_cast<char*>(oss.str().c_str()));
    else
      vtkOutputWindowDisplayErrorText(const_cast<char*>(oss.str().c_str()));
    vtkObject::BreakOnError();
    return 0;
  }
  return 1;
}

void vtkSimpleMode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
