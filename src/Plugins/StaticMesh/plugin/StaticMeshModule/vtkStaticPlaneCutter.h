/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticPlaneCutter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkStaticPlaneCutter
 * @brief   StaticMesh aware implementation of vtkPlaneCutter vtk vtkUnstructuredGrid
 *
 * This class specialize vtkPlaneCutter for vtkUnstructuredGrid input.
 * It uses a cache when the associated data chage over time but not the geometry.
 * In order to be able to update the cache, we keep track of cells ids
 * when the cache is computed.
 * Contrary to its parent, this class does not interpolate point data,
 * only transmit cell data.
 *
 *
 * @sa
 * vtkPlaneCutter
*/

#ifndef vtkStaticPlaneCutter_h
#define vtkStaticPlaneCutter_h

#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkPlaneCutter.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkMultiPieceDataSet;

class vtkStaticPlaneCutter : public vtkPlaneCutter
{
public:
  static vtkStaticPlaneCutter* New();
  typedef vtkPlaneCutter Superclass; // vtkTypeMacro can't be used with a factory built object
  void PrintSelf(ostream &os, vtkIndent indent) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

protected:
  vtkStaticPlaneCutter();
  ~vtkStaticPlaneCutter() override;

  /**
   * Check input for Ids cell array, if absent, compute and add them.
   */
  static void AddIdsArray(vtkDataSet* input, vtkDataSet* output);

  /**
   * Remove an Ids cell array in all polydata pieces of output
   */
  static void RemoveIdsArray(vtkMultiPieceDataSet* output);

  /**
   * Update cache point, cell and field data using input
   */
  void UpdateCacheData(vtkDataSet* input);

  /**
   * Compute the ids and weights to be used when updating the cache later
   */
  void ComputeIds(vtkUnstructuredGrid* input);

  void ClearWeightsVector();

  vtkNew<vtkMultiPieceDataSet> Cache;
  std::vector<vtkSmartPointer<vtkIdList> > CellToCopyFrom;
  std::vector<vtkSmartPointer<vtkIdList> > CellToCopyTo;
  std::vector<std::vector<std::pair<vtkSmartPointer<vtkIdList>, double*>>> WeightsVectorCompo;
  vtkMTimeType InputMeshTime;
  vtkMTimeType FilterMTime;

private:
  // Hide these from the user and the compiler.
  vtkStaticPlaneCutter(const vtkStaticPlaneCutter&) = delete;
  void operator=(const vtkStaticPlaneCutter&) = delete;
};

#endif
