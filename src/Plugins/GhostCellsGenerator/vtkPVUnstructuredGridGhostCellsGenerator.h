/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPVUnstructuredGridGhostCellsGenerator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkPVUnstructuredGridGhostCellsGenerator - Builds ghost cells for a
//  distributed unstructured grid dataset.
//
// .SECTION Description
// This filter generate ghost cells for distributed a unstructured grid in
// parallel - using MPI asynchronous communcations.
// The filter can take benefit of the input grid point global ids to perform.
//
// .SECTION Caveats
//  <ul>
//    <li> A step of 'all reduce' (each processor send/receive data to/from
//         all other processors.
//    <li> The code currently assumes one grid per rank. </li>
//    <li> PointData and CellData must match across partitions/processes. </li>
//  </ul>
//
// .SECTION See Also
// vtkDistributedDataFilter vtkPUnstructuredGridGhostDataGenerator
//
// .SECTION Thanks
// This filter has been developed by Joachim Pouderoux, Kitware SAS 2015.

#ifndef vtkPVUnstructuredGridGhostCellsGenerator_h
#define vtkPVUnstructuredGridGhostCellsGenerator_h

#include "vtkUnstructuredGridAlgorithm.h"

class vtkMultiProcessController;
class vtkUnstructuredGrid;

class vtkPVUnstructuredGridGhostCellsGenerator:
  public vtkUnstructuredGridAlgorithm
{
  vtkTypeMacro(vtkPVUnstructuredGridGhostCellsGenerator, vtkUnstructuredGridAlgorithm);

public:
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkPVUnstructuredGridGhostCellsGenerator *New();

  // Description:
  // Set/Get the MPI multi process controller object.
  void SetController(vtkMultiProcessController *c);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);

  // Description:
  // Specify if the filter must take benefit of global point ids if they exist.
  // If false, point coordinates are used. Default is TRUE.
  vtkSetMacro(UseGlobalPointIds, bool);
  vtkGetMacro(UseGlobalPointIds, bool);
  vtkBooleanMacro(UseGlobalPointIds, bool);

  // Description:
  // Specify the name of the global point ids data array if the GlobalIds
  // attribute array is not set. Default is "GlobalNodeIds".
  vtkSetStringMacro(GlobalPointIdsArrayName);
  vtkGetStringMacro(GlobalPointIdsArrayName);

protected:
  vtkPVUnstructuredGridGhostCellsGenerator();
  ~vtkPVUnstructuredGridGhostCellsGenerator();

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

  vtkMultiProcessController *Controller;
  char* GlobalPointIdsArrayName;
  bool UseGlobalPointIds;

private:

  vtkPVUnstructuredGridGhostCellsGenerator(const vtkPVUnstructuredGridGhostCellsGenerator&); // Not implemented
  void operator=(const vtkPVUnstructuredGridGhostCellsGenerator&); // Not implemented
};

#endif
