//  Copyright (C) 2015 CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License.
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


#include "vtkJSONParserTest.hxx"
#include <vtkJSONParser.h>

#include <vtkTable.h>
#include <vtkVariant.h>
#include <vtkAbstractArray.h>

#include <iostream>
#include <sstream>
#include <stdlib.h>

//---------------------------------------------------
std::string vtkJSONParserTest::getGoodFilesDir() {
  char* c =  getenv("PARAVIS_SRC_DIR");
  std::string s = std::string(c == 0 ? "" : c);
  if( !s.empty() ) {
    s+="/src";
    s+="/Plugins";
    s+="/JSONReader";
    s+="/Examples/";
  }
  return s;
}

//---------------------------------------------------
std::string vtkJSONParserTest::getBadFilesDir() {
  char* c =  getenv("PARAVIS_SRC_DIR");
  std::string s = std::string(c == 0 ? "" : c);
  if( !s.empty() ) {
    s+="/src";
    s+="/Plugins";
    s+="/JSONReader";
    s+="/Test";
    s+="/BadFiles";
    s+="/";
  }  
  return s;
}

//---------------------------------------------------
void vtkJSONParserTest::testParseBadFiles() {
  std::string dir = getBadFilesDir();
  if(dir.empty())
    CPPUNIT_FAIL("Can't get examples dir !!! ");
  std::string fn = "bad_ex";
  
  // 11 existing files, bad_ex12.json doesn't exists, check exception
  for(int i = 1; i <=12; i++) {
    std::string s = dir + fn;
    std::stringstream convert;
    convert << i;
    s += convert.str();
    s += ".json";
    vtkJSONParser* Parser = vtkJSONParser::New();
    vtkTable* table = vtkTable::New();
    Parser->SetFileName(s.c_str());
    bool expected_exception_thrown = false;
    try{
      Parser->Parse(table);
    } catch(vtkJSONException e) {
      expected_exception_thrown = true;
    }
    Parser->Delete();
    table->Delete();
    if(!expected_exception_thrown) {
      CPPUNIT_FAIL("Expected exception is not thrown !!! ");
    }
  }
}

//---------------------------------------------------
void vtkJSONParserTest::testParseGoodFiles() {
  std::string dir = getGoodFilesDir();
  if(dir.empty())
    CPPUNIT_FAIL("Can't get examples dir !!! ");
  std::string fn = "example";
  
  for(int i = 1; i <=2; i++) {
    std::string s = dir + fn;
    std::stringstream convert;
    convert << i;
    s += convert.str();
    s += ".json";
    vtkJSONParser* Parser = vtkJSONParser::New();
    vtkTable* table = vtkTable::New();
    Parser->SetFileName(s.c_str());
    bool exception_thrown = false;
    try{
      Parser->Parse(table);
    } catch(vtkJSONException e) {
      exception_thrown = true;
    }
    Parser->Delete();
    table->Delete();
    
    if(exception_thrown) {
      CPPUNIT_FAIL("Unexpected exception has been thrown !!! ");
    }
  }

  vtkJSONParser* Parser = vtkJSONParser::New();
  vtkTable* table = vtkTable::New();
  fn = "example";
  std::string s = dir + fn;
  std::stringstream convert;
  convert << 2;
  s += convert.str();
  s += "_wo_metadata.json";
  Parser->SetFileName(s.c_str());
  bool exception_thrown = false;
  try{
    Parser->Parse(table);
  } catch(vtkJSONException e) {
    exception_thrown = true;      
  }    
  if(exception_thrown) {
    CPPUNIT_FAIL("Unexpected exception has been thrown !!! ");
  }

  double v = table->GetValue(2,0).ToDouble();
  double v1 = table->GetValue(2,1).ToDouble();
  double v2 = table->GetValue(2,2).ToDouble();

  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 3.15, table->GetValue(0,0).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 0.0, table->GetValue(0,1).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 43.5, table->GetValue(0,2).ToDouble(), 0.001);

  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 4.156, table->GetValue(1,0).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 465, table->GetValue(1,1).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 137.5, table->GetValue(1,2).ToDouble(), 0.001);

  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", -3.0305890e+4, table->GetValue(2,0).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", 1.0305890e-1, table->GetValue(2,1).ToDouble(), 0.001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Unexpected value : ", -3.0305890e+5, table->GetValue(2,2).ToDouble(), 0.001);

  std::string s1 ();
    
  CPPUNIT_ASSERT_EQUAL( std::string("P[n/a]"), std::string(table->GetColumn(0)->GetName()));
  CPPUNIT_ASSERT_EQUAL( std::string("u[n/a]"), std::string(table->GetColumn(1)->GetName()));
  CPPUNIT_ASSERT_EQUAL( std::string("weight[n/a]"), std::string(table->GetColumn(2)->GetName()));
  
  Parser->Delete();
  table->Delete();
}
