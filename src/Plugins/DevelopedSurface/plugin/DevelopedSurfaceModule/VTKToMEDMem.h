// Copyright (C) 2017-2019  CEA/DEN, EDF R&D
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

#ifndef __VTKTOMEDMEM_HXX__
#define __VTKTOMEDMEM_HXX__

#include "vtkSystemIncludes.h" //needed for exports

#include "MEDCouplingRefCountObject.hxx"
#include "MEDCouplingMemArray.hxx"
#include "MEDCouplingFieldDouble.hxx"
#include "MEDCouplingFieldFloat.hxx"
#include "MEDCouplingFieldInt.hxx"
#include "MEDFileData.hxx"
#include "MEDFileField.hxx"
#include "MEDFileMesh.hxx"
#include "MEDLoaderTraits.hxx"

#include <exception>
#include <string>

///////////////////

class vtkDataSet;

class VTK_EXPORT MZCException : public std::exception
{
public:
  MZCException(const std::string& s):_reason(s) { }
  virtual const char *what() const throw() { return _reason.c_str(); }
  virtual ~MZCException() throw() { }
private:
  std::string _reason;
};

namespace VTKToMEDMem
{
  class VTK_EXPORT Grp
  {
  public:
    Grp(const std::string& name):_name(name) { }
    void setFamilies(const std::vector<std::string>& fams) { _fams=fams; }
    std::string getName() const { return _name; }
    std::vector<std::string> getFamilies() const { return _fams; }
  private:
    std::string _name;
    std::vector<std::string> _fams;
  };

  class VTK_EXPORT Fam
  {
  public:
    Fam(const std::string& name);
    std::string getName() const { return _name; }
    int getID() const { return _id; }
  private:
    std::string _name;
    int _id;
  };
}

class vtkDataObject;

void VTK_EXPORT WriteMEDFileFromVTKDataSet(MEDCoupling::MEDFileData *mfd, vtkDataSet *ds, const std::vector<int>& context, double timeStep, int tsId);

void VTK_EXPORT WriteMEDFileFromVTKGDS(MEDCoupling::MEDFileData *mfd, vtkDataObject *input, double timeStep, int tsId);
  
void VTK_EXPORT PutFamGrpInfoIfAny(MEDCoupling::MEDFileData *mfd, const std::string& meshName, const std::vector<VTKToMEDMem::Grp>& groups, const std::vector<VTKToMEDMem::Fam>& fams);

#endif

