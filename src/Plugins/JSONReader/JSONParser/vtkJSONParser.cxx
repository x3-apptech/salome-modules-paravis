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

#include "vtkJSONParser.h"

#include <vtkObjectFactory.h>
#include <vtkTable.h>
#include <vtkInformation.h>
#include <vtkDoubleArray.h>

#include <algorithm>
#include <sstream>
#include <list>
#include <map>

//DEBUG macro
//#define __DEBUG


// Key worlds
#define MD  "_metadata"
#define CMT "_comment"
#define TBN "table_name"
#define TBD "table_description"
#define SHT "short_names"
#define LNG "long_names"
#define UNT "units"
#define DT "date"

// Separators
#define COMMA ','

#define COLON ':'

#define OSB '['
#define CSB ']'

#define OCB '{'
#define CCB '}'
#define ENDL '\n'
#define QTS  '"'
#define SPS ' '
#define TAB  '\t'

#define NA   "n/a"


#define addInfo(info) \
  Info I; \
  I.type = info; \
  I.ln = this->LineNumber; \
  I.cn = this->ColumnNumber; \
  I.pos = ftell(this->File); \
  this->CInfoVector.push_back(I);

//---------------------------------------------------
bool isBlankOrEnd(const char c) {
  return c ==  SPS || c == ENDL || c == TAB || c == EOF;
}

bool isDigitOrDot(const char c) {
  return (c >= '.' && c <='9') || c == '+' || c == '-' || c == 'e';
}

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

// Containers to store information about nodes:

// Base node
//---------------------------------------------------
class vtkJSONNode {
public:
  vtkJSONNode() { Name = 0; }
  virtual ~vtkJSONNode() { }
  const char* GetName() { return this->Name ; }
  void SetName(const char* name) { this->Name  = name; }
private:
  const char* Name;
};

class vtkJSONMetaNode : public vtkJSONNode {
public:  
  vtkJSONMetaNode() : vtkJSONNode() {
    this->SetName(MD);
  this->Comment  = 0;
  this->TableName = 0;
  this->TableDescription = 0;
  this->Date = 0;
  }

  virtual ~vtkJSONMetaNode() {
    delete this->Comment; this->Comment = 0;
    delete this->TableName; this->TableName = 0;
    delete this->Date; this->Date= 0;
    delete this->TableDescription; this->TableDescription = 0;
    for(int i = 0; i < this->ShortNames.size(); i++) {
      delete this->ShortNames[i]; this->ShortNames[i] = 0;
    }
    for(int i = 0; i < this->LongNames.size(); i++) {
      delete this->LongNames[i]; this->LongNames[i] = 0;
    }
    for(int i = 0; i < this->Units.size(); i++) {
      delete this->Units[i]; this->Units[i] = 0;
    }
  }

  void SetComment(const char* comment) { this->Comment = comment; }
  const char* GetComment() { return this->Comment; }

  void SetTableName(const char* name) { this->TableName = name; }
  const char* GetTableName() { return this->TableName; }

  void SetDate(const char* name) { this->Date = name; }
  const char* GetDate() { return this->Date; }

  void SetTableDescription(const char* description) { this->TableDescription = description; }
  const char* GetDescription() { return this->TableDescription; }

  void SetShortNames(std::vector<const char*> names) { this->ShortNames = names; }
  std::vector<const char*> GetShortNames() { return this->ShortNames; }

  void SetLongNames(std::vector<const char*> names) { this->LongNames = names; }
  std::vector<const char*> GetLongNames() { return this->LongNames; }

  void SetUnits(std::vector<const char*> units) { this->Units = units; }
  std::vector<const char*> GetUnits() { return this->Units; }

private:
  const char* Comment;
  const char* TableName;
  const char* TableDescription;
  const char* Date;
  std::vector<const char*> ShortNames;
  std::vector<const char*> LongNames;
  std::vector<const char*> Units;
};

struct char_cmp { 
  bool operator () (const char *a, const char *b) const 
  {
    return strcmp(a,b)<0;
  } 
};

typedef std::map<const char*, double, char_cmp> vtkJSONMap;

class vtkJSONInfoNode : public vtkJSONNode {
public:  
  vtkJSONInfoNode(const char* name) : vtkJSONNode() {
    this->SetName(name);
  }
  virtual ~vtkJSONInfoNode() {
    vtkJSONMap::iterator it = this->Values.begin();
    for( ;it != this->Values.end(); it++) {
      delete it->first;
      const char* c = it->first;
      c = 0;
    }
  }
  void AddValue(const char* key, const double value) {
    this->Values.insert(std::pair<const char*,double>(key,value)); 
  }
  vtkJSONMap& GetValues() { return this->Values; }
private:  
  vtkJSONMap Values;
};


//---------------------------------------------------
vtkStandardNewMacro(vtkJSONParser);

//---------------------------------------------------
vtkJSONParser::vtkJSONParser()
{
  this->FileName = NULL;
  this->LineNumber = 1;
  this->ColumnNumber = 0;
  this->File = 0;
  this->InsideQuotes = false;
  this->LastString = 0;
  this->CurrentNode = 0;
  this->ParseList = false;
  this->ParseObjectList = false;
  this->LastValue = NAN;
  this->ShortNamesFilled = false;
}

//---------------------------------------------------
vtkJSONParser::~vtkJSONParser()
{
  delete this->FileName;
  this->FileName = 0;
}


//---------------------------------------------------
int vtkJSONParser::Parse(vtkTable* theTable) 
{
  if(!this->FileName) {
    vtkErrorMacro("The name of the file is not defined");
    return 1;
  }
  this->File = fopen( this->FileName, "r" );
  
  if( !this->File ) {
    std::string message  = std::string("Can't open file: ") + std::string(this->FileName);
    throwSimpleException(message.c_str());
    return 1;
  }
  char ch = 0;

  this->ExpectedCharacters.push_back(OCB);
  
  while( ch != EOF ) {
    ch = fgetc(this->File);
    processCharacter(ch);
    if(isDigitsAllowed() && isDigitOrDot(ch)) {
      readDoubleValue();
    }
    int nb = 1;
    switch(ch) {
    case OCB: processOCB(); break;
    case CCB: processCCB(); break;

    case OSB: processOSB(); break;
    case CSB: processCSB(); break;

    case ENDL: processENDL(); nb = 0; break;

    case QTS: processQTS(); break;

    case COLON: processCOLON(); break;

    case COMMA: processCOMMA(); break;
    }
    this->ColumnNumber+=nb;
  }  
  fclose(this->File);

  if(this->CInfoVector.size() > 0 ) {
    throwException("braket is not closed ", 
		   this->CInfoVector.back().ln,
		   this->CInfoVector.back().cn);
  }

  finalize(theTable);
  clean();
  return 1;
}

//---------------------------------------------------
void vtkJSONParser::finalize( vtkTable *t ) {
  std::vector<vtkJSONNode*>::iterator it = this->Nodes.begin();
  vtkJSONMetaNode* mn = 0;
  for( ; it != this->Nodes.end(); it++ ) {
    mn = dynamic_cast<vtkJSONMetaNode*>(*it);
    if (mn)
      break;
  }
  std::vector<const char*> units;
  if(mn) {
    if(!mn->GetShortNames().empty()) {
      this->ShortNames.clear();
      this->ShortNames = mn->GetShortNames();
    }
    t->GetInformation()->Set(vtkDataObject::FIELD_NAME(), mn->GetTableName() ? mn->GetTableName() : "");
    units = mn->GetUnits();
  }
  
  long nbRow = mn ? (this->Nodes.size() - 1 ) : this->Nodes.size();
  for (long col=0; col < this->ShortNames.size(); col++) {
    vtkDoubleArray* newCol = vtkDoubleArray::New();
    newCol->SetNumberOfValues(nbRow);
    vtkJSONInfoNode* in = 0;
    std::string name = this->ShortNames[col];
    name += "[";
    if(col < units.size()){
      name += units[col];
    } else {
      name += NA;
    }
    name += "]";
    newCol->SetName(name.c_str());
    it = this->Nodes.begin();
    long row = 0;
    for( ; it != this->Nodes.end(); it++ ) {
      in = dynamic_cast<vtkJSONInfoNode*>(*it);
      if (in) {
	vtkJSONMap& vl = in->GetValues();
	vtkJSONMap::iterator mit = vl.find(this->ShortNames[col]);
	if(mit != vl.end()) {
	  newCol->SetValue(row, mit->second);
	  vl.erase(mit);
	  delete mit->first;
	  const char* c = mit->first;
	  c = 0;
	  row++;
	} else {
	  std::string s("Item with name '");
	  s+=in->GetName();
	  s+="' has not key '";
	  s+=this->ShortNames[col];
	  s+="' !";
	  throwSimpleException(s.c_str());
	}
      }
    }
    t->AddColumn(newCol);
  } 
  it = this->Nodes.begin();
  vtkJSONInfoNode* in = 0;
  for( ; it != this->Nodes.end(); it++ ) {
    in = dynamic_cast<vtkJSONInfoNode*>(*it);
    if (in) {     
      vtkJSONMap& vl = in->GetValues();
      if(vl.size() > 0 ) {
	std::string s("Item with name '");
	s+=in->GetName();
	s+="' has unexpected key '";
	s+=vl.begin()->first;
	s+="' !";
	throwSimpleException(s.c_str());	
      }
    }
  }
}

//---------------------------------------------------
void vtkJSONParser::processQTS() {
  this->InsideQuotes = !this->InsideQuotes;
  if(this->InsideQuotes) {
    addInfo(QTS);
  } else {
    // Quotes is closed, get content
    Info i =  this->CInfoVector.back();
    this->CInfoVector.pop_back();
    if(i.type == QTS) {
      long begin = i.pos;
      long end = ftell(this->File) - 1;
      this->LastString = getString(begin, end);
      bool parse_list = (this->CInfoVector.size() >= 1 && this->CInfoVector.back().type == OSB);
      if(parse_list) {
	this->CurrentList.push_back(this->LastString);
      } else {
	this->Strings.push_back(this->LastString);
	processMetaNode();
      }
#ifdef __DEBUG
      std::cout<<"String  : "<<this->LastString<<std::endl;
#endif
      
      this->ExpectedCharacters.clear();
      this->ExpectedCharacters.push_back(COLON);
      this->ExpectedCharacters.push_back(COMMA);
      this->ExpectedCharacters.push_back(CCB);
      this->ExpectedCharacters.push_back(CSB); 
    }
  }
}

//---------------------------------------------------
void vtkJSONParser::processCOLON() {
  if (this->InsideQuotes)
    return;
  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(OCB);
  this->ExpectedCharacters.push_back(QTS);
  this->ExpectedCharacters.push_back(OSB);
  if(GetInfoNode() && Strings.size() ==  1 ) {
    allowsDigits();
  }
}

//---------------------------------------------------
void vtkJSONParser::processOCB() { 
  if (this->InsideQuotes)
    return;
  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(QTS);
  this->ExpectedCharacters.push_back(CCB);
  
  // Create node
  if(this->CInfoVector.size() >= 1) {    
    if ( !GetMetaNode() && this->Strings.size() > 0 && 
	 this->Strings.back() &&
	 strcmp(this->Strings.back(), MD) == 0 ) {
#ifdef __DEBUG
      std::cout<<"Create new Meta Node !!!"<<std::endl;
#endif      
      this->CurrentNode = new vtkJSONMetaNode();
      delete this->Strings.back();
      this->Strings.back() = 0;
      this->Strings.pop_back();
    } else {
      if(this->CInfoVector.back().type == OSB ) {
	this->ParseObjectList = true;
      }
#ifdef __DEBUG
      std::cout<<"Create new Node with name '"<<(this->Strings.size() == 0 ? "" : this->Strings.back())<<"' !!!"<<std::endl;
#endif          
      this->CurrentNode = new vtkJSONInfoNode(this->Strings.size() == 0 ? "" : this->Strings.back());
      if(!this->ParseObjectList && this->Strings.size() == 1) {
	this->Strings.pop_back();
      }
    }
  }
  addInfo(OCB);
}

//---------------------------------------------------
vtkJSONMetaNode* vtkJSONParser::GetMetaNode() {
  vtkJSONMetaNode *mnode = 0;
  if( this->CurrentNode ) {
    mnode = dynamic_cast<vtkJSONMetaNode*>(this->CurrentNode);
  }
  return mnode;
}

//---------------------------------------------------
vtkJSONInfoNode* vtkJSONParser::GetInfoNode() {
  vtkJSONInfoNode *mnode = 0;
  if( this->CurrentNode ) {
    mnode = dynamic_cast<vtkJSONInfoNode*>(this->CurrentNode);
  }
  return mnode;
}

//---------------------------------------------------
void vtkJSONParser::processENDL() { 
  if(this->InsideQuotes) {

    throwException("quote is not closed !");
  }
  this->LineNumber++;
  this->ColumnNumber=0;
}


//---------------------------------------------------
void vtkJSONParser::processCSB() {
  if (this->InsideQuotes)
    return;  
 
  if(this->CInfoVector.back().type == OSB) {
    if(this->ParseList) {
      this->ParseList = false;
    }    
    if(this->ParseObjectList) {
      this->ParseObjectList = false;
    }
    this->CInfoVector.pop_back();
  }

  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(COMMA);
  this->ExpectedCharacters.push_back(OCB);
  this->ExpectedCharacters.push_back(CCB);
}

//---------------------------------------------------
void vtkJSONParser::processOSB() {
  if (this->InsideQuotes)
    return;

  if(this->ParseList) {
    Info i = CInfoVector.back();
    if(i.type == OSB) {
      throwException("list, which has been opened in this place, has not been closed !", i.ln, i.cn);
    }
  }  
  if(GetMetaNode()) {
    this->ParseList = true;
  } else {
    if( this->Strings.size() > 0 ) {
      delete this->Strings[Strings.size()-1];
      this->Strings[Strings.size()-1] = 0;
      this->Strings.pop_back();
    }
  }
  addInfo(OSB);
  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(QTS);
  this->ExpectedCharacters.push_back(OCB);
}

//---------------------------------------------------
void vtkJSONParser::processCCB() {
  if (this->InsideQuotes)
    return;

  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(COMMA);
  this->ExpectedCharacters.push_back(CCB);
  if(this->ParseObjectList) {
    this->ExpectedCharacters.push_back(CSB);
  }

  processMetaNode();
  processInfoNode();

  if(this->CurrentNode)
    this->Nodes.push_back(this->CurrentNode);
  if( !this->ShortNamesFilled ){
    vtkJSONInfoNode* n = dynamic_cast<vtkJSONInfoNode*>(this->CurrentNode);
    if(n){
      this->ShortNamesFilled = true;
    }
  }

#ifdef __DEBUG
  if(this->CurrentNode)
    std::cout<<"End parsing node with name '"<<this->CurrentNode->GetName()<<"' !!!!"<<std::endl;
#endif

  if (this->CInfoVector.size() > 0 && this->CInfoVector.back().type == OCB ) {
    this->CInfoVector.pop_back();
  } else{
    throwException ("unexpected closed braket '}' !");
  }

  this->CurrentNode = 0;
}

//---------------------------------------------------
void vtkJSONParser::processCOMMA() {
  if (this->InsideQuotes)
    return;
  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(QTS);
  this->ExpectedCharacters.push_back(OCB);
  processMetaNode();
  processInfoNode();
}

void vtkJSONParser::processMetaNode() {
 vtkJSONMetaNode* mn = GetMetaNode();
  if(mn) {
    bool strings  = this->Strings.size() == 2 && !this->ParseList;
    bool str_and_list = (this->Strings.size() == 1 && this->CurrentList.size() > 0 && !this->ParseList);
    
    if( strings ) {
      if ( strcmp(this->Strings[0], CMT) == 0 ) {
	mn->SetComment(this->Strings[1]);
#ifdef __DEBUG
      std::cout<<"Table Comment  : "<<this->Strings[1]<<std::endl;
#endif
      } else if ( strcmp(this->Strings[0], TBN) == 0 ) { 
	mn->SetTableName(this->Strings[1]);
#ifdef __DEBUG
      std::cout<<"Table Name  : "<<this->Strings[1]<<std::endl;
#endif
      } else if ( strcmp(this->Strings[0], TBD) == 0 ) {
	mn->SetTableDescription(this->Strings[1]);
#ifdef __DEBUG
	std::cout<<"Table Description  : "<<this->Strings[1]<<std::endl;
#endif
      } else if ( strcmp(this->Strings[0], DT) == 0 ) {
	mn->SetDate(this->Strings[1]);
#ifdef __DEBUG
      std::cout<<"Date  : "<<this->Strings[1]<<std::endl;
#endif
      } else {
	std::stringstream s;
	s << "File : "<< this->FileName;
	s << ", line "<<this->LineNumber;
	s << " unexpected key world: '"<<this->Strings[0]<<"'";
      }
      delete this->Strings[0];
      this->Strings[0] = 0;
      Strings.pop_back();
      Strings.pop_back();
      
    } else if(str_and_list) {
      if ( strcmp(this->Strings[0], SHT) == 0 ) { 
	std::vector<const char*>::const_iterator it = this->CurrentList.begin();
	for( ;it != this->CurrentList.end(); it++) {
	  checkShortName(*it);
	}
	mn->SetShortNames(this->CurrentList);
#ifdef __DEBUG
	std::cout<<"Short Names : "<<std::endl;
#endif
      } else if ( strcmp(this->Strings[0], LNG) == 0 ) { 
	mn->SetLongNames(this->CurrentList);
#ifdef __DEBUG
	std::cout<<"Long Names : "<<std::endl;
#endif
      } else if ( strcmp(this->Strings[0], UNT) == 0 ) { 
	mn->SetUnits(this->CurrentList);
#ifdef __DEBUG
	std::cout<<"Units : ";
#endif
      } else {
	std::stringstream s;
	s << "File : "<< this->FileName;
	s << ", line "<<this->LineNumber;
	s << " unexpected key world: '"<<this->Strings[0]<<"'";
      }
      delete this->Strings[0];
      this->Strings[0] = 0;
      Strings.pop_back();
#ifdef __DEBUG
      std::vector<const char*>::const_iterator it = this->CurrentList.begin();
      std::cout<<"[ ";
      for( ;it != this->CurrentList.end(); it++) {
        std::cout<<"'"<<*it<<"'";
	if ( it+1 != this->CurrentList.end() )
	  std::cout<<", ";
      }
      std::cout<<" ]"<<std::endl;
#endif
      this->CurrentList.clear();
    }
  }
}
//---------------------------------------------------
void vtkJSONParser::processInfoNode() { 
 vtkJSONInfoNode* in = GetInfoNode();
  if(in) {
    if(this->Strings.size() == 1 && !isnan(this->LastValue)) {
      in->AddValue(this->Strings[0],this->LastValue);
      if(!ShortNamesFilled) {
	char* name = new char[strlen(Strings[0])];
	strcpy(name, Strings[0]);
	this->ShortNames.push_back(name);
      }
      this->Strings.pop_back();
      this->LastValue = NAN;
    }
  }
}

//---------------------------------------------------
void vtkJSONParser::processCharacter(const char ch) {
  if (this->InsideQuotes)
    return;
  if(isBlankOrEnd(ch))
    return;

  if(this->ExpectedCharacters.empty())
    return;
  
  std::vector<char>::const_iterator it = std::find(this->ExpectedCharacters.begin(),
						   this->ExpectedCharacters.end(),
						   ch);  
  
  // Unexpected character is found
  if(it == this->ExpectedCharacters.end()) {
    std::string s("unexpected character '");
    s+=ch;
    s+="' !";
    throwException(s.c_str());

  }
}

//---------------------------------------------------
char* vtkJSONParser::getString(long b, long e) {
  char* result = 0;

  long old_pos = ftell(this->File);
  fseek(this->File, b, SEEK_SET);
  long data_s = e - b;
  result = new char[data_s];
  result[0] = 0;
  size_t nb_read = fread(result, sizeof(char), data_s, this->File);  
  result[nb_read] = '\0';
  fseek(this->File, old_pos, SEEK_SET);
  return result;
}

//---------------------------------------------------
void vtkJSONParser::allowsDigits() {
  for(char c = '.'; c <= '9'; c++) {
    ExpectedCharacters.push_back(c);
  }
  ExpectedCharacters.push_back('-');
  ExpectedCharacters.push_back('+');
  ExpectedCharacters.push_back('e');
}

//---------------------------------------------------
bool vtkJSONParser::isDigitsAllowed() {
  std::vector<char>::const_iterator it = std::find(this->ExpectedCharacters.begin(),
						   this->ExpectedCharacters.end(),
						   '0');
  return (it != this->ExpectedCharacters.end());
}

//---------------------------------------------------
void vtkJSONParser::readDoubleValue() {
  long b = ftell(this->File);

  while(1) {
    char ch = fgetc(this->File); 
    if(!isDigitOrDot(ch)) {
      break;
    }
  }

  long e = ftell(this->File);
  fseek(this->File, b-1, SEEK_SET);
  long data_s = e - b;
  char* result = new char[data_s];
  result[0] = 0;
  size_t nb_read = fread(result, sizeof(char), data_s, this->File);  
  result[nb_read] = '\0';
  this->ExpectedCharacters.clear();
  this->ExpectedCharacters.push_back(COMMA);
  this->ExpectedCharacters.push_back(CCB);
  this->LastValue = atof(result);
#ifdef __DEBUG
  std::cout<<"Read number : "<<this->LastValue<<std::endl;
#endif
}

//---------------------------------------------------
void vtkJSONParser::checkShortName(const char* name) {
  size_t ln = strlen(name);
  if( ln > 0 ){
    for( size_t i = 0; i < ln; i++ ) { 
      // a - z
      if(!(name[i] >= 'a' &&  name[i] <= 'z') && 
      // A - Z
	 !(name[i] >= 'A' &&  name[i] <= 'Z') ) {
	std::string s("wrong short name '");
	s += name;
	s += "' !";
	throwException(s.c_str(), this->LineNumber);
      }
    }
  }
}

void vtkJSONParser::clean() {
  std::vector<vtkJSONNode*>::iterator it = this->Nodes.begin();
  for( ; it != this->Nodes.end(); it++ ) {
    delete *(it);
    *it = 0;
  }
}

//---------------------------------------------------
void vtkJSONParser::throwException(const char* message) {
    std::stringstream s;
    s << "File : "<< this->FileName;
    s << ", line "<<this->LineNumber;
    s << ", column " << this->ColumnNumber<<" : ";
    s << message;
    clean();
    throw vtkJSONException(s.str().c_str());  
}

//---------------------------------------------------
void vtkJSONParser::throwException(const char* message, int ln, int cn) {
    std::stringstream s;
    s << "File : "<< this->FileName;
    s << ", line "<<ln;
    s << ", column :" << cn << "  ";
    s << message;
    clean();
    throw vtkJSONException(s.str().c_str());  
}
//---------------------------------------------------
void vtkJSONParser::throwException(const char* message, int ln) {
  std::stringstream s;
   s << "File "<< this->FileName;
   s << ", line "<<ln <<" : ";
   s << message;
   clean();
   throw vtkJSONException(s.str().c_str());  
}

//---------------------------------------------------
void vtkJSONParser::throwSimpleException(const char* message) {
  clean();
  throw vtkJSONException(message);
}
