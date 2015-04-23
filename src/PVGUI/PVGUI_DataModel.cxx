// Copyright (C) 2010-2015  CEA/DEN, EDF R&D
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
// Author : Adrien Bruneton (CEA)
//

//Local includes
#include "PVGUI_DataModel.h"
#include "PVGUI_Module.h"

// GUI includes
#include <LightApp_Study.h>
#include <CAM_DataObject.h>

// Qt includes
#include <QFile>
#include <QTextStream>

PVGUI_DataModel::PVGUI_DataModel( PVGUI_Module* theModule ):
  LightApp_DataModel(theModule)
{}

PVGUI_DataModel::~PVGUI_DataModel()
{}

bool PVGUI_DataModel::dumpPython( const QString& path, CAM_Study* std,
            bool isMultiFile, QStringList& listOfFiles)
{

  LightApp_Study* study = dynamic_cast<LightApp_Study*>( std );
  if(!study)
    return false;

  std::string aTmpDir = study->GetTmpDir( path.toLatin1().constData(), isMultiFile );
  std::string aFile = aTmpDir + "paravis_dump.tmp";

  listOfFiles.append(aTmpDir.c_str());
  listOfFiles.append("paravis_dump.tmp");

  QFile file(aFile.c_str());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;

  PVGUI_Module * mod = (PVGUI_Module *) getModule();
  QString trace(mod->getTraceString());

  if (isMultiFile)
    {
      QStringList abuffer;
      abuffer.push_back(QString("def RebuildData( theStudy ):"));
      QStringList lst(trace.split("\n"));
      foreach(QString elem, lst)
        {
          QString s = "  " + elem;
          abuffer.push_back(s);
        }
      abuffer.push_back(QString("  pass"));
      trace = abuffer.join("\n");
    }
  QTextStream out(&file);
  out << trace.toStdString().c_str() << "\n";
  out.flush();
  file.close();

  return true;
}

/*-----------------------------------------------------------------------------------------*/
bool PVGUI_DataModel::open( const QString& theName, CAM_Study* theStudy, QStringList theList) {
  bool res = LightApp_DataModel::open(theName, theStudy, theList);
  publishComponent(theStudy);
  return res;
}

/*-----------------------------------------------------------------------------------------*/
bool PVGUI_DataModel::create( CAM_Study* theStudy) {
  bool res = LightApp_DataModel::create(theStudy);
  publishComponent(theStudy);
  return res;
}
/*-----------------------------------------------------------------------------------------*/
void PVGUI_DataModel::publishComponent( CAM_Study* theStudy ) {
  LightApp_Study* study = dynamic_cast<LightApp_Study*>( theStudy );
  CAM_ModuleObject *aModelRoot = dynamic_cast<CAM_ModuleObject*>( root());
  if( study && aModelRoot == NULL ) {
    aModelRoot = createModuleObject( theStudy->root() );
    aModelRoot->setDataModel( this );
    setRoot(aModelRoot);
  }
}
