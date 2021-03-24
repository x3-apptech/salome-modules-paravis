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

#include <sstream>
#include <vector>
#include <set>
#include <map>

#include "MEDLoaderForPV.h"

class MEDLOADERFORPV_EXPORT ExtractGroupStatus
{
public:
  ExtractGroupStatus():_status(false) { }
  ExtractGroupStatus(const char *name);
  bool getStatus() const { return _status; }
  void setStatus(bool status) const { _status=status; }
  void cpyStatusFrom(const ExtractGroupStatus& other) { _status=other._status; }
  std::string getName() const { return _name; }
  void resetStatus() const { _status=false; }
  const char *getKeyOfEntry() const { return _ze_key_name.c_str(); }
  virtual void printMySelf(std::ostream& os) const;
  virtual bool isSameAs(const ExtractGroupStatus& other) const;
protected:
mutable bool _status;
std::string _name;
std::string _ze_key_name;
};

class MEDLOADERFORPV_EXPORT ExtractGroupGrp : public ExtractGroupStatus
{
public:
  ExtractGroupGrp(const char *name):ExtractGroupStatus(name) { std::ostringstream oss; oss << START << name; _ze_key_name=oss.str(); }
  void setFamilies(const std::vector<std::string>& fams) { _fams=fams; }
  const std::vector<std::string>& getFamiliesLyingOn() const { return _fams; }
  bool isSameAs(const ExtractGroupGrp& other) const;
public:
  static const char START[];
  std::vector<std::string> _fams;
};

class MEDLOADERFORPV_EXPORT ExtractGroupFam : public ExtractGroupStatus
{
public:
  ExtractGroupFam(const char *name);
  void printMySelf(std::ostream& os) const;
  void fillIdsToKeep(std::set<int>& s) const;
  int getId() const { return _id; }
  bool isSameAs(const ExtractGroupFam& other) const;
public:
  static const char START[];
private:
  int _id;
};

class vtkInformationDataObjectMetaDataKey;
class vtkMutableDirectedGraph;
class vtkInformation;

class MEDLOADERFORPV_EXPORT ExtractGroupInternal
{
public:
  void loadFrom(vtkMutableDirectedGraph *sil);
  int getNumberOfEntries() const;
  const char *getMeshName() const;
  const char *getKeyOfEntry(int i) const;
  bool getStatusOfEntryStr(const char *entry) const;
  void setStatusOfEntryStr(const char *entry, bool status);
  void printMySelf(std::ostream& os) const;
  std::set<int> getIdsToKeep() const;
  std::vector< std::pair<std::string,std::vector<int> > > getAllGroups() const;
  void clearSelection() const;
  int getIdOfFamily(const std::string& famName) const;
  static bool IndependantIsInformationOK(vtkInformationDataObjectMetaDataKey *medReaderMetaData, vtkInformation *info);
private:
  std::map<std::string,int> computeFamStrIdMap() const;
  const ExtractGroupStatus& getEntry(const char *entry) const;
  ExtractGroupStatus& getEntry(const char *entry);
private:
  std::vector<ExtractGroupGrp> _groups;
  std::vector<ExtractGroupFam> _fams;
  mutable std::vector< std::pair<std::string,bool> > _selection;
  std::string _mesh_name;
};

