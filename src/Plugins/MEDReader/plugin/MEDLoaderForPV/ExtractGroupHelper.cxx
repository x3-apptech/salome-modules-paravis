// Copyright (C) 2020  CEA/DEN, EDF R&D
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

#include "ExtractGroupHelper.h"
#include "MEDFileFieldRepresentationTree.hxx"

#include "vtkInformation.h"
#include "vtkInformationDataObjectMetaDataKey.h"
#include "vtkAdjacentVertexIterator.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkDataSetAttributes.h"
#include "vtkStringArray.h"

#include <cstring>

const char ExtractGroupGrp::START[]="GRP_";

const char ExtractGroupFam::START[]="FAM_";

ExtractGroupStatus::ExtractGroupStatus(const char *name):_status(false),_name(name)
{
}

void ExtractGroupStatus::printMySelf(std::ostream& os) const
{
  os << "      -" << _ze_key_name << "(";
  if(_status)
    os << "X";
  else
    os << " ";
  os << ")" << std::endl;
}

bool ExtractGroupStatus::isSameAs(const ExtractGroupStatus& other) const
{
  return _name==other._name && _ze_key_name==other._ze_key_name;
}

bool ExtractGroupGrp::isSameAs(const ExtractGroupGrp& other) const
{
  bool ret(ExtractGroupStatus::isSameAs(other));
  if(ret)
    return _fams==other._fams;
  else
    return false;
}

ExtractGroupFam::ExtractGroupFam(const char *name):ExtractGroupStatus(name),_id(0)
{
  std::size_t pos(_name.find(MEDFileFieldRepresentationLeavesArrays::ZE_SEP));
  std::string name0(_name.substr(0,pos)),name1(_name.substr(pos+strlen(MEDFileFieldRepresentationLeavesArrays::ZE_SEP)));
  std::istringstream iss(name1);
  iss >> _id;
  std::ostringstream oss; oss << START << name; _ze_key_name=oss.str(); _name=name0;
}

bool ExtractGroupFam::isSameAs(const ExtractGroupFam& other) const
{
  bool ret(ExtractGroupStatus::isSameAs(other));
  if(ret)
    return _id==other._id;
  else
    return false;
}

void ExtractGroupFam::printMySelf(std::ostream& os) const
{
  os << "      -" << _ze_key_name << " famName : \"" << _name << "\" id : " << _id << " (";
  if(_status)
    os << "X";
  else
    os << " ";
  os << ")" << std::endl;
}

void ExtractGroupFam::fillIdsToKeep(std::set<int>& s) const
{
  s.insert(_id);
}
///////////////////

bool ExtractGroupInternal::IndependantIsInformationOK(vtkInformationDataObjectMetaDataKey *medReaderMetaData, vtkInformation *info)
{
  // Check the information contain meta data key
  if(!info->Has(medReaderMetaData))
    return false;

  // Recover Meta Data
  vtkMutableDirectedGraph *sil(vtkMutableDirectedGraph::SafeDownCast(info->Get(medReaderMetaData)));
  if(!sil)
    return false;
  int idNames(0);
  vtkAbstractArray *verticesNames(sil->GetVertexData()->GetAbstractArray("Names",idNames));
  vtkStringArray *verticesNames2(vtkStringArray::SafeDownCast(verticesNames));
  if(!verticesNames2)
    return false;
  for(int i=0;i<verticesNames2->GetNumberOfValues();i++)
    {
      vtkStdString &st(verticesNames2->GetValue(i));
      if(st=="MeshesFamsGrps")
        return true;
    }
  return false;
}

const char *ExtractGroupInternal::getMeshName() const
{
  return this->_mesh_name.c_str();
}

void ExtractGroupInternal::loadFrom(vtkMutableDirectedGraph *sil)
{
  std::vector<ExtractGroupGrp> oldGrps(_groups); _groups.clear();
  std::vector<ExtractGroupFam> oldFams(_fams); _fams.clear();
  int idNames(0);
  vtkAbstractArray *verticesNames(sil->GetVertexData()->GetAbstractArray("Names",idNames));
  vtkStringArray *verticesNames2(vtkStringArray::SafeDownCast(verticesNames));
  vtkIdType id0;
  bool found(false);
  for(int i=0;i<verticesNames2->GetNumberOfValues();i++)
    {
      vtkStdString &st(verticesNames2->GetValue(i));
      if(st=="MeshesFamsGrps")
        {
          id0=i;
          found=true;
        }
    }
  if(!found)
    throw INTERP_KERNEL::Exception("There is an internal error ! The tree on server side has not the expected look !");
  vtkAdjacentVertexIterator *it0(vtkAdjacentVertexIterator::New());
  sil->GetAdjacentVertices(id0,it0);
  int kk(0),ll(0);
  while(it0->HasNext())
    {
      vtkIdType id1(it0->Next());
      std::string meshName((const char *)verticesNames2->GetValue(id1));
      this->_mesh_name=meshName;
      vtkAdjacentVertexIterator *it1(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(id1,it1);
      vtkIdType idZeGrps(it1->Next());//zeGroups
      vtkAdjacentVertexIterator *itGrps(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(idZeGrps,itGrps);
      while(itGrps->HasNext())
        {
          vtkIdType idg(itGrps->Next());
          ExtractGroupGrp grp((const char *)verticesNames2->GetValue(idg));
          vtkAdjacentVertexIterator *itGrps2(vtkAdjacentVertexIterator::New());
          sil->GetAdjacentVertices(idg,itGrps2);
          std::vector<std::string> famsOnGroup;
          while(itGrps2->HasNext())
            {
              vtkIdType idgf(itGrps2->Next());
              famsOnGroup.push_back(std::string((const char *)verticesNames2->GetValue(idgf)));
            }
          grp.setFamilies(famsOnGroup);
          itGrps2->Delete();
          _groups.push_back(grp);
        }
      itGrps->Delete();
      vtkIdType idZeFams(it1->Next());//zeFams
      it1->Delete();
      vtkAdjacentVertexIterator *itFams(vtkAdjacentVertexIterator::New());
      sil->GetAdjacentVertices(idZeFams,itFams);
      while(itFams->HasNext())
        {
          vtkIdType idf(itFams->Next());
          ExtractGroupFam fam((const char *)verticesNames2->GetValue(idf));
          _fams.push_back(fam);
        }
      itFams->Delete();
    }
  it0->Delete();
  //
  std::size_t szg(_groups.size()),szf(_fams.size());
  if(szg==oldGrps.size() && szf==oldFams.size())
    {
      bool isSame(true);
      for(std::size_t i=0;i<szg && isSame;i++)
        isSame=_groups[i].isSameAs(oldGrps[i]);
      for(std::size_t i=0;i<szf && isSame;i++)
        isSame=_fams[i].isSameAs(oldFams[i]);
      if(isSame)
        {
          for(std::size_t i=0;i<szg;i++)
            _groups[i].cpyStatusFrom(oldGrps[i]);
          for(std::size_t i=0;i<szf;i++)
            _fams[i].cpyStatusFrom(oldFams[i]);
        }
    }
}

int ExtractGroupInternal::getNumberOfEntries() const
{
  std::size_t sz0(_groups.size()),sz1(_fams.size());
  return (int)(sz0+sz1);
}

const char *ExtractGroupInternal::getKeyOfEntry(int i) const
{
  int sz0((int)_groups.size());
  if(i>=0 && i<sz0)
    return _groups[i].getKeyOfEntry();
  else
    return _fams[i-sz0].getKeyOfEntry();
}

bool ExtractGroupInternal::getStatusOfEntryStr(const char *entry) const
{
  const ExtractGroupStatus& elt(getEntry(entry));
  return elt.getStatus();
}

void ExtractGroupInternal::setStatusOfEntryStr(const char *entry, bool status)
{
  _selection.emplace_back(entry,status);
}

const ExtractGroupStatus& ExtractGroupInternal::getEntry(const char *entry) const
{
  std::string entryCpp(entry);
  for(std::vector<ExtractGroupGrp>::const_iterator it0=_groups.begin();it0!=_groups.end();it0++)
    if(entryCpp==(*it0).getKeyOfEntry())
      return *it0;
  for(std::vector<ExtractGroupFam>::const_iterator it0=_fams.begin();it0!=_fams.end();it0++)
    if(entryCpp==(*it0).getKeyOfEntry())
      return *it0;
  std::ostringstream oss; oss << "vtkExtractGroupInternal::getEntry : no such entry \"" << entry << "\"!";
  throw INTERP_KERNEL::Exception(oss.str().c_str());
}

ExtractGroupStatus& ExtractGroupInternal::getEntry(const char *entry)
{
  std::string entryCpp(entry);
  for(std::vector<ExtractGroupGrp>::iterator it0=_groups.begin();it0!=_groups.end();it0++)
    if(entryCpp==(*it0).getKeyOfEntry())
      return *it0;
  for(std::vector<ExtractGroupFam>::iterator it0=_fams.begin();it0!=_fams.end();it0++)
    if(entryCpp==(*it0).getKeyOfEntry())
      return *it0;
  std::ostringstream oss; oss << "vtkExtractGroupInternal::getEntry : no such entry \"" << entry << "\"!";
  throw INTERP_KERNEL::Exception(oss.str().c_str());
}

void ExtractGroupInternal::printMySelf(std::ostream& os) const
{
  os << "Groups :" << std::endl;
  for(std::vector<ExtractGroupGrp>::const_iterator it0=_groups.begin();it0!=_groups.end();it0++)
    (*it0).printMySelf(os);
  os << "Families :" << std::endl;
  for(std::vector<ExtractGroupFam>::const_iterator it0=_fams.begin();it0!=_fams.end();it0++)
    (*it0).printMySelf(os);
}

int ExtractGroupInternal::getIdOfFamily(const std::string& famName) const
{
  for(std::vector<ExtractGroupFam>::const_iterator it=_fams.begin();it!=_fams.end();it++)
    {
      if((*it).getName()==famName)
        return (*it).getId();
    }
  return std::numeric_limits<int>::max();
}

std::set<int> ExtractGroupInternal::getIdsToKeep() const
{
  for(auto it: _selection)
    {
      const ExtractGroupStatus& elt(getEntry(it.first.c_str()));
      elt.setStatus(it.second);
    }
  std::map<std::string,int> m(this->computeFamStrIdMap());
  std::set<int> s;
  for(std::vector<ExtractGroupGrp>::const_iterator it0=_groups.begin();it0!=_groups.end();it0++)
    {
      if((*it0).getStatus())
        {
          const std::vector<std::string>& fams((*it0).getFamiliesLyingOn());
          for(std::vector<std::string>::const_iterator it1=fams.begin();it1!=fams.end();it1++)
            {
              std::map<std::string,int>::iterator it2(m.find((*it1)));
              if(it2!=m.end())
                s.insert((*it2).second);
            }
        }
     }
  for(std::vector<ExtractGroupFam>::const_iterator it0=_fams.begin();it0!=_fams.end();it0++)
    if((*it0).getStatus())
      (*it0).fillIdsToKeep(s);
  return s;
}

// see reference : https://en.cppreference.com/w/cpp/iterator/iterator
class FamilyIterator : public std::iterator< std::input_iterator_tag, long, long, int*, int >
{
  long _num = 0;
  const ExtractGroupInternal *_egi = nullptr;
  const std::vector<std::string> *_fams = nullptr;
public:
  explicit FamilyIterator(long num , const ExtractGroupInternal *egi, const std::vector<std::string>& fams) : _num(num),_egi(egi),_fams(&fams) {}
  FamilyIterator& operator++() { ++_num; return *this;}
  bool operator==(const FamilyIterator& other) const {return _num == other._num;}
  bool operator!=(const FamilyIterator& other) const {return !(*this == other);}
  reference operator*() const {return _egi->getIdOfFamily((*_fams)[_num]);}
};

std::vector< std::pair<std::string,std::vector<int> > > ExtractGroupInternal::getAllGroups() const
{
    std::vector< std::pair<std::string,std::vector<int> > > ret;
    for(const auto&  grp : _groups)
    {
        const std::vector<std::string>& fams(grp.getFamiliesLyingOn());
        std::vector<int> famIds(FamilyIterator(0,this,fams),FamilyIterator(fams.size(),this,fams));
        std::pair<std::string,std::vector<int> > elt(grp.getName(),std::move(famIds));
        ret.emplace_back(std::move(elt));
    }
    return ret;
}

void ExtractGroupInternal::clearSelection() const
{
  _selection.clear();
  for(auto it : _groups)
    it.resetStatus();
  for(auto it : _fams)
    it.resetStatus();
}

std::map<std::string,int> ExtractGroupInternal::computeFamStrIdMap() const
{
  std::map<std::string,int> ret;
  for(std::vector<ExtractGroupFam>::const_iterator it0=_fams.begin();it0!=_fams.end();it0++)
    ret[(*it0).getName()]=(*it0).getId();
  return ret;
}
