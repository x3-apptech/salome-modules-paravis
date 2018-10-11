/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticPUnstructuredGridGhostCellsGenerator.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkStaticPUnstructuredGridGhostCellsGenerator
 * @brief   StaticMesh aware GhostCellGenerator implementation
 *
 * This class specialize vtkPUnstructuredGridGhostCellGenerator
 * This class improves it by making it static mesh aware.
 * The first time this filter is executed it will store its output
 * in a cache as well as a list of ghost and point ids to request from other rank
 * On next execution, if the mesh is static, it will uses the list of ids to request
 * only point and cell data for the ghost point and cell from other
 * allowing to update the output without needing to recompute everything
 *
 * @sa
 * vtkPUnstructuredGridGhostCellsGenerator
*/

#ifndef vtkStaticPUnstructuredGridGhostCellsGenerator_h
#define vtkStaticPUnstructuredGridGhostCellsGenerator_h

#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkPUnstructuredGridGhostCellsGenerator.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkUnstructuredGrid;

class vtkStaticPUnstructuredGridGhostCellsGenerator : public vtkPUnstructuredGridGhostCellsGenerator
{
public:
  static vtkStaticPUnstructuredGridGhostCellsGenerator* New();
  typedef vtkPUnstructuredGridGhostCellsGenerator
    Superclass; // vtkTypeMacro can't be used with a factory built object
  void PrintSelf(ostream& os, vtkIndent indent) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

protected:
  vtkStaticPUnstructuredGridGhostCellsGenerator();
  ~vtkStaticPUnstructuredGridGhostCellsGenerator() override;

  /**
   * Check input for "ProcessId" and "Ids" point and cell array,
   * If absent, compute and add them.
   */
  static void AddIdsArrays(vtkDataSet* input, vtkDataSet* output);

  /**
   * Using the Cache, exchange and update ghost point and cell
   * ids between ranks
   */
  virtual void ProcessGhostIds();

  /**
   * Update cache with input and with ghost cells info
   */
  virtual void UpdateCacheData(vtkDataSet* input);

  /**
   * Copy input point, cell and field data into cache
   */
  virtual void UpdateCacheDataWithInput(vtkDataSet* input);

  /**
   * Using Cached ghost cell and points info
   * Update ghost cell and point data in cache
   * by sending input point and cell data to other ranks
   */
  virtual void UpdateCacheGhostCellAndPointData(vtkDataSet* input);

  vtkNew<vtkUnstructuredGrid> Cache;
  vtkMTimeType InputMeshTime;
  vtkMTimeType FilterMTime;

  std::vector<vtkSmartPointer<vtkIdList> > GhostCellsToReceive;
  std::vector<vtkSmartPointer<vtkIdList> > GhostCellsToSend;
  std::vector<vtkSmartPointer<vtkIdList> > GhostPointsToReceive;
  std::vector<vtkSmartPointer<vtkIdList> > GhostPointsToSend;

private:
  // Hide these from the user and the compiler.
  vtkStaticPUnstructuredGridGhostCellsGenerator(
    const vtkStaticPUnstructuredGridGhostCellsGenerator&) = delete;
  void operator=(const vtkStaticPUnstructuredGridGhostCellsGenerator&) = delete;
};

#endif
