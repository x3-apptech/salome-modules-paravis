// PARAVIS : ParaView wrapper SALOME module
//
// Copyright (C) 2010-2019  CEA/DEN, EDF R&D
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
// File   : PVGUI_Module.cxx

#define PARAVIS_MODULE_NAME "PARAVIS"

#include <Standard_math.hxx>  // E.A. must be included before Python.h to fix compilation on windows
#ifdef HAVE_FINITE
#undef HAVE_FINITE            // VSR: avoid compilation warning on Linux : "HAVE_FINITE" redefined
#endif
#include <vtkPython.h> // Python first

#include "PVGUI_Module.h"

#include "PVViewer_ViewManager.h"
#include "PVViewer_Core.h"
#include "PVViewer_ViewWindow.h"
#include "PVViewer_ViewModel.h"
#include "PVGUI_ParaViewSettingsPane.h"
#include "PVViewer_GUIElements.h"
#include "PVServer_ServiceWrapper.h"
#include "PVGUI_DataModel.h"

// SALOME Includes
#include <utilities.h>
#include <SUIT_DataBrowser.h>
#include <SUIT_Desktop.h>
#include <SUIT_MessageBox.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <SUIT_OverrideCursor.h>
#include <SUIT_ExceptionHandler.h>

#include <LightApp_SelectionMgr.h>
#include <LightApp_NameDlg.h>
#include <LightApp_Application.h>
#include <LightApp_Study.h>
#include <SALOME_ListIO.hxx>

#include <QtxActionMenuMgr.h>
#include <QtxActionToolMgr.h>

#include <PARAVIS_version.h>

// External includes
#include <sstream>

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QStatusBar>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QToolBar>
#include <QTextStream>
#include <QShortcut>
#include <QDockWidget>
#include <QHelpEngine>

// Paraview includes
#include <vtkPVConfig.h>  // for symbol PARAVIEW_VERSION
#include <vtkProcessModule.h>
#include <vtkPVSession.h>
#include <vtkPVProgressHandler.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>
#include <vtkSMProxy.h>
#include <vtkSmartPointer.h>
#include <vtkSMSession.h>
#include <vtkSMTrace.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMParaViewPipelineController.h>
#include <vtkSmartPyObject.h>

#include <pqApplicationCore.h>
#include <pqPVApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqOptions.h>
#include <pqSettings.h>
#include <pqServer.h>
#include <pqUndoStack.h>
#include <pqTabbedMultiViewWidget.h>
#include <pqActiveObjects.h>
#include <pqHelpReaction.h>
#include <pqPluginManager.h>
//#include <pqPythonDialog.h>
#include <pqPythonManager.h>
#include <pqLoadDataReaction.h>
#include <pqPythonScriptEditor.h>
#include <pqDataRepresentation.h>
#include <pqDisplayColorWidget.h>
#include <pqColorToolbar.h>
#include <pqScalarBarVisibilityReaction.h>
#include <pqServerResource.h>
#include <pqServerConnectReaction.h>
#include <pqPluginManager.h>
#include <pqVCRToolbar.h>
#include <pqAnimationScene.h>
#include <pqServerManagerModel.h>
#include <pqAnimationTimeToolbar.h>
#include <pqPipelineBrowserWidget.h>
#include <pqCoreUtilities.h>

#if PY_VERSION_HEX < 0x03050000
static char*
Py_EncodeLocale(const wchar_t *arg, size_t *size)
{
	return _Py_wchar2char(arg, size);
}
static wchar_t*
Py_DecodeLocale(const char *arg, size_t *size)
{
	return _Py_char2wchar(arg, size);
}
#endif

//----------------------------------------------------------------------------
PVGUI_Module* ParavisModule = 0;

/*!
  \mainpage
  This is the doxygen documentation of the ParaVis module.
  If you are looking for general information about the structure of the module, you should
  take a look at the <a href="../index.html">Sphinx documentation</a> first.

  The integration of ParaView into SALOME is split in two parts:
  \li the PVViewer in the GUI module (folder *src/PVViewer*)
  \li the ParaVis module itself (the pages you are currently browsing)
*/

/*!
  \class PVGUI_Module
  \brief Implementation 
         SALOME module wrapping ParaView GUI.
*/

/*!
  \brief Clean up function

  Used to stop ParaView progress events when
  exception is caught by global exception handler.
*/
void paravisCleanUp()
{
  if ( pqApplicationCore::instance() ) {
    pqServer* s = pqApplicationCore::instance()->getActiveServer();
    if ( s ) s->session()->GetProgressHandler()->CleanupPendingProgress();
  }
}

/*!
  \brief Constructor. Sets the default name for the module.
*/
PVGUI_Module::PVGUI_Module()
  : LightApp_Module( PARAVIS_MODULE_NAME ),
    mySourcesMenuId( -1 ),
    myFiltersMenuId( -1 ),
    myMacrosMenuId(-1),
    myRecentMenuId(-1),
    myCatalystMenuId(-1),
    myOldMsgHandler(0),
    myTraceWindow(0),
    myInitTimer(0),
    myGuiElements(0)
{
#ifdef HAS_PV_DOC
  Q_INIT_RESOURCE( PVGUI );
#endif
  ParavisModule = this;
}

/*!
  \brief Destructor.
*/
PVGUI_Module::~PVGUI_Module()
{
  if (myInitTimer)
    delete myInitTimer;
}

/*!
  \brief Retrieve the PVSERVER CORBA engine.
  This uses the Python wrapper provided
  by the PVViewer code in GUI (class PVViewer_EngineWrapper).
  \sa GetCPPEngine()
*/
PVServer_ServiceWrapper* PVGUI_Module::GetEngine()
{
  return PVServer_ServiceWrapper::GetInstance();
}

/*!
  \brief Create data model.
  \return module specific data model
*/
CAM_DataModel* PVGUI_Module::createDataModel()
{
  return new PVGUI_DataModel( this );
}

/*!
  \brief Get the ParaView application singleton.
*/
pqPVApplicationCore* PVGUI_Module::GetPVApplication()
{
  return PVViewer_Core::GetPVApplication();
}

/*!
  \brief Initialize module. Creates menus, prepares context menu, etc.
  \param app SALOME GUI application instance
*/
void PVGUI_Module::initialize( CAM_Application* app )
{
  LightApp_Module::initialize( app );

  // Uncomment to debug ParaView initialization
  // "aa" used instead of "i" as GDB doesn't like "i" variables :)
  /*
  int aa = 1;
  while( aa ){
    aa = aa;
  }
  */

  LightApp_Application* anApp = getApp();
  SUIT_Desktop* aDesktop = anApp->desktop();

  // Remember current state of desktop toolbars
  QList<QToolBar*> foreignToolbars = aDesktop->findChildren<QToolBar*>();

  // Initialize ParaView client and associated behaviors
  // and connect to externally launched pvserver
  PVViewer_Core::ParaviewInitApp(aDesktop);

  // Clear old copies of embedded macros files
  //QString aDestPath = QString( "%1/.config/%2/Macros" ).arg( QDir::homePath() ).arg( QApplication::applicationName() );
  QString aDestPath = pqCoreUtilities::getParaViewUserDirectory() + "/Macros";
  QStringList aFilter;
  aFilter << "*.py";

  QDir aDestDir(aDestPath);
  QStringList aDestFiles = aDestDir.entryList(aFilter, QDir::Files);
  foreach(QString aMacrosPath, getEmbeddedMacrosList()) {
	  QString aMacrosName = QFileInfo(aMacrosPath).fileName();
	  if (aDestFiles.contains(aMacrosName)) {
		  aDestDir.remove(aMacrosName);
	  }
  }

  myGuiElements = PVViewer_GUIElements::GetInstance(aDesktop);


  // [ABN]: careful with the order of the GUI element creation, the loading of the configuration
  // and the connection to the server. This order is very sensitive if one wants to make
  // sure all menus, etc ... are correctly populated.
  // Reference points are: ParaViewMainWindow.cxx and branded_paraview_initializer.cxx.in
  setupDockWidgets();

  pvCreateActions();
  pvCreateMenus();
  pvCreateToolBars();

  PVViewer_Core::ParaviewInitBehaviors(true, aDesktop);

  QList<QDockWidget*> activeDocks = aDesktop->findChildren<QDockWidget*>();
  QList<QMenu*> activeMenus = aDesktop->findChildren<QMenu*>();

  // Setup quick-launch shortcuts.
  QShortcut *ctrlSpace = new QShortcut(Qt::CTRL + Qt::Key_Space, aDesktop);
  QObject::connect(ctrlSpace, SIGNAL(activated()),
    pqApplicationCore::instance(), SLOT(quickLaunch()));

  // Find Plugin Dock Widgets
  QList<QDockWidget*> currentDocks = aDesktop->findChildren<QDockWidget*>();
  QList<QDockWidget*>::iterator i;
  for (i = currentDocks.begin(); i != currentDocks.end(); ++i) {
    if(!activeDocks.contains(*i)) {
      myDockWidgets[*i] = false; // hidden by default
      (*i)->hide();
    }
  }

    // Find Plugin Menus
    // [ABN] TODO: fix this - triggers a SEGFAULT at deactivation() time.
//    QList<QMenu*> currentMenus = aDesktop->findChildren<QMenu*>();
//    QList<QMenu*>::iterator im;
//    for (im = currentMenus.begin(); im != currentMenus.end(); ++im) {
//      if(!activeMenus.contains(*im)) {
//          QString s = (*im)->title();
//          std::cout << " MENU "<<  s.toStdString() << std::endl;
//          myMenus.append(*im);
//      }
//    }

  // Connect after toolbar creation, etc ... as some activations of the toolbars is triggered
  // by the ServerConnection event:
  const QString configPath(PVViewer_ViewManager::GetPVConfigPath());
  PVViewer_Core::ParaviewLoadConfigurations(configPath, true);
  PVViewer_ViewManager::ConnectToExternalPVServer(aDesktop);
  updateObjBrowser();

  // Find created toolbars
  QCoreApplication::processEvents();

  // process PVViewer toolbars (might be added by PVViewer created BEFORE activating ParaVis)
  QList<QToolBar*> pvToolbars = myGuiElements->getToolbars();
  foreach(QToolBar* aBar, pvToolbars) {
    if (!myToolbars.contains(aBar)) {
      myToolbars[aBar] = true;
      myToolbarBreaks[aBar] = false;
      aBar->setVisible(false);
      aBar->toggleViewAction()->setVisible(false);
    }
  }

  // process other toolbars (possibly added by Paraview)
  QList<QToolBar*> allToolbars = aDesktop->findChildren<QToolBar*>();
  foreach(QToolBar* aBar, allToolbars) {
    if (!foreignToolbars.contains(aBar) && !myToolbars.contains(aBar)) {
      myToolbars[aBar] = true;
      myToolbarBreaks[aBar] = false;
      aBar->setVisible(false);
      aBar->toggleViewAction()->setVisible(false);
    }
  }

  updateMacros();
 
  SUIT_ResourceMgr* aResourceMgr = SUIT_Session::session()->resourceMgr();
  bool isStop = aResourceMgr->booleanValue( PARAVIS_MODULE_NAME, "stop_trace", false );
  if(!isStop)
    {
      // Start a timer to schedule asap:
      //  - the trace start
      myInitTimer = new QTimer(aDesktop);
      QObject::connect(myInitTimer, SIGNAL(timeout()), this, SLOT(onInitTimer()) );
      myInitTimer->setSingleShot(true);
      myInitTimer->start(0);
    }

  this->VTKConnect = vtkEventQtSlotConnect::New();
  
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  if(pm) {
    vtkPVSession* pvs = dynamic_cast<vtkPVSession*>(pm->GetSession());
    if(pvs) {
      vtkPVProgressHandler* ph = pvs->GetProgressHandler();
      if(ph) {
          this->VTKConnect->Connect(ph, vtkCommand::StartEvent,
                                    this, SLOT(onStartProgress()));
          this->VTKConnect->Connect(ph, vtkCommand::EndEvent,
                                    this, SLOT(onEndProgress()));
      }
    }
  }
  connect( application(), SIGNAL( appClosed() ), this, SLOT( onStopTrace() ) );
}

/*!
 * \brief Slot called when the progress bar starts.
 */
void PVGUI_Module::onStartProgress()
{
  // VSR 19/03/2015, issue 0023025
  // next line is commented: it is bad idea to show wait cursor on ANY vtk event
  // moreover, it does not work when running pvserver with --multi-client mode
  //QApplication::setOverrideCursor(Qt::WaitCursor);
}

/*!
 * \brief Slot called when the progress bar is done.
 */
void PVGUI_Module::onEndProgress()
{
  // VSR 19/03/2015, issue 0023025
  // next line is commented: it is bad idea to show wait cursor on ANY vtk event
  // moreover, it does not work when running pvserver with --multi-client mode
  //QApplication::restoreOverrideCursor();
}

void PVGUI_Module::onDataRepresentationUpdated() {
  LightApp_Study* activeStudy = dynamic_cast<LightApp_Study*>(application()->activeStudy());
  if(!activeStudy) return;
  
  activeStudy->Modified();
}

/*!
  \brief Initialisation timer event - Starts up the Python trace
*/
void PVGUI_Module::onInitTimer()
{
  startTrace();
}
  
/*!
  \brief Get list of embedded macros files
*/
QStringList PVGUI_Module::getEmbeddedMacrosList()
{
  QString aRootDir = getenv("PARAVIS_ROOT_DIR");

  QString aSourcePath = aRootDir + "/bin/salome/Macro";

  QStringList aFilter;
  aFilter << "*.py";

  QDir aSourceDir(aSourcePath);
  QStringList aSourceFiles = aSourceDir.entryList(aFilter, QDir::Files);
  QStringList aFullPathSourceFiles;
  foreach (QString aMacrosName, aSourceFiles) {
    aFullPathSourceFiles << aSourceDir.absoluteFilePath(aMacrosName);
  }
  return aFullPathSourceFiles;
}

/*!
  \brief Update the list of embedded macros
*/
void PVGUI_Module::updateMacros()
{
  pqPythonManager* aPythonManager = pqPVApplicationCore::instance()->pythonManager();
  if(!aPythonManager)  {
    return;
  }
  
  foreach (QString aStr, getEmbeddedMacrosList()) {
    aPythonManager->addMacro(aStr);
  }
}


/*!
  \brief Get list of compliant dockable GUI elements
  \param m map to be filled in ("type":"default_position")
*/
void PVGUI_Module::windows( QMap<int, int>& m ) const
{
  m.insert( LightApp_Application::WT_ObjectBrowser, Qt::LeftDockWidgetArea );
#ifndef DISABLE_PYCONSOLE
  m.insert( LightApp_Application::WT_PyConsole, Qt::BottomDockWidgetArea );
#endif
  // ParaView diagnostic output redirected here
  m.insert( LightApp_Application::WT_LogWindow, Qt::BottomDockWidgetArea );
}

/*!
  \brief Shows (toShow = true) or hides ParaView view window
*/
void PVGUI_Module::showView( bool toShow )
{
  // VSR: TODO: all below is not needed, if we use standard approach
  // that consists in implementing viewManagers() function properly
  // This should be done after we decide what to do with Log window.
  LightApp_Application* anApp = getApp();
  PVViewer_ViewManager* viewMgr =
    dynamic_cast<PVViewer_ViewManager*>( anApp->getViewManager( PVViewer_Viewer::Type(), false ) );
  if ( !viewMgr ) {
    viewMgr = new PVViewer_ViewManager( anApp->activeStudy(), anApp->desktop() );
    anApp->addViewManager( viewMgr );
    connect( viewMgr, SIGNAL( lastViewClosed( SUIT_ViewManager* ) ),
             anApp, SLOT( onCloseView( SUIT_ViewManager* ) ) );
  }

  PVViewer_ViewWindow* pvWnd = dynamic_cast<PVViewer_ViewWindow*>( viewMgr->getActiveView() );
  if ( !pvWnd ) {
    pvWnd = dynamic_cast<PVViewer_ViewWindow*>( viewMgr->createViewWindow() );
    // this also connects to the pvserver and instantiates relevant PV behaviors
  }

  pvWnd->setVisible( toShow );
  if ( toShow ) pvWnd->setFocus();
}

/*!
  \brief Slot to show help for proxy.
*/
void PVGUI_Module::showHelpForProxy( const QString& groupname, const QString& proxyname )
{
  pqHelpReaction::showProxyHelp(groupname, proxyname);
}

/*!
  \brief Slot to show the waiting state.
*/
void PVGUI_Module::onPreAccept()
{
  getApp()->desktop()->statusBar()->showMessage(tr("STB_PREACCEPT"));
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

/*!
  \brief Slot to show the ready state.
*/
void PVGUI_Module::onPostAccept()
{
  getApp()->desktop()->statusBar()->showMessage(tr("STB_POSTACCEPT"), 2000);
  QTimer::singleShot(0, this, SLOT(endWaitCursor()));
}

/*!
  \brief Slot to switch off wait cursor.
*/
void PVGUI_Module::endWaitCursor()
{
  QApplication::restoreOverrideCursor();
}

/*!
  \brief Activate module.
  \param study current study
  \return \c true if activaion is done successfully or 0 to prevent
  activation on error
*/
bool PVGUI_Module::activateModule( SUIT_Study* study )
{
  SUIT_ExceptionHandler::addCleanUpRoutine( paravisCleanUp );

  storeCommonWindowsState();

  bool isDone = LightApp_Module::activateModule( study );
  if ( !isDone ) return false;

  showView( true );
  if ( mySourcesMenuId != -1 ) menuMgr()->show(mySourcesMenuId);
  if ( myFiltersMenuId != -1 ) menuMgr()->show(myFiltersMenuId);
  if ( myMacrosMenuId != -1 ) menuMgr()->show(myMacrosMenuId);
  if ( myCatalystMenuId != -1 ) menuMgr()->show(myCatalystMenuId);

  // Update the various menus with the content pre-loaded in myGuiElements
//  QMenu* srcMenu = menuMgr()->findMenu( mySourcesMenuId );
//  myGuiElements->updateSourcesMenu(srcMenu);
//  QMenu* filtMenu = menuMgr()->findMenu( myFiltersMenuId );
//  myGuiElements->updateFiltersMenu(filtMenu);
//  QMenu* macMenu = menuMgr()->findMenu( myMacrosMenuId );
//  myGuiElements->updateMacrosMenu(macMenu);

  setMenuShown( true );
  setToolShown( true );

  restoreDockWidgetsState();

  QMenu* aMenu = menuMgr()->findMenu( myRecentMenuId );
  if(aMenu) {
    QList<QAction*> anActns = aMenu->actions();
    for (int i = 0; i < anActns.size(); ++i) {
      QAction* a = anActns.at(i);
      if(a)
        a->setVisible(true);
    }
  }

  QList<QMenu*>::iterator it;
  for (it = myMenus.begin(); it != myMenus.end(); ++it) {
    QAction* a = (*it)->menuAction();
    if(a)
      a->setVisible(true);
  }

  if ( myRecentMenuId != -1 ) menuMgr()->show(myRecentMenuId);

  // VSR 18/10/2018 - 0023170: Workaround to re-select current index after module activation
  QItemSelectionModel* selection_model = myGuiElements->getPipelineBrowserWidget()->getSelectionModel();
  QModelIndex idx = selection_model->currentIndex();
  selection_model->clearCurrentIndex();
  selection_model->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);

  return isDone;
}


/*!
  \brief Deactivate module.
  \param study current study
  \return \c true if deactivaion is done successfully or 0 to prevent
  deactivation on error
*/
bool PVGUI_Module::deactivateModule( SUIT_Study* study )
{
  MESSAGE("PARAVIS deactivation ...")

  QMenu* aMenu = menuMgr()->findMenu( myRecentMenuId );
  if(aMenu) {
    QList<QAction*> anActns = aMenu->actions();
    for (int i = 0; i < anActns.size(); ++i) {
      QAction* a = anActns.at(i);
      if(a)
        a->setVisible(false);
    }
  }

  QList<QMenu*>::iterator it;
  for (it = myMenus.begin(); it != myMenus.end(); ++it) {
    QAction* a = (*it)->menuAction();
    if(a)
      a->setVisible(false);
  }

  QList<QDockWidget*> aStreamingViews = application()->desktop()->findChildren<QDockWidget*>("pqStreamingControls");
  foreach(QDockWidget* aView, aStreamingViews) {
    if (!myDockWidgets.contains(aView))
      myDockWidgets[aView] = aView->isVisible();
  }

  /*if (pqImplementation::helpWindow) {
    pqImplementation::helpWindow->hide();
    }*/
  // hide menus
  menuMgr()->hide(myRecentMenuId);
  menuMgr()->hide(mySourcesMenuId);
  menuMgr()->hide(myFiltersMenuId);
  menuMgr()->hide(myMacrosMenuId);
  menuMgr()->hide(myCatalystMenuId);
  setMenuShown( false );
  setToolShown( false );

  saveDockWidgetsState();

  SUIT_ExceptionHandler::removeCleanUpRoutine( paravisCleanUp );

  if (myOldMsgHandler)
    qInstallMessageHandler(myOldMsgHandler);

  restoreCommonWindowsState();
  
  return LightApp_Module::deactivateModule( study );
}


/*!
  \brief Called when application is closed.

  Process finalize application functionality from ParaView in order to save server settings
  and nullify application pointer if the application is being closed.

  \param theApp application
*/
void PVGUI_Module::onApplicationClosed( SUIT_Application* theApp )
{
  PVViewer_Core::ParaviewCleanup();
  CAM_Module::onApplicationClosed(theApp);
}

/*!
  \brief Called when study is closed.

  Removes data model from the \a study.

  \param study study being closed
*/
void PVGUI_Module::studyClosed(SUIT_Study* study)
{
  showView(false); // VSR: this seems to be not needed (all views are automatically closed)
  clearParaviewState();
  //Re-start trace 
  onRestartTrace();

  LightApp_Module::studyClosed(study);
}

/*!
  \brief Open file of format supported by ParaView
*/
void PVGUI_Module::openFile( const char* theName )
{
  QStringList aFiles;
  aFiles << theName;
  pqLoadDataReaction::loadData( aFiles );
}

/*!
  \brief Starts Python trace.
 
  Start trace invoking the newly introduced C++ API (PV 4.2)
  (inspired from pqTraceReaction::start())
*/
void PVGUI_Module::startTrace()
{
  vtkSMSessionProxyManager* pxm = pqActiveObjects::instance().activeServer()->proxyManager();

  vtkSmartPointer<vtkSMProxy> proxy;
  proxy.TakeReference( pxm->NewProxy( "pythontracing", "PythonTraceOptions" ) );
  if ( proxy ) {
    vtkNew<vtkSMParaViewPipelineController> controller;
    controller->InitializeProxy( proxy );
  }
  vtkSMTrace* trace = vtkSMTrace::StartTrace();
  if ( proxy ) {
    // Set manually the properties entered via the dialog box poping-up when requiring
    // a trace start in PV4.2 (trace options)
    SUIT_ResourceMgr* aResourceMgr = SUIT_Session::session()->resourceMgr();
    int type = aResourceMgr->integerValue( PARAVIS_MODULE_NAME, "tracestate_type", 2 );
    trace->SetPropertiesToTraceOnCreate( type );
    trace->SetFullyTraceSupplementalProxies( false );
  }
}

/*!
  \brief Stops Python trace.
*/
void PVGUI_Module::stopTrace()
{
  vtkSMTrace::StopTrace();
}

/*!
  \brief Execute a Python script.
*/
void PVGUI_Module::executeScript( const char* script )
{
  // ???
  // Not sure this is the right fix, but the PYTHON_MANAGER has a function named
  // executeScript() which seems to do what the runScript on pyShellDialog() class
  // was doing.
#ifndef WNT
  pqPythonManager* manager =
    qobject_cast<pqPythonManager*>(pqApplicationCore::instance()->manager("PYTHON_MANAGER"));

  if ( manager )  {
    manager->executeScript(script);
  }
#endif
  /*
#ifndef WNT
  pqPythonManager* manager = qobject_cast<pqPythonManager*>(
                             pqApplicationCore::instance()->manager( "PYTHON_MANAGER" ) );
  if ( manager )  {
    pqPythonDialog* pyDiag = manager->pythonShellDialog();
    if ( pyDiag ) {
      pyDiag->runString(script);  
    }
  }
#endif
  */
}

///**
// *  Debug function printing out the given interpreter's execution context
// */
//void printInterpContext(PyInterp_Interp * interp )
//{
//  // Extract __smtraceString from interpreter's context
//  const PyObject* ctxt = interp->getExecutionContext();
//
//  PyObject* lst = PyDict_Keys((PyObject *)ctxt);
//  Py_ssize_t siz = PyList_GET_SIZE(lst);
//  for (Py_ssize_t i = 0; i < siz; i++)
//    {
//      PyObject * elem = PyList_GetItem(lst, i);
//      if (PyString_Check(elem))
//        {
//          std::cout << "At pos:" << i << ", " << Py_EncodeLocale(PyUnicode_AS_UNICODE(elem), NULL) << std::endl;
//        }
//      else
//        std::cout << "At pos:" << i << ", not a string!" << std::endl;
//    }
//  Py_XDECREF(lst);
//}

/*!
  \brief Returns trace string
*/
QString PVGUI_Module::getTraceString()
{
  QString traceString = "";

  static const QString replaceStr( "paraview.simple" );
  std::stringstream nl;
  nl << std::endl; // surely there is some Qt trick to do that in a portable way??
  QString end_line( nl.str().c_str() );

  vtkSMTrace* tracer = vtkSMTrace::GetActiveTracer();
  if ( tracer ) {
    traceString = tracer->GetCurrentTrace();
    // 'import pvsimple' is necessary to fix the first call to DisableFirstRenderCamera in the paraview trace
    // 'ShowParaviewView()' ensure there is an opened viewing window (otherwise SEGFAULT!)
    traceString = "import pvsimple" + end_line +
      "pvsimple.ShowParaviewView()" + end_line + traceString;

    // Replace import "paraview.simple" by "pvsimple"
    if ( !traceString.isEmpty() ) {
      int aPos = traceString.indexOf( replaceStr );
      while ( aPos != -1 ) {
        traceString = traceString.replace( aPos, replaceStr.length(), "pvsimple" );
        aPos = traceString.indexOf( replaceStr, aPos );
      }
    }
  }

  // Save camera position to, which is no longer output by the tracer ...
  {
    vtkPythonScopeGilEnsurer psge;
    PyObject * mods(PySys_GetObject(const_cast<char*>("modules")));
    PyObject * trace_mod(PyDict_GetItemString(mods, "paraview.smtrace"));  // module was already (really) imported by vtkSMTrace
    if (trace_mod && trace_mod != Py_None && PyModule_Check(trace_mod)) {
        vtkSmartPyObject save_cam(PyObject_GetAttrString(trace_mod, const_cast<char*>("SaveCameras")));
        vtkSmartPyObject camera_trace(PyObject_CallMethod(save_cam, const_cast<char*>("get_trace"), NULL));
        // Convert to a single string
        vtkSmartPyObject ret(PyUnicode_FromUnicode(Py_DecodeLocale(end_line.toStdString().c_str(), NULL), end_line.size()));
        vtkSmartPyObject final_string(PyObject_CallMethod(ret, const_cast<char*>("join"),
            const_cast<char*>("O"), (PyObject*)camera_trace));
        if (PyUnicode_CheckExact(final_string))
          {
            QString camera_qs(Py_EncodeLocale(PyUnicode_AS_UNICODE(final_string.GetPointer()), NULL));  // deep copy
            traceString = traceString + end_line  + end_line + QString("#### saving camera placements for all active views")
                + end_line + end_line + camera_qs + end_line;
          }
      }
  } 

  return traceString;
}

/*!
  \brief Saves trace string to disk file
*/
void PVGUI_Module::saveTrace( const char* theName )
{
  QFile file( theName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
    MESSAGE( "Could not open file:" << theName );
    return;
  }
  QTextStream out( &file );
  out << getTraceString();
  file.close();
}

/*!
  \brief Saves ParaView state to a disk file
*/
void PVGUI_Module::saveParaviewState( const QString& theFileName )
{
  pqApplicationCore::instance()->saveState( theFileName.toStdString().c_str() );
}

/*!
  \brief Delete all objects for Paraview Pipeline Browser
*/
void PVGUI_Module::clearParaviewState()
{
  QAction* deleteAllAction = action( DeleteAllId );
  if ( deleteAllAction ) {
    deleteAllAction->activate( QAction::Trigger );
  }
}

/*!
  \brief Restores ParaView state from a disk file
*/
void PVGUI_Module::loadParaviewState( const QString& theFileName )
{
  pqApplicationCore::instance()->loadState( theFileName.toStdString().c_str(), getActiveServer() );
}

/*!
  \brief Returns current active ParaView server
*/
pqServer* PVGUI_Module::getActiveServer()
{
  return pqApplicationCore::instance()->getActiveServer();
}


/*!
  \brief Creates PARAVIS preferences panel.
*/
void PVGUI_Module::createPreferences()
{
  QList<QVariant> aIndices;
  QStringList aStrings;

  // Paraview settings tab
  int aParaViewSettingsTab = addPreference( tr( "TIT_PVIEWSETTINGS" ) );

  setPreferenceProperty(aParaViewSettingsTab, "stretch", false );
  int aPanel = addPreference( QString(), aParaViewSettingsTab,
                              LightApp_Preferences::UserDefined, PARAVIS_MODULE_NAME, "" );

  setPreferenceProperty( aPanel, "content", (qint64)( new PVGUI_ParaViewSettingsPane( 0, getApp() ) ) );

  // Paravis settings tab
  int aParaVisSettingsTab = addPreference( tr( "TIT_PVISSETTINGS" ) );

  addPreference( tr( "PREF_NO_EXT_PVSERVER" ), aParaVisSettingsTab, 
                 LightApp_Preferences::Bool, PARAVIS_MODULE_NAME, "no_ext_pv_server" );

  int aSaveType = addPreference( tr( "PREF_SAVE_TYPE_LBL" ), aParaVisSettingsTab,
                                 LightApp_Preferences::Selector,
                                 PARAVIS_MODULE_NAME, "savestate_type" );

  aStrings.clear();
  aIndices.clear();
  aIndices << 0 << 1 << 2;
  aStrings << tr("PREF_SAVE_TYPE_0") << tr("PREF_SAVE_TYPE_1") << tr("PREF_SAVE_TYPE_2");
  setPreferenceProperty( aSaveType, "strings", aStrings );
  setPreferenceProperty( aSaveType, "indexes", aIndices );

  // ... "Language" group <<start>>
  int traceGroup = addPreference( tr( "PREF_GROUP_TRACE" ), aParaVisSettingsTab );

  int stopTrace = addPreference( tr( "PREF_STOP_TRACE" ), traceGroup, 
                                 LightApp_Preferences::Bool, PARAVIS_MODULE_NAME, "stop_trace" );
  setPreferenceProperty( stopTrace, "restart",  true );

  int aTraceType = addPreference( tr( "PREF_TRACE_TYPE_LBL" ), traceGroup,
                                 LightApp_Preferences::Selector,
                                 PARAVIS_MODULE_NAME, "tracestate_type" );
  aStrings.clear();
  aIndices.clear();
  aIndices << 0 << 1 << 2;
  aStrings << tr("PREF_TRACE_TYPE_0") << tr("PREF_TRACE_TYPE_1") << tr("PREF_TRACE_TYPE_2");
  setPreferenceProperty( aTraceType, "strings", aStrings );
  setPreferenceProperty( aTraceType, "indexes", aIndices );
  setPreferenceProperty( aTraceType, "restart",  true );
}

/*!
  \brief. Show ParaView python trace.
*/
void PVGUI_Module::onShowTrace()
{
  if (!myTraceWindow) {
    myTraceWindow = new pqPythonScriptEditor(getApp()->desktop());
  }
  myTraceWindow->setText(getTraceString());
  myTraceWindow->show();
  myTraceWindow->raise();
  myTraceWindow->activateWindow();
}


/*!
  \brief. Re-initialize ParaView python trace.
*/
void PVGUI_Module::onRestartTrace()
{
  stopTrace();
  startTrace();
}

/*!
  \brief. Close ParaView python trace.
*/
void PVGUI_Module::onStopTrace()
{
  stopTrace();
}
/*!
  \brief Called when view manager is added
*/
void PVGUI_Module::onViewManagerAdded( SUIT_ViewManager* vm )
{
  if ( PVViewer_ViewManager* pvvm = dynamic_cast<PVViewer_ViewManager*>( vm ) ) {
    connect( pvvm, SIGNAL( viewCreated( SUIT_ViewWindow* ) ), 
             this, SLOT( onPVViewCreated( SUIT_ViewWindow* ) ) );
    connect( pvvm, SIGNAL( deleteView( SUIT_ViewWindow* ) ),
             this,  SLOT( onPVViewDelete( SUIT_ViewWindow* ) ) );
  }
}

/*!
  \brief Called when view manager is removed
*/
void PVGUI_Module::onViewManagerRemoved( SUIT_ViewManager* vm )
{
  if ( PVViewer_ViewManager* pvvm = dynamic_cast<PVViewer_ViewManager*>( vm ) )
    disconnect( pvvm, SIGNAL( viewCreated( SUIT_ViewWindow* ) ),
                this, SLOT( onPVViewCreated( SUIT_ViewWindow* ) ) );
}

/*!
  \brief Show toolbars at \a vw PV view window creating when PARAVIS is active.
*/
void PVGUI_Module::onPVViewCreated( SUIT_ViewWindow* vw )
{
  myGuiElements->setToolBarVisible( true );
  restoreDockWidgetsState();
}

/*!
  \brief Save toolbars state at \a view view closing.
*/
void PVGUI_Module::onPVViewDelete( SUIT_ViewWindow* view )
{
  if ( dynamic_cast<PVViewer_ViewWindow*>( view ) )
    saveDockWidgetsState( false );
}

/*!
  \fn CAM_Module* createModule();
  \brief Export module instance (factory function).
  \return new created instance of the module
*/

#ifdef WNT
#define PVGUI_EXPORT __declspec(dllexport)
#else   // WNT
#define PVGUI_EXPORT
#endif  // WNT

extern "C" {

  bool flag = false;
  PVGUI_EXPORT CAM_Module* createModule() {
    return new PVGUI_Module();
  }
  
  PVGUI_EXPORT char* getModuleVersion() {
    return (char*)PARAVIS_VERSION_STR;
  }
}
