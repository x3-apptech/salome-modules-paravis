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
// Author : Roman NIKOLAEV

//Local includes
#include "vtkMEDQuadraturePointsGenerator.h"
#include "MEDFileFieldRepresentationTree.hxx"

//VTK includes
#include <vtkObjectFactory.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkInformationQuadratureSchemeDefinitionVectorKey.h>
#include <vtkQuadratureSchemeDefinition.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMEDQuadraturePointsGenerator)

//-----------------------------------------------------------------------------
vtkMEDQuadraturePointsGenerator::vtkMEDQuadraturePointsGenerator()
{
}

//-----------------------------------------------------------------------------
vtkMEDQuadraturePointsGenerator::~vtkMEDQuadraturePointsGenerator()
{}


//-----------------------------------------------------------------------------
int vtkMEDQuadraturePointsGenerator::RequestData(
      vtkInformation* request,
      vtkInformationVector **input,
      vtkInformationVector *output)
{
  if (this->Superclass::RequestData(request, input, output) == 0 )
    {
      return 0;
    }    

  //Fill MED internal array
  vtkDataObject *tmpDataObj;
  
  // Get the input.
  tmpDataObj = input[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT());
  vtkUnstructuredGrid *usgIn = vtkUnstructuredGrid::SafeDownCast(tmpDataObj);
  // Get the output.
  tmpDataObj = output->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT());
  vtkPolyData *pdOut = vtkPolyData::SafeDownCast(tmpDataObj);
  vtkDataArray* offsets = this->GetInputArrayToProcess(0, input);

  if (usgIn == NULL || pdOut == NULL || offsets == NULL )
    {
    vtkErrorMacro("Filter data has not been configured correctly. Aborting.");
    return 1;
    }
  
  vtkInformation *info = offsets->GetInformation();
  vtkInformationQuadratureSchemeDefinitionVectorKey *key 
    = vtkQuadratureSchemeDefinition::DICTIONARY();
  if (!key->Has(info))
    {
    vtkErrorMacro(
    << "Dictionary is not present in array "
    << offsets->GetName() << " " << offsets
    << " Aborting.");
    return 0;
    }

  vtkIdType nCells = usgIn->GetNumberOfCells();
  int dictSize = key->Size(info);
  vtkQuadratureSchemeDefinition **dict
    = new vtkQuadratureSchemeDefinition *[dictSize];
  key->GetRange(info, dict, 0, 0, dictSize);  

  // Loop over all fields to map the internal MED cell array to the points array
  int nCArrays = usgIn->GetCellData()->GetNumberOfArrays();
  for (int i = 0; i<nCArrays; ++i)
    {
    vtkDataArray* array = usgIn->GetCellData()->GetArray(i);
    if ( !array )
      {
      continue;
      }
    std::string arrName = array->GetName();
    if ( arrName == MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_CELL_NAME ) 
      {	
        vtkDataArray *out_id_cells = array->NewInstance();
        out_id_cells->SetName(MEDFileFieldRepresentationLeavesArrays::FAMILY_ID_NODE_NAME);
        out_id_cells->SetNumberOfComponents(array->GetNumberOfComponents());
        out_id_cells->CopyComponentNames( array );
        for (int cellId = 0; cellId < nCells; cellId++)
          {
          int cellType = usgIn->GetCellType(cellId);

          // a simple check to see if a scheme really exists for this cell type.
          // should not happen if the cell type has not been modified.
          if (dict[cellType] == NULL)
            {
            continue;
            }

          int np = dict[cellType]->GetNumberOfQuadraturePoints();
          for (int id = 0; id < np; id++)
            {
            out_id_cells->InsertNextTuple(cellId, array);
            }
          }
        out_id_cells->Squeeze();
        pdOut->GetPointData()->AddArray(out_id_cells);
        out_id_cells->Delete();
      }	  
    }
  delete[] dict;  
  return 1;
}
