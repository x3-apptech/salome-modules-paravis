/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTemporalUGWavelet.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkTemporalUGWavelet
 * @brief   Create a Temporal Unstructured Grid Wavelet with a static mesh
 *
 * vtkTemporalUGWavelet specialize vtkRTAnalyticSource to create
 * a wavelet converted to vtkUnstructuredGrid, with timesteps.
 * The "tPoint" and "tCell" arrays are the only data actually changing over time
 * make the output a static mesh with data evolving over time.
*/

#ifndef vtkTemporalUGWavelet_h
#define vtkTemporalUGWavelet_h

#include "vtkRTAnalyticSource.h"

class vtkUnstructuredGrid;

class vtkTemporalUGWavelet : public vtkRTAnalyticSource
{
public:
  static vtkTemporalUGWavelet* New();
  vtkTypeMacro(vtkTemporalUGWavelet, vtkRTAnalyticSource);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  //@{
  /**
   * Set/Get the number of time steps. Initial value is 10.
   */
  vtkSetMacro(NumberOfTimeSteps, int);
  vtkGetMacro(NumberOfTimeSteps, int);
  //@}

protected:
  vtkTemporalUGWavelet();
  ~vtkTemporalUGWavelet();

  int FillOutputPortInformation(int vtkNotUsed(port), vtkInformation* info) VTK_OVERRIDE;

  int RequestInformation(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) VTK_OVERRIDE;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) VTK_OVERRIDE;

  int NumberOfTimeSteps;
  vtkUnstructuredGrid* Cache;
  vtkTimeStamp CacheMTime;

private:
  vtkTemporalUGWavelet(const vtkTemporalUGWavelet&) VTK_DELETE_FUNCTION;
  void operator=(const vtkTemporalUGWavelet&) VTK_DELETE_FUNCTION;
};

#endif
