// Copyright (C) 2020  CEA/DEN, EDF R&D
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

#include "vtkUgSelectCellIds.h"

#include "vtkCellArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkInformationVector.h"
#include "vtkUnsignedCharArray.h"
#include "vtkInformation.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

vtkStandardNewMacro(vtkUgSelectCellIds)

void vtkUgSelectCellIds::SetIds(vtkIdTypeArray *ids)
{
  _ids = ids;
}

int vtkUgSelectCellIds::RequestData(vtkInformation *vtkNotUsed(request), vtkInformationVector **inputVector, vtkInformationVector *outputVector)
{
  vtkInformation* inputInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid *ds(vtkUnstructuredGrid::SafeDownCast(inputInfo->Get(vtkDataObject::DATA_OBJECT())));
  if(!ds)
  {
    vtkErrorMacro("vtkUgSelectCellIds::RequestData : input is expected to be an unstructuredgrid !");
    return 0;
  }
  if( ! this->_ids.Get() )
  {
    vtkErrorMacro("vtkUgSelectCellIds::RequestData : not initialized array !");
    return 0;
  }
  vtkInformation *outInfo(outputVector->GetInformationObject(0));
  vtkUnstructuredGrid *output(vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT())));
  output->SetPoints(ds->GetPoints());
  vtkNew<vtkCellArray> outCellArray;
  vtkNew<vtkIdTypeArray> outConn;
  vtkCellData *inputCellData(ds->GetCellData());
  vtkPointData *inputPointData(ds->GetPointData());
  // Finish the parlote : feed data arrays
  vtkCellData *outCellData(output->GetCellData());
  vtkPointData *outPointData(output->GetPointData());
  vtkIdType inpNbCells( ds->GetNumberOfCells() );
  vtkIdType outputNbCells( _ids->GetNumberOfTuples() );
  vtkNew<vtkIdTypeArray> outNodalConn;
  vtkNew<vtkUnsignedCharArray> outCellTypes;
  outCellTypes->SetNumberOfComponents(1); outCellTypes->SetNumberOfTuples(outputNbCells);
  vtkNew<vtkIdTypeArray> outCellLocations;
  outCellLocations->SetNumberOfComponents(1); outCellLocations->SetNumberOfTuples(outputNbCells);
  vtkIdType outConnLgth(0);
  for( const vtkIdType *cellId = _ids->GetPointer(0) ; cellId != _ids->GetPointer(outputNbCells) ; ++cellId )
  {
    if( *cellId >=0 && *cellId < inpNbCells )
    {
      vtkIdType npts;
      const vtkIdType *pts;
      ds->GetCellPoints(*cellId, npts, pts);
      outConnLgth += 1+npts;
    }
    else
    {
      vtkErrorMacro(<< "vtkUgSelectCellIds::RequestData : presence of " << *cellId << " in array must be in [0," << inpNbCells << "[ !");
      return 0;
    } 
  }
  outNodalConn->SetNumberOfComponents(1); outNodalConn->SetNumberOfTuples(outConnLgth);
  vtkIdType *outConnPt(outNodalConn->GetPointer(0)),*outCellLocPt(outCellLocations->GetPointer(0));
  vtkIdType outCurCellLoc = 0;
  unsigned char *outCellTypePt(outCellTypes->GetPointer(0));
  for( const vtkIdType *cellId = _ids->GetPointer(0) ; cellId != _ids->GetPointer(outputNbCells) ; ++cellId )
  {
      outCellLocPt[0] = outCurCellLoc;
      vtkIdType npts;
      const vtkIdType *pts;
      ds->GetCellPoints(*cellId, npts, pts);
      *outConnPt++ = npts;
      outConnPt = std::copy(pts,pts+npts,outConnPt);
      *outCellTypePt++ = ds->GetCellType(*cellId);
      outCurCellLoc += npts+1;//not sure
  }
  for( int cellFieldId = 0 ; cellFieldId < inputCellData->GetNumberOfArrays() ; ++cellFieldId )
  {
    vtkDataArray *array( inputCellData->GetArray(cellFieldId) );
    vtkSmartPointer<vtkDataArray> outArray;
    outArray.TakeReference( array->NewInstance() );
    outArray->SetNumberOfComponents(array->GetNumberOfComponents()); outArray->SetNumberOfTuples(outputNbCells);
    outArray->SetName(array->GetName());
    vtkIdType pos(0);
    for( const vtkIdType *cellId = _ids->GetPointer(0) ; cellId != _ids->GetPointer(outputNbCells) ; ++cellId )
    {
      outArray->SetTuple(pos++,*cellId,array);
    }
    outCellData->AddArray(outArray);
  }
  for( int pointFieldId = 0 ; pointFieldId < inputPointData->GetNumberOfArrays() ; ++pointFieldId )
  {
    vtkDataArray *array( inputPointData->GetArray(pointFieldId) );
    vtkSmartPointer<vtkDataArray> outArray;
    outArray.TakeReference( array->NewInstance() );
    outArray->ShallowCopy(array);
    outPointData->AddArray(outArray);
  }
  //
  outCellArray->SetCells(outputNbCells,outNodalConn);
  output->SetCells(outCellTypes,outCellLocations,outCellArray);
  //
  return 1;
}
