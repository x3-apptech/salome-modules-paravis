// PARAVIS : ParaView wrapper SALOME module
//
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
// File   : PVGUI_Module_MenuActions.cxx
// Author : Margarita KARPUNINA
//

#include "PVGUI_Module.h"
#include <PVViewer_GUIElements.h>
#include <PVViewer_ViewManager.h>

#include <QtxAction.h>
#include <QtxActionMenuMgr.h>
#include <QtxActionToolMgr.h>
#include <SUIT_Desktop.h>
#include <SUIT_ResourceMgr.h>
#include <SUIT_Session.h>
#include <LightApp_Application.h>

#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QFile>

#include <pqApplicationCore.h>

#include <pqSaveStateReaction.h>
#include <pqLoadDataReaction.h>
#include <pqLoadStateReaction.h>
#include <pqSaveDataReaction.h>
#include <pqSaveScreenshotReaction.h>
#include <pqExportReaction.h>
#include <pqSaveAnimationReaction.h>
#include <pqSaveAnimationGeometryReaction.h>
#include <pqServerConnectReaction.h>
#include <pqServerDisconnectReaction.h>
#include <pqCameraUndoRedoReaction.h>
#include <pqUndoRedoReaction.h>
#include <pqDeleteReaction.h>
#include <pqChangePipelineInputReaction.h>
#include <pqApplicationSettingsReaction.h>
#include <pqIgnoreSourceTimeReaction.h>
#include <pqCreateCustomFilterReaction.h>
#include <pqManageLinksReaction.h>
#include <pqManagePluginsReaction.h>
#include <pqCameraLinkReaction.h>
#include <pqManageCustomFiltersReaction.h>
#include <pqTimerLogReaction.h>
#include <pqTestingReaction.h>
#include <pqAboutDialogReaction.h>
#include <pqHelpReaction.h>
#include <pqDataQueryReaction.h>
//#include <pqPythonShellReaction.h>

#include <pqViewManager.h>
#include <pqViewMenuManager.h>
#include <pqParaViewMenuBuilders.h>
#include <pqProxyGroupMenuManager.h>
#include <pqSetName.h>
#include <pqRecentFilesMenu.h>
#include <pqPipelineBrowserWidget.h>

/*!
  \brief Create actions for ParaView GUI operations
  duplicating menus and toolbars in MainWindow class of
  the standard ParaView GUI client application.
*/
void PVGUI_Module::pvCreateActions()
{
  SUIT_ResourceMgr* resMgr = SUIT_Session::session()->resourceMgr();

  QPixmap aPixmap;
  QAction* anAction;
  
  // --- Menu "File"

  // Open File
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_OPEN_FILE"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_OPEN_FILE"), this);
  anAction->setToolTip(tr("TOP_OPEN_FILE"));
  anAction->setStatusTip(tr("STB_OPEN_FILE"));
  registerAction(OpenFileId, anAction);
  new pqLoadDataReaction(anAction);
  
  // Load State
  anAction = new QAction(tr("MEN_LOAD_STATE"), this);
  anAction->setToolTip(tr("TOP_LOAD_STATE"));
  anAction->setStatusTip(tr("STB_LOAD_STATE"));
  registerAction(LoadStateId, anAction);
  new pqLoadStateReaction(anAction);
  
  // Save State
  anAction = new QAction(tr("MEN_SAVE_STATE"), this);
  anAction->setToolTip(tr("TOP_SAVE_STATE"));
  anAction->setStatusTip(tr("STB_SAVE_STATE"));
  registerAction(SaveStateId, anAction);
  new pqSaveStateReaction(anAction);

  // Save Data
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_SAVE_DATA"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_SAVE_DATA"), this);
  anAction->setToolTip(tr("TOP_SAVE_DATA"));
  anAction->setStatusTip(tr("STB_SAVE_DATA"));
  registerAction(SaveDataId, anAction);
  new pqSaveDataReaction(anAction);

  // Save Screenshot
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_SAVE_SCREENSHOT"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_SAVE_SCREENSHOT"), this);
  anAction->setToolTip(tr("TOP_SAVE_SCREENSHOT"));
  anAction->setStatusTip(tr("STB_SAVE_SCREENSHOT"));
  registerAction(SaveScreenshotId, anAction);
  new pqSaveScreenshotReaction(anAction);

  // Export
  anAction = new QAction(tr("MEN_EXPORT"), this);
  anAction->setToolTip(tr("TOP_EXPORT"));
  anAction->setStatusTip(tr("STB_EXPORT"));
  registerAction(ExportId, anAction);
  new pqExportReaction(anAction);

  // Save Animation
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_SAVE_ANIMATION"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_SAVE_ANIMATION"), this);
  anAction->setToolTip(tr("TOP_SAVE_ANIMATION"));
  anAction->setStatusTip(tr("STB_SAVE_ANIMATION"));
  registerAction(SaveAnimationId, anAction);
  new pqSaveAnimationReaction(anAction);

  // Save Geometry
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_SAVE_GEOMETRY"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_SAVE_GEOMETRY"), this);
  anAction->setToolTip(tr("TOP_SAVE_GEOMETRY"));
  anAction->setStatusTip(tr("STB_SAVE_GEOMETRY"));
  registerAction(SaveGeometryId, anAction);
  new pqSaveAnimationGeometryReaction(anAction);

  // Connect
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_CONNECT"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_CONNECT"), this);
  anAction->setToolTip(tr("TOP_CONNECT"));
  anAction->setStatusTip(tr("STB_CONNECT"));
  registerAction(ConnectId, anAction);
  new pqServerConnectReaction(anAction);

  // Disconnect
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_DISCONNECT"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_DISCONNECT"), this);
  anAction->setToolTip(tr("TOP_DISCONNECT"));
  anAction->setStatusTip(tr("STB_DISCONNECT"));
  registerAction(DisconnectId, anAction);
  new pqServerDisconnectReaction(anAction);

  // --- Menu "Edit"

  // Undo
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_UNDO"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_UNDO"), this);
  anAction->setToolTip(tr("TOP_UNDO"));
  anAction->setStatusTip(tr("STB_UNDO"));
  registerAction(UndoId, anAction);
  new pqUndoRedoReaction(anAction, true);

  // Redo
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_REDO"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_REDO"), this);
  anAction->setToolTip(tr("TOP_REDO"));
  anAction->setStatusTip(tr("STB_REDO"));
  registerAction(RedoId, anAction);
  new pqUndoRedoReaction(anAction, false);

  // Camera Undo
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_CAMERA_UNDO"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_CAMERA_UNDO"), this);
  anAction->setToolTip(tr("TOP_CAMERA_UNDO"));
  anAction->setStatusTip(tr("STB_CAMERA_UNDO"));
  registerAction(CameraUndoId, anAction);
  new pqCameraUndoRedoReaction(anAction, true);
  
  // Camera Redo
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_CAMERA_REDO"), false );
  anAction = new QAction(QIcon(aPixmap), tr("MEN_CAMERA_REDO"), this);
  anAction->setToolTip(tr("TOP_CAMERA_REDO"));
  anAction->setStatusTip(tr("STB_CAMERA_REDO"));
  registerAction(CameraRedoId, anAction);
  new pqCameraUndoRedoReaction(anAction, false);

  // Find Data
  anAction = new QAction(tr("MEN_FIND_DATA"), this);
  anAction->setToolTip("");
  anAction->setStatusTip("");
  registerAction(FindDataId, anAction);
  new pqDataQueryReaction(anAction);
  
  // Change Input
  anAction = new QAction(tr("MEN_CHANGE_INPUT"), this);
  anAction->setToolTip(tr("TOP_CHANGE_INPUT"));
  anAction->setStatusTip(tr("STB_CHANGE_INPUT"));
  registerAction(ChangeInputId, anAction);
  new pqChangePipelineInputReaction(anAction);

  // Ignore source time
  anAction = new QAction(tr("MEN_IGNORE_TIME"), this);
  anAction->setToolTip(tr("TOP_IGNORE_TIME"));
  anAction->setStatusTip(tr("STB_IGNORE_TIME"));
  anAction->setCheckable(true);
  registerAction(IgnoreTimeId, anAction);
  new pqIgnoreSourceTimeReaction(anAction);

  // Delete
  anAction = new QAction(tr("MEN_DELETE"), this);
  anAction->setToolTip(tr("TOP_DELETE"));
  anAction->setStatusTip(tr("STB_DELETE"));
  registerAction(DeleteId, anAction);
  new pqDeleteReaction(anAction);

  // Delete All
  anAction = new QAction(tr("MEN_DELETE_ALL"), this);
  anAction->setToolTip(tr("TOP_DELETE_ALL"));
  anAction->setStatusTip(tr("STB_DELETE_ALL"));
  registerAction(DeleteAllId, anAction);
  new pqDeleteReaction(anAction, true);


  // Setting
  /*anAction = new QAction(tr("MEN_SETTINGS"), this);
  anAction->setToolTip(tr("TOP_SETTINGS"));
  anAction->setStatusTip(tr("STB_SETTINGS"));
  registerAction(SettingsId, anAction);
  new pqApplicationSettingsReaction(anAction);*/
  
  // View Settings
//  anAction = new QAction(tr("MEN_VIEW_SETTINGS"), this);
//  anAction->setToolTip(tr("TOP_VIEW_SETTINGS"));
//  anAction->setStatusTip(tr("STB_VIEW_SETTINGS"));
//  registerAction(ViewSettingsId, anAction);
//  new pqViewSettingsReaction(anAction);

  // --- Menu "View"
  //pqViewManager* viewManager = qobject_cast<pqViewManager*>(
  //                             pqApplicationCore::instance()->manager("MULTIVIEW_WIDGET"));

  //rnv: Commented to implement issue 
  //21318: EDF 1615 ALL: Display in full screen mode
  //Switching to the "Full screen" mode added in the SALOME GUI module.
  //if (viewManager) {
  //anAction = new QAction("Full Screen", this);
  //anAction->setObjectName("actionFullScreen");
  //anAction->setShortcut(QKeySequence("F11"));
  //connect(anAction, SIGNAL(triggered()), viewManager, SLOT(toggleFullScreen()));
  //registerAction(FullScreenId, anAction);
  //}

  // --- Menu "Tools"
  // Create Custom Filter
  anAction = new QAction(tr("MEN_CREATE_CUSTOM_FILTER"), this);
  anAction->setToolTip(tr("TOP_CREATE_CUSTOM_FILTER"));
  anAction->setStatusTip(tr("STB_CREATE_CUSTOM_FILTER"));
  registerAction(CreateCustomFilterId, anAction);
  new pqCreateCustomFilterReaction(anAction << pqSetName("actionToolsCreateCustomFilter"));

  // Manage Custom Filters
  anAction = new QAction(tr("MEN_MANAGE_CUSTOM_FILTERS"), this);
  anAction->setToolTip(tr("TOP_MANAGE_CUSTOM_FILTERS"));
  anAction->setStatusTip(tr("STB_MANAGE_CUSTOM_FILTERS"));
  registerAction(ManageCustomFiltersId, anAction);
  new pqManageCustomFiltersReaction(anAction << pqSetName("actionToolsManageCustomFilters"));

  // Manage Links
  anAction = new QAction(tr("MEN_MANAGE_LINKS"), this);
  anAction->setToolTip(tr("TOP_MANAGE_LINKS"));
  anAction->setStatusTip(tr("STB_MANAGE_LINKS"));
  registerAction(ManageLinksId, anAction);
  new pqManageLinksReaction(anAction << pqSetName("actionToolsManageLinks"));

  // Add Camera Link
  anAction = new QAction(tr("MEN_ADD_CAMERA_LINK"), this);
  anAction->setToolTip(tr("TOP_ADD_CAMERA_LINK"));
  anAction->setStatusTip(tr("STB_ADD_CAMERA_LINK"));
  registerAction(AddCameraLinkId, anAction);
  new pqCameraLinkReaction(anAction << pqSetName("actionToolsAddCameraLink"));

  // Manage Plugins/Extensions
  anAction = new QAction(tr("MEN_MANAGE_PLUGINS"), this);
  anAction->setToolTip(tr("TOP_MANAGE_PLUGINS"));
  anAction->setStatusTip(tr("STB_MANAGE_PLUGINS"));
  registerAction(ManagePluginsExtensionsId, anAction);
  new pqManagePluginsReaction(anAction << pqSetName("actionManage_Plugins"));

  // Record Test
  anAction = new QAction(tr("MEN_RECORD_TEST"), this);
  anAction->setToolTip(tr("TOP_RECORD_TEST"));
  anAction->setStatusTip(tr("STB_RECORD_TEST"));
  registerAction(RecordTestId, anAction);
  new pqTestingReaction(anAction << pqSetName("actionToolsRecordTest"), pqTestingReaction::RECORD);

  // Play Test
  anAction = new QAction(tr("MEN_PLAY_TEST"), this);
  anAction->setToolTip(tr("TOP_PLAY_TEST"));
  anAction->setStatusTip(tr("STB_PLAY_TEST"));
  registerAction(PlayTestId, anAction);
  new pqTestingReaction(anAction << pqSetName("actionToolsPlayTest"), pqTestingReaction::PLAYBACK);

  // Max Window Size
  anAction = new QAction(tr("MEN_MAX_WINDOW_SIZE"), this);
  anAction->setToolTip(tr("TOP_MAX_WINDOW_SIZE"));
  anAction->setStatusTip(tr("PRP_APP_MAX_WINDOW_SIZE"));
  anAction->setCheckable(true);
  registerAction(MaxWindowSizeId, anAction);
  new pqTestingReaction(anAction << pqSetName("actionTesting_Window_Size"),
                        pqTestingReaction::LOCK_VIEW_SIZE);

  // Custom Window Size
  anAction = new QAction(tr("MEN_CUSTOM_WINDOW_SIZE"), this);
  anAction->setToolTip(tr(""));
  anAction->setStatusTip(tr(""));
  anAction->setCheckable(true);
  registerAction(CustomWindowSizeId, anAction);
  new pqTestingReaction(anAction << pqSetName("actionTesting_Window_Size_Custom"),
                        pqTestingReaction::LOCK_VIEW_SIZE_CUSTOM);

  // Timer Log
  anAction = new QAction(tr("MEN_TIMER_LOG"), this);
  anAction->setToolTip(tr("TOP_TIMER_LOG"));
  anAction->setStatusTip(tr("STB_TIMER_LOG"));
  registerAction(TimerLogId, anAction);
  new pqTimerLogReaction(anAction << pqSetName("actionToolsTimerLog"));

  // Python Shell
  // ???
  // No more pqPythonShellReaction class, but there is a pqPythonShell class.
  // Is that equivalent? I don't know what to do at this stage.
  anAction = new QAction(tr("MEN_PYTHON_SHELL"), this);
  anAction->setToolTip(tr("TOP_PYTHON_SHELL"));
  anAction->setStatusTip(tr("STB_PYTHON_SHELL"));
  registerAction(PythonShellId, anAction);
  /*
  new pqPythonShellReaction(anAction << pqSetName("actionToolsPythonShell"));
  */

  //Show Trace
  anAction = new QAction(tr("MEN_SHOW_TRACE"), this);
  anAction->setToolTip(tr("TOP_SHOW_TRACE"));
  anAction->setStatusTip(tr("STB_SHOW_TRACE"));
  connect(anAction, SIGNAL(triggered()), this, SLOT(onShowTrace()));
  registerAction(ShowTraceId, anAction);

  //Restart Trace
  anAction = new QAction(tr("MEN_RESTART_TRACE"), this);
  anAction->setToolTip(tr("TOP_RESTART_TRACE"));
  anAction->setStatusTip(tr("STB_RESTART_TRACE"));
  connect(anAction, SIGNAL(triggered()), this, SLOT(onRestartTrace()));
  registerAction(RestartTraceId, anAction);

  // --- Menu "Help"

  // About
  anAction = new QAction(tr("MEN_ABOUT"), this);
  anAction->setToolTip(tr("TOP_ABOUT"));
  anAction->setStatusTip(tr("STB_ABOUT"));
  registerAction(AboutParaViewId, anAction);
  new pqAboutDialogReaction(anAction << pqSetName("actionAbout"));

#ifdef HAS_PV_DOC
  // Native ParaView user documentation
  aPixmap = resMgr->loadPixmap( "ParaView", tr("ICON_PARAVIEW_HELP"), false );
  anAction = new QAction(tr("MEN_PARAVIEW_HELP"), this);
  anAction->setToolTip(tr("TOP_PARAVIEW_HELP"));
  anAction->setStatusTip(tr("STB_PARAVIEW_HELP"));
  registerAction(ParaViewHelpId, anAction);
  new pqHelpReaction(anAction);
#endif
}

/*!
  \brief Create menus for ParaView GUI operations
  duplicating menus in pqMainWindow ParaView class

  In particular, ParaView is responsible for updating "Sources" and "Filters" menus. 
  For this, specific menu managers created by pqMainWindowCore class are used, and PVGUI_Module
  is responsible for creation of corresponding QMenu objects only.
*/
void PVGUI_Module::pvCreateMenus()
{
  SUIT_Desktop* desk = application()->desktop();
  
  // --- Menu "File"
  int aPVMnu = createMenu( tr( "MEN_DESK_FILE" ), -1, -1 );

  createMenu( OpenFileId, aPVMnu, 5 );

  // Recent Files
   myRecentMenuId = createMenu( tr( "MEN_RECENT_FILES" ), aPVMnu, -1, 5 );
   QMenu* aMenu = menuMgr()->findMenu( myRecentMenuId );
   /*pqRecentFilesMenu* aRecentFilesMenu = */new pqRecentFilesMenu( *aMenu, getApp()->desktop() );
   QList<QAction*> anActns = aMenu->actions();
   for (int i = 0; i < anActns.size(); ++i)
     createMenu( anActns.at(i), myRecentMenuId );

  createMenu( separator(), aPVMnu, -1, 5 );

  createMenu( LoadStateId, aPVMnu, 15 );
  createMenu( SaveStateId, aPVMnu, 15 );
  createMenu( separator(), aPVMnu, -1, 15 );

  createMenu( SaveDataId, aPVMnu, 25 );
  createMenu( SaveScreenshotId, aPVMnu, 25 );
  createMenu( ExportId, aPVMnu, 25 );
  createMenu( separator(), aPVMnu, -1, 25 );

  createMenu( SaveAnimationId, aPVMnu, 35 );
  createMenu( SaveGeometryId, aPVMnu, 35 );
  createMenu( separator(), aPVMnu, -1, 35 );

  createMenu( ConnectId, aPVMnu, 45 );
  createMenu( DisconnectId, aPVMnu, 45 );
  createMenu( separator(), aPVMnu, -1, 45 );

  // --- Menu "Edit"
  aPVMnu = createMenu( tr( "MEN_DESK_EDIT" ), -1, -1 );

  createMenu( UndoId, aPVMnu );
  createMenu( RedoId, aPVMnu );
  createMenu( separator(), aPVMnu );

  createMenu( CameraUndoId, aPVMnu );
  createMenu( CameraRedoId, aPVMnu );
  createMenu( separator(), aPVMnu );

  createMenu( FindDataId, aPVMnu );
  createMenu( ChangeInputId, aPVMnu );
  createMenu( IgnoreTimeId, aPVMnu );
  createMenu( DeleteId, aPVMnu );
  createMenu( DeleteAllId, aPVMnu );
  createMenu( separator(), aPVMnu );

  //createMenu( SettingsId, aPVMnu );
  //createMenu( ViewSettingsId, aPVMnu );
  createMenu( separator(), aPVMnu );

  // --- Menu "View"
  aPVMnu = createMenu( tr( "MEN_DESK_VIEW" ), -1, -1 );

  createMenu( FullScreenId, aPVMnu );
  
  // --- Menu "Sources"
  // Install ParaView managers for "Sources" menu
  QMenu* aRes = 0;
  PVViewer_GUIElements * guiElements = PVViewer_GUIElements::GetInstance(desk);
  aRes = guiElements->getSourcesMenu();
  mySourcesMenuId = createMenu( tr( "MEN_DESK_SOURCES" ), -1, -1, 60, -1, aRes);
  
  // --- Menu "Filters"
  // Install ParaView managers for "Filters" menu
  aRes = guiElements->getFiltersMenu();
  myFiltersMenuId = createMenu( tr( "MEN_DESK_FILTERS" ), -1, -1, 70, -1, aRes);

   // --- Menu "Macros"
  aRes = guiElements->getMacrosMenu();
  myMacrosMenuId = createMenu( tr( "MEN_MACROS" ), -1, -1, 80, -1, aRes);
 
  // --- Menu "Tools"
  int aToolsMnu = createMenu( tr( "MEN_DESK_TOOLS" ), -1, -1, 90 );

  createMenu( CreateCustomFilterId, aToolsMnu );
  createMenu( AddCameraLinkId, aToolsMnu );
  createMenu( separator(), aToolsMnu );
  createMenu( ManageCustomFiltersId, aToolsMnu );
  createMenu( ManageLinksId, aToolsMnu );
  createMenu( ManagePluginsExtensionsId, aToolsMnu );
  createMenu( separator(), aToolsMnu );

  createMenu( RecordTestId, aToolsMnu );
  createMenu( PlayTestId, aToolsMnu );
  createMenu( MaxWindowSizeId, aToolsMnu );
  createMenu( CustomWindowSizeId, aToolsMnu );
  createMenu( separator(), aToolsMnu );

  createMenu( TimerLogId, aToolsMnu );
  createMenu( separator(), aToolsMnu );

  createMenu( PythonShellId, aToolsMnu );
  createMenu( separator(), aToolsMnu );
  createMenu( ShowTraceId, aToolsMnu );
  createMenu( RestartTraceId, aToolsMnu );

  // --- Menu "Catalyst"
  aRes = guiElements->getCatalystMenu();
  myCatalystMenuId = createMenu( tr( "MEN_DESK_CATALYST" ), -1, -1, 100, -1, aRes);

  // --- Menu "Help"

  int aHelpMnu = createMenu( tr( "MEN_DESK_HELP" ), -1, -1 );
  int aUsersGuide = createMenu( tr( "User's Guide" ), aHelpMnu, -1 );
  int aPVHelpMnu = createMenu( tr( "ParaViS module" ), aUsersGuide, -1, 5 );
#ifdef HAS_PV_DOC
  createMenu( ParaViewHelpId,  aPVHelpMnu );
  createMenu( separator(),     aPVHelpMnu );
#endif
  createMenu( AboutParaViewId, aPVHelpMnu );

  // -- Context menu in the pipeline browser
  pqPipelineBrowserWidget * pq = guiElements->getPipelineBrowserWidget();
  pqParaViewMenuBuilders::buildPipelineBrowserContextMenu( *(pq->contextMenu()) );
}

/*!
  \brief Create toolbars for ParaView GUI operations
  duplicating toolbars in pqMainWindow ParaView class
*/
void PVGUI_Module::pvCreateToolBars()
{
  SUIT_Desktop* desk = application()->desktop();
//  pqParaViewMenuBuilders::buildToolbars(*desk);
  PVViewer_GUIElements * guiElements = PVViewer_GUIElements::GetInstance(desk);
  guiElements->setToolBarVisible(true);
}
