// Copyright (C) 2010-2014  CEA/DEN, EDF R&D
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

#include "vtkELNOFilter.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkPolyData.h"
#include "vtkIdTypeArray.h"
#include "vtkFieldData.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkCell.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkQuadratureSchemeDefinition.h"
#include "vtkUnstructuredGrid.h"

#include "MEDUtilities.hxx"
#include "InterpKernelAutoPtr.hxx"

//vtkCxxRevisionMacro(vtkELNOFilter, "$Revision: 1.2.2.2 $");
vtkStandardNewMacro(vtkELNOFilter);

vtkELNOFilter::vtkELNOFilter()
{
  this->ShrinkFactor = 0.5;
}

vtkELNOFilter::~vtkELNOFilter()
{
}

int vtkELNOFilter::RequestData(vtkInformation *request, vtkInformationVector **input, vtkInformationVector *output)
{
  vtkUnstructuredGrid *usgIn(vtkUnstructuredGrid::SafeDownCast( input[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));
  vtkPolyData *pdOut(vtkPolyData::SafeDownCast(output->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));

  vtkDataArray *array(this->GetInputArrayToProcess(0, input));
  vtkIdTypeArray* offsets(vtkIdTypeArray::SafeDownCast(this->GetInputArrayToProcess(0, input)));

  if(usgIn == NULL || offsets == NULL || pdOut == NULL)
    {
      vtkDebugMacro("vtkELNOFilter no correctly configured : offsets = " << offsets);
      return 1;
    }

  vtkInformation *info(offsets->GetInformation());
  vtkInformationQuadratureSchemeDefinitionVectorKey *key(vtkQuadratureSchemeDefinition::DICTIONARY());
  if(!key->Has(info))
    {
      vtkDebugMacro("Dictionary is not present in array " << offsets->GetName() << " " << offsets << " Aborting." );
      return 1;
    }

  int res(this->Superclass::RequestData(request, input, output));
  if(res == 0)
    return 0;
  
  int dictSize(key->Size(info));
  vtkQuadratureSchemeDefinition **dict = new vtkQuadratureSchemeDefinition *[dictSize];
  key->GetRange(info, dict, 0, 0, dictSize);

  vtkIdType ncell(usgIn->GetNumberOfCells());
  vtkPoints *points(pdOut->GetPoints());
  vtkIdType start(0);
  for(vtkIdType cellId = 0; cellId < ncell; cellId++)
    {
      vtkIdType offset(offsets->GetValue(cellId));
      int cellType(usgIn->GetCellType(cellId));
      // a simple check to see if a scheme really exists for this cell type.
      // should not happen if the cell type has not been modified.
      if(dict[cellType] == NULL)
        continue;
      int np = dict[cellType]->GetNumberOfQuadraturePoints();
      double center[3] = {0, 0, 0};
      for(int id = start; id < start + np; id++)
        {
          double *position = points->GetPoint(id);
          center[0] += position[0];
          center[1] += position[1];
          center[2] += position[2];
        }
      center[0] /= np;
      center[1] /= np;
      center[2] /= np;
      for(int id = start; id < start + np; id++)
        {
          double *position = points->GetPoint(id);
          double newpos[3];
          newpos[0] = position[0] * this->ShrinkFactor + center[0] * (1 - this->ShrinkFactor);
          newpos[1] = position[1] * this->ShrinkFactor + center[1] * (1 - this->ShrinkFactor);
          newpos[2] = position[2] * this->ShrinkFactor + center[2] * (1 - this->ShrinkFactor);
          points->SetPoint(id, newpos);
        }
      start += np;
    }
  //// bug EDF 8407 PARAVIS - mantis 22610
  vtkFieldData *fielddata(usgIn->GetFieldData());
  for(int index=0;index<fielddata->GetNumberOfArrays();index++)
    {
      vtkDataArray *data(fielddata->GetArray(index));
      vtkInformation *info(data->GetInformation());
      const char *arrayOffsetName(info->Get(vtkQuadratureSchemeDefinition::QUADRATURE_OFFSET_ARRAY_NAME()));
      vtkIdTypeArray *offData(0);
      bool isELNO(false);
      if(arrayOffsetName)
        {
          vtkCellData *cellData(usgIn->GetCellData());
          vtkDataArray *offDataTmp(cellData->GetArray(arrayOffsetName));
          isELNO=offDataTmp->GetInformation()->Get(MEDUtilities::ELNO())==1;
          offData=dynamic_cast<vtkIdTypeArray *>(offDataTmp);
        }
      if(isELNO && offData)
        {
          vtkIdType nbCellsInput(usgIn->GetNumberOfCells());
          if(nbCellsInput==0)
            continue ;//no cells -> no fields
          // First trying to detected if we are in the special case where data can be used directly. To detect that look at offData. If offData.front()==0 && offData->back()+NbOfNodesInLastCell==data->GetNumberOfTuples() OK.
          vtkCell *cell(usgIn->GetCell(nbCellsInput-1));
          bool statement0(offData->GetTuple1(0)==0);
          bool statement1(offData->GetTuple1(nbCellsInput-1)+cell->GetNumberOfPoints()==data->GetNumberOfTuples());
          if(statement0 && statement1)
            pdOut->GetPointData()->AddArray(data);//We are lucky ! No extraction needed.
          else
            {//not lucky ! Extract values from data. A previous threshold has been done... Bug EDF8662
              vtkDataArray *newArray(data->NewInstance());
              newArray->SetName(data->GetName());
              pdOut->GetPointData()->AddArray(newArray);
              newArray->SetNumberOfComponents(data->GetNumberOfComponents());
              newArray->SetNumberOfTuples(pdOut->GetNumberOfPoints());
              newArray->CopyComponentNames(data);
              newArray->Delete();
              vtkIdType *offsetPtr(offData->GetPointer(0));
              vtkIdType zeId(0);
              for(vtkIdType cellId=0;cellId<nbCellsInput;cellId++)
                {
                  vtkCell *cell(usgIn->GetCell(cellId));
                  vtkIdType nbPoints(cell->GetNumberOfPoints()),offset(offsetPtr[cellId]);
                  for(vtkIdType j=0;j<nbPoints;j++,zeId++)
                    newArray->SetTuple(zeId,offsetPtr[cellId]+j,data);
                }
            }
        }
    }
  AttachCellFieldsOn(usgIn,pdOut->GetCellData(),pdOut->GetNumberOfCells());
  return 1;
}

void vtkELNOFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ShrinkFactor : " << this->ShrinkFactor << endl;
}

/*!
 * This method attach fields on cell of \a inGrid and add it as a point data in \a outData.
 */
void vtkELNOFilter::AttachCellFieldsOn(vtkUnstructuredGrid *inGrid, vtkCellData *outData, int nbCellsOut)
{
  vtkCellData *cd(inGrid->GetCellData());
  int nbOfArrays(cd->GetNumberOfArrays());
  vtkIdType nbCells(inGrid->GetNumberOfCells());
  if(nbOfArrays==0)
    return ;
  INTERP_KERNEL::AutoPtr<vtkIdType> tmpPtr(new vtkIdType[nbCells]);
  for(vtkIdType cellId=0;cellId<nbCells;cellId++)
    {
      vtkCell *cell(inGrid->GetCell(cellId));
      tmpPtr[cellId]=cell->GetNumberOfPoints();
    }
  for(int index=0;index<nbOfArrays;index++)
    {
      vtkDataArray *data(cd->GetArray(index));
      vtkDataArray *newArray(data->NewInstance());
      newArray->SetName(data->GetName());
      outData->AddArray(newArray);
      newArray->SetNumberOfComponents(data->GetNumberOfComponents());
      newArray->SetNumberOfTuples(nbCellsOut);
      newArray->CopyComponentNames(data);
      newArray->Delete();
      vtkIdType offset(0);
      for(vtkIdType cellId=0;cellId<nbCells;cellId++)
        {
          for(vtkIdType j=0;j<tmpPtr[cellId];j++,offset++)
            newArray->SetTuple(offset,cellId,data);
        }
    }
}
