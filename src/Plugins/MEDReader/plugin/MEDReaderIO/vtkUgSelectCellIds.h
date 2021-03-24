// Copyright (C) 2020-2021  CEA/DEN, EDF R&D
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

#pragma once

#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkIdTypeArray.h>

/*!
 * Class taking only specified cellIds of input vtkUnstructuredGrid dataset
 * For performance reasons orphan nodes are not removed here.
 * The output vtkUnstructuredGrid is lying on the same points than input one.
 */
class vtkUgSelectCellIds : public vtkUnstructuredGridAlgorithm
{
public:
    static vtkUgSelectCellIds* New();
    vtkTypeMacro(vtkUgSelectCellIds, vtkUnstructuredGridAlgorithm)
    void SetIds(vtkIdTypeArray *ids);
    vtkUgSelectCellIds() = default;
    ~vtkUgSelectCellIds() override = default;
protected:
    int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
private:
    vtkSmartPointer<vtkIdTypeArray> _ids;
};
