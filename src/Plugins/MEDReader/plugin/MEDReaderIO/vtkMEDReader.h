// Copyright (C) 2010-2021  CEA/DEN, EDF R&D
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
// Author : Anthony Geay

#ifndef __vtkMEDReader_h_
#define __vtkMEDReader_h_

#include <string>

#include "vtkMultiBlockDataSetAlgorithm.h"
#include "vtkInformationGaussDoubleVectorKey.h"

class vtkDataSet;
class vtkMutableDirectedGraph;
class vtkInformationDataObjectMetaDataKey;
class vtkInformationDoubleVectorKey;
class ExportedTinyInfo;

class VTK_EXPORT vtkMEDReader : public vtkMultiBlockDataSetAlgorithm
{
 private:
  vtkMEDReader(const vtkMEDReader&); // Not implemented.
  void operator=(const vtkMEDReader&); // Not implemented.
 public:
  static vtkMEDReader *New();
  vtkTypeMacro(vtkMEDReader, vtkMultiBlockDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);
  virtual void SetFileName(const char*);
  virtual char *GetFileName();
  virtual const char *GetFileExtensions() { return ".med .rmed"; }
  virtual const char *GetDescriptiveName() { return "MED file (Data Exchange Model)"; }
  //
  virtual void SetFieldsStatus(const char *name, int status);
  virtual int GetNumberOfFieldsTreeArrays();
  virtual const char *GetFieldsTreeArrayName(int index);
  virtual int GetFieldsTreeArrayStatus(const char *name);
  //
  virtual int GetTimesFlagsArrayStatus(const char *name);
  virtual void SetTimesFlagsStatus(const char *name, int status);
  virtual int GetNumberOfTimesFlagsArrays();
  virtual const char *GetTimesFlagsArrayName(int index);
  //! Build the graph used to pass information to the client on the supports
  virtual std::string BuildSIL(vtkMutableDirectedGraph*);

  // Description
  // Reload will delete the internal reader and recreate it with default properties
  virtual void Reload();

  virtual void GenerateVectors(int);
  virtual void ChangeMode(int);
  virtual void GhostCellGeneratorCallForPara(int);
  static const char *GetSeparator();

  // Description
  // Static information key used to transfer the meta data graph along the pipeline
  static vtkInformationDataObjectMetaDataKey* META_DATA();
  static vtkInformationGaussDoubleVectorKey* GAUSS_DATA();

 protected:
  vtkMEDReader();
  virtual ~vtkMEDReader();
  virtual int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
  virtual int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);
 private:
  void UpdateSIL(vtkInformation * request, vtkInformation * info);
  virtual double PublishTimeStepsIfNeeded(vtkInformation*, bool& isUpdated);
  virtual void FillMultiBlockDataSetInstance(vtkMultiBlockDataSet *output, double reqTS, ExportedTinyInfo *internalInfo=0);
  vtkDataSet *RetrieveDataSetAtTime(double reqTS, ExportedTinyInfo *internalInfo);
 private:
  //BTX
  //ETX

  class vtkMEDReaderInternal;
  vtkMEDReaderInternal* Internal;
};

#endif //__vtkMEDReader_h_
