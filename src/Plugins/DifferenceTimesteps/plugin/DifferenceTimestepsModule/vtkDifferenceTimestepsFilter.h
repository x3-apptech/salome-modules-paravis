// Copyright (C) 2014-2020  CEA/DEN, EDF R&D
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
// Author : Maxim Glibin

#ifndef __DifferenceTimestepsFilter_h_
#define __DifferenceTimestepsFilter_h_

#include <vtkMultiTimeStepAlgorithm.h>

#include <vector>

class vtkDataSet;
class vtkStringArray;

/**
 * Description of class:
 * Class allows to compute difference between two time steps of one data array (field).
 */
class VTK_EXPORT vtkDifferenceTimestepsFilter : public vtkMultiTimeStepAlgorithm
{
public:
  /// Returns pointer on a new instance of the class
  static vtkDifferenceTimestepsFilter* New();

  vtkTypeMacro(vtkDifferenceTimestepsFilter, vtkMultiTimeStepAlgorithm);

  /// Prints current state of the objects
  void PrintSelf(ostream&, vtkIndent) override;

  // Description:
  // Set/Get methods for first time step.
  vtkSetMacro(FirstTimeStepIndex, int);
  vtkGetMacro(FirstTimeStepIndex, int);

  // Description:
  // Set/Get methods for first time step.
  vtkSetMacro(SecondTimeStepIndex, int);
  vtkGetMacro(SecondTimeStepIndex, int);

  // Description:
  // Get methods for range of indices of time steps.
  vtkGetVector2Macro(RangeIndicesTimeSteps, int);

  // Description:
  // Set/Get methods for prefix of array name.
  vtkSetStringMacro(ArrayNamePrefix);
  vtkGetStringMacro(ArrayNamePrefix);

protected:
  /// Constructor & destructor
  vtkDifferenceTimestepsFilter();
  ~vtkDifferenceTimestepsFilter() override;

  /// The methods which is called on filtering data
  int FillInputPortInformation(int, vtkInformation*) override;

  int FillOutputPortInformation(int, vtkInformation*) override;

  int RequestDataObject(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int RequestUpdateExtent(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  // Description:
  // General computation differences routine for any type on input data. This
  // is called recursively when heirarchical/multiblock data is encountered.
  vtkDataObject* DifferenceDataObject(vtkDataObject* theInput1, vtkDataObject* theInput2);

  // Description:
  // Root level interpolation for a concrete dataset object.
  // Point/Cell data and points are different.
  // Needs improving if connectivity is to be handled.
  virtual vtkDataSet* DifferenceDataSet(vtkDataSet* theInput1, vtkDataSet* theInput2);

  // Description:
  // Compute difference a single vtkDataArray. Called from computation
  // of the difference routine on pointdata or celldata.
  virtual vtkDataArray* DifferenceDataArray(vtkDataArray** theArrays, vtkIdType theN);

  // Description:
  // Range of indices of the time steps.
  int RangeIndicesTimeSteps[2];

  // Description:
  // First time step index.
  int FirstTimeStepIndex;

  // Description:
  // Second time step index.
  int SecondTimeStepIndex;

  // Description:
  // Length of time steps array
  int NumberTimeSteps;

  // Description:
  // Array of time step values.
  std::vector<double> TimeStepValues;

  // Description:
  // Prefix of array name.
  char* ArrayNamePrefix;

private:
  vtkDifferenceTimestepsFilter(const vtkDifferenceTimestepsFilter&) = delete;
  void operator=(const vtkDifferenceTimestepsFilter&) = delete;

  // Description:
  // Get field association type.
  int GetInputFieldAssociation();

  // Description:
  // Called just before computation of the difference of the dataset to ensure that
  // each data array has the same array name, number of tuples or components and etc.
  bool VerifyArrays(vtkDataArray** theArrays, int theNumArrays);
};

#endif // __DifferenceTimestepsFilter_h_
