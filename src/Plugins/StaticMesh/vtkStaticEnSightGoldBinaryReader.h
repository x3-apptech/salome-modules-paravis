/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkStaticEnSightGoldBinaryReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkStaticEnSightGoldBinaryReader
 * @brief   class to read binary EnSight6 files
 *
 * vtkStaticEnSightGoldBinaryReader is a class to read binary EnSight6 files into vtk.
 * Because the different parts of the EnSight data can be of various data
 * types, this reader produces multiple outputs, one per part in the input
 * file.
 * All variable information is being stored in field data.  The descriptions
 * listed in the case file are used as the array names in the field data.
 * For complex vector variables, the description is appended with _r (for the
 * array of real values) and _i (for the array if imaginary values).  Complex
 * scalar variables are stored as a single array with 2 components, real and
 * imaginary, listed in that order.
 * @warning
 * You must manually call Update on this reader and then connect the rest
 * of the pipeline because (due to the nature of the file format) it is
 * not possible to know ahead of time how many outputs you will have or
 * what types they will be.
 * This reader can only handle static EnSight datasets (both static geometry
 * and variables).
*/

#ifndef vtkStaticEnSightGoldBinaryReader_h
#define vtkStaticEnSightGoldBinaryReader_h

#include "vtkEnSightGoldBinaryReader.h"
#include "vtkNew.h"

class vtkMultiBlockDataSet;

class vtkStaticEnSightGoldBinaryReader : public vtkEnSightGoldBinaryReader
{
public:
  static vtkStaticEnSightGoldBinaryReader *New();
  vtkTypeMacro(vtkStaticEnSightGoldBinaryReader, vtkEnSightGoldBinaryReader);

protected:
  vtkStaticEnSightGoldBinaryReader() = default;
  ~vtkStaticEnSightGoldBinaryReader() override = default;

  int RequestData(vtkInformation*,
                  vtkInformationVector**,
                  vtkInformationVector*) override;

  vtkNew<vtkMultiBlockDataSet> Cache;
  vtkTimeStamp CacheMTime;

private:
  vtkStaticEnSightGoldBinaryReader(const vtkStaticEnSightGoldBinaryReader&) = delete;
  void operator=(const vtkStaticEnSightGoldBinaryReader&) = delete;
};

#endif

