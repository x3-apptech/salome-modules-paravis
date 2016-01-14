//  Copyright (C) 2015 CEA/DEN, EDF R&D, OPEN CASCADE
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
// Author: Roman NIKOLAEV

#include "vtkJSONReader.h"

#include <vtkObjectFactory.h>
#include <vtkTable.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkVariantArray.h>
#include <vtkStringArray.h>
#include <vtkStringToNumeric.h>
#include <vtkSetGet.h>
#include <vtksys/RegularExpression.hxx>

#include <stdexcept>
#include <sstream>

// Key words
#define MD  "_metadata"
#define CMT "_comment"
#define TBN "table_name"
#define TBD "table_description"
#define SHT "short_names"
#define LNG "long_names"
#define UNT "units"
#define DT "date"

#define NA "n/a"

// Exception
//---------------------------------------------------
vtkJSONException::vtkJSONException(const char *reason) : Reason(reason) {

}

//---------------------------------------------------
vtkJSONException::~vtkJSONException() throw () {
}

//---------------------------------------------------
const char* vtkJSONException::what() const throw() {
  return Reason.c_str();
}


//---------------------------------------------------
class Container {
public:
  typedef std::vector<std::pair<std::string,std::vector<double>>> DataType;
  Container(){}
  void initKeys(std::vector<std::string> &keys, std::string &err) {
    for(int i = 0; i < keys.size(); i++) {
      if( !checkVar(keys[i].c_str()) ) {
	std::ostringstream oss;
	oss<<"Bad key value '"<<keys[i].c_str()<<"'";
	err = oss.str();
	return;
      }
      _data.push_back(std::make_pair(keys[i],std::vector<double>()));
    }
  }

  void addValue(std::string key, double value, std::string& err) {
    if( !checkVar(key.c_str()) ) {
      std::ostringstream oss;
      oss<<"Bad key value '"<<key.c_str()<<"'";
      err = oss.str();
      return;
    }
    bool found = false;
    for(int i = 0; i < _data.size(); i++) {
      if(_data[i].first == key) {
	_data[i].second.push_back(value);
	found = true;
	break;
      }
    }
    if(!found) {
      std::ostringstream oss;
      oss<<"Bad key value '"<<key<<"'";
      err = oss.str();
    }
  }
  
  DataType& getData() {
    return _data;
  }
private:
  bool checkVar(const char* var) {
    vtksys::RegularExpression regEx("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return regEx.find(var);
  }
private:
  DataType _data;
};

vtkStandardNewMacro(vtkJSONReader);

//---------------------------------------------------
vtkJSONReader::vtkJSONReader() {
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->FileName = NULL;
}

//---------------------------------------------------
vtkJSONReader::~vtkJSONReader()
{
  this->SetFileName(NULL);
}

//---------------------------------------------------
void vtkJSONReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " 
      << (this->FileName ? this->FileName : "(none)") << endl;
}

//---------------------------------------------------
int vtkJSONReader::RequestData(vtkInformation*, 
			       vtkInformationVector**,
			       vtkInformationVector* outputVector) {
  vtkTable* const output_table = vtkTable::GetData(outputVector);
  
  vtkInformation* const outInfo = outputVector->GetInformationObject(0);
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) &&
     outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0) {
    return 0;
  }
  
  Json::Value root;
  int parseResult = 0;
  parseResult = this->CanParseFile(this->FileName, root);
  if(!parseResult)
    return 0;

  try {
    return this->Parse(root, output_table);
    return 1;
  } 
  catch(vtkJSONException e)  {
    std::ostringstream oss;
    oss<<e.what();
    if(this->HasObserver("ErrorEvent") )
      {
	this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
      }
    else
      {
	vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str())); 
	vtkObject::BreakOnError();
      }
    return 0;
  }
}

//---------------------------------------------------
int vtkJSONReader::CanParseFile(const char *fname, Json::Value &root)
{
  if ( !fname) {
    vtkErrorMacro(<< "Input file name is not specified !!! ");
    return 0;
  }

  ifstream file;
  std::ostringstream oss;
  bool parsedSuccess = true;
  Json::Reader reader;

  file.open(fname);
  if ( !file.is_open() ) {
      oss<< "Unable to open file: " << this->FileName;
      parsedSuccess = false;
  } else {
    parsedSuccess = reader.parse(file, root, false);
    file.close();    
  }
  if ( !parsedSuccess ) {
    if(oss.str().empty()) {
      oss<<"Failed to parse JSON file: " << "\n" << reader.getFormattedErrorMessages();
    }
    if(this->HasObserver("ErrorEvent") ) {
      this->InvokeEvent("ErrorEvent",const_cast<char *>(oss.str().c_str()));
    }
    else {
      vtkOutputWindowDisplayErrorText(const_cast<char *>(oss.str().c_str())); 
      vtkObject::BreakOnError();
    }
    return 0;
  }
  return 1;
}

//---------------------------------------------------
int vtkJSONReader::Parse(Json::Value& root, vtkTable *table) {
  bool hasShortNames = false;
  bool hasUnits = false;
  Container container;


  Json::Value jSONListOfNames;
  Json::Value jSONListOfUnits;
  Json::Value jSONTableName;
  std::vector<std::string> short_names;

  if ( root.isMember(MD) ) {
    Json::Value jSONMetaData = root.get(MD, Json::Value::null);

    if ( jSONMetaData.isMember(CMT) ) {
      Json::Value jSONComment = jSONMetaData.get(CMT, Json::Value::null);
      vtkDebugMacro(<<"Table Comment  : " << jSONComment.asString());
    }
    if ( jSONMetaData.isMember(TBN) ) {
      jSONTableName = jSONMetaData.get(TBN, Json::Value::null);      
      vtkDebugMacro(<<"Table Name  : " << jSONTableName.asString());
    }

    if ( jSONMetaData.isMember(TBD) ) {
      Json::Value jSONTableDescription = jSONMetaData.get(TBD, Json::Value::null);
      vtkDebugMacro(<<"Table Description  : "  << jSONTableDescription.asString());
    }

    if ( jSONMetaData.isMember(DT) ) {
      Json::Value jSONDate = jSONMetaData.get("date", Json::Value::null);
      vtkDebugMacro(<<"Date  : " << jSONDate.asString());
    }

    if ( jSONMetaData.isMember(SHT) ) {
      hasShortNames = true;
      jSONListOfNames = jSONMetaData.get(SHT, Json::Value::null);
      std::ostringstream oss;
      oss<< "Short Names : [ ";
      for (int i = 0; i < jSONListOfNames.size(); i++) {
        oss << "'" << jSONListOfNames[i].asString() << "'";
	short_names.push_back(jSONListOfNames[i].asString());
        if ( i != jSONListOfNames.size() - 1 ) {
          oss << ", ";
        }
      }
      oss << " ]";
      vtkDebugMacro(<<oss.str().c_str());
    }

    Json::Value jSONListOfLongName;
    if ( jSONMetaData.isMember(LNG) ) {
      jSONListOfLongName = jSONMetaData.get(LNG, Json::Value::null);
      std::ostringstream oss;
      oss << "Long Names : [ ";
      for (int i = 0; i < jSONListOfLongName.size(); ++i) {
        oss << "'" << jSONListOfLongName[i].asString() << "'";
        if ( i != jSONListOfLongName.size()-1 ) {
          oss << ", ";
        }
      }
      oss<<" ]";
      vtkDebugMacro(<<oss.str().c_str());
    }
    if ( jSONMetaData.isMember(UNT) ) {
      jSONListOfUnits = jSONMetaData.get(UNT, Json::Value::null);
      hasUnits = true;
      std::ostringstream oss;
      oss << "Units : [ ";
      for (int i = 0; i < jSONListOfUnits.size(); ++i) {
        oss << "'" << jSONListOfUnits[i].asString() << "'";
        if ( i != jSONListOfUnits.size()-1 ) {
          oss << ", ";
        }
      }
      oss << " ]";
      vtkDebugMacro(<<oss.str().c_str());
    }
    root.removeMember(MD);
  }

  Json::Value::iterator it = root.begin();
  Json::Value newRoot = Json::Value::null;
  
  int nbElems=0;
  bool hasArray = false;
  for( ; it != root.end(); it++) {
    nbElems++;
    if((*it).type() == Json::arrayValue) {
      newRoot = (*it);
      hasArray = true;
    }
  }
  if(hasArray && nbElems > 1) {
    throw vtkJSONException("Wrong JSON file: it contains array and others elements");
  }

  if(newRoot == Json::Value::null) {
    newRoot = root;
  }
    
  it = newRoot.begin();
  bool initContainer = false;
  for( ; it != newRoot.end(); it++) {
    if((*it).type() != Json::objectValue) {
      std::ostringstream oss;
      oss<<"Wrong JSON file: unexpected element, named '"<<(it.name())<<"'";
      throw vtkJSONException(oss.str().c_str());
    }
    Json::Value::Members members = (*it).getMemberNames();    
    if(!initContainer) {
      if(!hasShortNames) {
	short_names = members;
      }
      std::string err;
      container.initKeys(short_names,err);
      if(!err.empty()){
	throw vtkJSONException(err.c_str());
      }
      initContainer = true;
    }
    for(int i=0; i < members.size(); i++) { 
      Json::Value val = (*it).get(members[i],Json::Value::null);
      double value = 0.0;
      switch (val.type()) {	
          case Json::stringValue: {
            std::string s("Item with name '");
            s += it.name();
            s += "' has key '";
            s += members[i];
            s += "' with string value '";
            s += val.asString();
            s += "' ! Value set to 0.0";
	    if(this->HasObserver("WarningEvent") ) {
	      this->InvokeEvent("WarningEvent",const_cast<char *>(s.c_str()));
	    }
	    else {
	      vtkOutputWindowDisplayWarningText(const_cast<char *>(s.c_str())); 
	    }
            break;
	  }
          default:
            value = val.asDouble();
      }
      std::string err;
      container.addValue(members[i],value,err);
      if(!err.empty()){
	throw vtkJSONException(err.c_str());
      }
    }
  }
  
  table->GetInformation()->Set(vtkDataObject::FIELD_NAME(), jSONTableName.asString().c_str());
  Container::DataType data = container.getData();
  if(hasUnits && data.size() != jSONListOfUnits.size()) {
    throw vtkJSONException("W");
  }
  
  int nbRows = 0;
  if(data.size() > 0)
    nbRows = data[0].second.size();

  for(int i = 0; i < data.size(); i++) {
    vtkDoubleArray* newCol = vtkDoubleArray::New();
    newCol->SetNumberOfValues(nbRows);
    std::string name = data[i].first;
    name += "[";
    if(!jSONListOfUnits[i].asString().empty()){
      name += jSONListOfUnits[i].asString();
    } else {
      name += NA;
    }
    name += "]";
    newCol->SetName(name.c_str());
    for(int j = 0; j < data[i].second.size(); j++) {
      newCol->SetValue(j, data[i].second[j]);
    }
    table->AddColumn(newCol);
  }
}
