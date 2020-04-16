// Copyright (C) 2010-2020  CEA/DEN, EDF R&D
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
#include <LightApp_Module.h>
#include <LightApp_Application.h>
#include <LightApp_DataModel.h>
#include <CAM_DataObject.h>
#include <SUIT_Tools.h>
#include <SUIT_Session.h>
#include <SUIT_ResourceMgr.h>

// KERNEL
#include <utilities.h>

// Qt includes
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDomNode>

// ParaView include
#include <pqApplicationCore.h>
#include <pqServer.h>
#include <vtkOutputWindow.h>

const QString PVGUI_DataModel::RESTORE_FLAG_FILE = "do_restore_paravis_references.par";

/*!
 * XML processing functions to handle the PV state file.
 */
namespace {

  QStringList multiFormats()
  {
    static QStringList formats;
    if ( formats.isEmpty() )
      formats << "vtm" << "lata" << "pvd";
    return formats;
  }
  void warning( const QString& message )
  {
    vtkOutputWindow* vtkWindow = vtkOutputWindow::GetInstance();
    if ( vtkWindow )
    {
      vtkWindow->DisplayWarningText( qPrintable( message ) );
    }
  }

  void processElements(QDomNode& thePropertyNode, QStringList& theFileNames,
                       const QString& theNewPath, bool theRestore)
  {
    QDomElement aProperty = thePropertyNode.toElement();
    int aNbElements = aProperty.attribute("number_of_elements").toInt();
    if ( aNbElements > 1 )
      warning( QString("You save data file with number of entities > 1 (%1); some data may be lost!").arg(aNbElements) );
    QDomNode aElementNode = thePropertyNode.firstChild();
    while (aElementNode.isElement()) {
      QDomElement aElement = aElementNode.toElement();
      if (aElement.tagName() == "Element") {
        QString aIndex = aElement.attribute("index");
        if (aIndex == "0") {
          QString aValue = aElement.attribute("value");
          if (!aValue.isNull()) {
            if (theNewPath.isEmpty()) {
              QFileInfo aFInfo(aValue);
              if (aFInfo.exists()) {
                QString ext = aFInfo.suffix();
                if ( multiFormats().contains(aFInfo.suffix()) )
                  warning( QString("You save data in multiple file format (%1); some data may be not saved!").arg(ext) );
                theFileNames<<aValue;
                aElement.setAttribute("value", aFInfo.fileName());
              }
              break;
            } else {
              if (theRestore)
                aElement.setAttribute("value", QString(theNewPath) + aValue);
            }
          }
        }
      }
      aElementNode = aElementNode.nextSibling();
    }
  }

  void processProperties(QDomNode& theProxyNode, QStringList& theFileNames,
                         const QString& theNewPath, bool theRestore)
  {
    QDomNode aPropertyNode = theProxyNode.firstChild();
    while (aPropertyNode.isElement()) {
      QDomElement aProperty = aPropertyNode.toElement();
      QString aName = aProperty.attribute("name");
      if ((aName == "FileName") || (aName == "FileNameInfo") || (aName == "FileNames")) {
        processElements(aPropertyNode, theFileNames, theNewPath, theRestore);
      }
      aPropertyNode = aPropertyNode.nextSibling();
    }
  }


  void processProxies(QDomNode& theNode, QStringList& theFileNames,
                      const QString& theNewPath, bool theRestore)
  {
    QDomNode aProxyNode = theNode.firstChild();
    while (aProxyNode.isElement()) {
      QDomElement aProxy = aProxyNode.toElement();
      if (aProxy.tagName() == "Proxy") {
        QString aGroup = aProxy.attribute("group");
        if (aGroup == "sources") {
          processProperties(aProxyNode, theFileNames, theNewPath, theRestore);
        }
      }
      aProxyNode = aProxyNode.nextSibling();
    }
  }

  bool processAllFilesInState(const QString& aFileName, QStringList& theFileNames,
                              const QString& theNewPath, bool theRestore)
  {
    QFile aFile(aFileName);
    if (!aFile.open(QFile::ReadOnly)) {
      MESSAGE("Can't open state file "<<aFileName.toStdString());
      return false;
    }
    QDomDocument aDoc;
    bool aRes = aDoc.setContent(&aFile);
    aFile.close();

    if (!aRes) {
      MESSAGE("File "<<aFileName.toStdString()<<" is not XML document");
      return false;
    }

    QDomElement aRoot = aDoc.documentElement();
    if ( aRoot.isNull() ) {
      MESSAGE( "Invalid XML root" );
      return false;
    }

    QDomNode aNode = aRoot.firstChild();
    while (aRes  && !aNode.isNull() ) {
      aRes = aNode.isElement();
      if ( aRes ) {
        QDomElement aSection = aNode.toElement();
        if (aSection.tagName() == "ServerManagerState") {
          processProxies(aNode, theFileNames, theNewPath, theRestore);
        }
      }
      aNode = aNode.nextSibling();
    }
    if (!aFile.open(QFile::WriteOnly | QFile::Truncate)) {
      MESSAGE("Can't open state file "<<aFileName.toStdString()<<" for writing");
      return false;
    }
    QTextStream out(&aFile);
    aDoc.save(out, 2);
    aFile.close();

    return true;
  }
}


PVGUI_DataModel::PVGUI_DataModel( PVGUI_Module* theModule ):
  LightApp_DataModel(theModule),
  myStudyURL("")
{}

PVGUI_DataModel::~PVGUI_DataModel()
{}

bool PVGUI_DataModel::create( CAM_Study* theStudy) {
  bool res = LightApp_DataModel::create(theStudy);
  publishComponent(theStudy);
  return res;
}

void PVGUI_DataModel::publishComponent( CAM_Study* theStudy ) {
  LightApp_Study* study = dynamic_cast<LightApp_Study*>( theStudy );
  CAM_ModuleObject *aModelRoot = dynamic_cast<CAM_ModuleObject*>( root());
  if( study && aModelRoot == NULL ) {
    aModelRoot = createModuleObject( theStudy->root() );
    aModelRoot->setDataModel( this );
    setRoot(aModelRoot);
  }
}

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
      abuffer.push_back(QString("def RebuildData():"));
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

/*!
  \brief Open data model (read ParaView pipeline state from the files).
  \param theName study file path
  \param theStudy study pointer
  \param theList list of the (temporary) files with data
  \return operation status (\c true on success and \c false on error)
*/
bool PVGUI_DataModel::open( const QString& theName, CAM_Study* theStudy, QStringList theList)
{
  bool ret = false;
  LightApp_Study* aDoc = dynamic_cast<LightApp_Study*>( theStudy );
  if ( !aDoc )
    return false;

  LightApp_DataModel::open( theName, aDoc, theList );
  publishComponent(theStudy);

  // The first list item contains path to a temporary
  // directory, where the persistent files was placed
  if ( theList.count() > 0 ) {
    QString aTmpDir ( theList[0] );

    if ( theList.size() >= 2 ) {
      myStudyURL = theName;
      QString aFullPath = SUIT_Tools::addSlash( aTmpDir ) + theList[1];
//      std::cout << "open: tmp dir is" << aFullPath.toStdString() << std::endl;
      PVGUI_Module * mod = dynamic_cast<PVGUI_Module *>(getModule());
      if (mod)
        {
          bool doRestore = false;
          QStringList srcFilesEmpty;
          createAndCheckRestoreFlag(aTmpDir, srcFilesEmpty, /*out*/doRestore);
          if(doRestore)
            {
              // Update state file so that it points to new dir:
              processAllFilesInState(aFullPath, srcFilesEmpty, aTmpDir.toStdString().c_str(), true);
            }

          mod->loadParaviewState(aFullPath);
          ret = true;
        }
      ret = true;
    }
  }

  return ret;
}

/*!
 * Create an empty file indicating whether source files in the pipeline should be restored.
 */
bool PVGUI_DataModel::createAndCheckRestoreFlag(const QString& tmpdir, QStringList& listOfFiles, bool & alreadyThere)
{
  QString aFullPath = SUIT_Tools::addSlash( tmpdir ) + RESTORE_FLAG_FILE;
  QFile f(aFullPath);
  if (f.exists())
    {
    alreadyThere = true;
    return true;
    }
  else
    {
      bool ret = f.open(QFile::WriteOnly);
      if (ret)
        {
        f.close();
        listOfFiles << RESTORE_FLAG_FILE;
        }
      return ret;
    }
}


/*!
  \brief Save data model (write ParaView pipeline to the files).
  \param listOfFiles returning list of the (temporary) files with saved data
  \return operation status (\c true on success and \c false on error)
*/
bool PVGUI_DataModel::save( QStringList& theListOfFiles)
{
  bool isMultiFile = false; // TODO: decide, how to access this parameter
  bool ret = false;

  LightApp_DataModel::save( theListOfFiles );

  LightApp_Study* study = dynamic_cast<LightApp_Study*>( getModule()->getApp()->activeStudy() );
  QString aTmpDir = study->GetTmpDir( myStudyURL.toLatin1(), isMultiFile ).c_str();
//  std::cout << "save: tmp dir is" << aTmpDir.toStdString() << std::endl;

  QString aFileName = SUIT_Tools::file( myStudyURL, false ) + "_PARAVIS.pvsm";
  QString aFullPath = aTmpDir + aFileName;

  PVGUI_Module * mod = dynamic_cast<PVGUI_Module *>(getModule());
  QStringList srcFiles;
  if (mod)
    {
      // Create ParaView state file:
      mod->saveParaviewState(aFullPath.toStdString().c_str());

      // add this to the list to be saved:
      theListOfFiles << aTmpDir;
      theListOfFiles << aFileName;

      // Potentially save referenced files:
      SUIT_ResourceMgr* aResourceMgr = SUIT_Session::session()->resourceMgr();
      int aSavingType = aResourceMgr->integerValue( "PARAVIS", "savestate_type", 0 );

      bool unused;
      bool isBuiltIn = false;
      pqServer* aServer;
      QString nullS;

      switch (aSavingType) {
        case 0: // Save referenced files when they are accessible
          createAndCheckRestoreFlag(aTmpDir, theListOfFiles ,unused);
          processAllFilesInState(aFullPath, srcFiles, nullS, false);
          break;
        case 1: // Save referenced files only if this is the builtin server
          aServer = pqApplicationCore::instance()->getActiveServer();
          if (aServer)
            isBuiltIn = !aServer->isRemote();
          if(isBuiltIn)
            {
              createAndCheckRestoreFlag(aTmpDir, theListOfFiles, unused);
              processAllFilesInState(aFullPath, srcFiles, nullS, false);
            }
          break;
        case 2: // Do not save referenced file
          break;
        default:
          break;
      }

      ret = true;
    }
  // Copying valid source files to the temp directory and adding them to the list
  foreach(QString fName, srcFiles)
  {
    QFile fSrc(fName);
    if (fSrc.exists())
      {
        QFileInfo inf(fSrc);
        QString newPth(SUIT_Tools::addSlash( aTmpDir ) + inf.fileName());
        if (fSrc.copy(newPth))
          {
            theListOfFiles << inf.fileName();
          }
      }
  }

  return ret;
}

/*!
  \brief Save data model (write ParaView pipeline state to the files).
  \param url study file path
  \param study study pointer
  \param listOfFiles returning list of the (temporary) files with saved data
  \return operation status (\c true on success and \c false on error)
*/
bool PVGUI_DataModel::saveAs( const QString& url, CAM_Study* study, QStringList& theListOfFiles)
{
  myStudyURL = url;
  return save( theListOfFiles );
}
