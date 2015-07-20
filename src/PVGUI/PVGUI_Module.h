// PARAVIS : ParaView wrapper SALOME module
//
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
// File   : PVGUI_Module.h
// Author : Sergey ANIKIN, Adrien BRUNETON
//


#ifndef PVGUI_Module_H
#define PVGUI_Module_H

#include <LightApp_Module.h>

class QMenu;
class QTimer;
class vtkEventQtSlotConnect;
class pqServer;
class pqPythonScriptEditor;
class pqPVApplicationCore;
class PVViewer_GUIElements;
class PVServer_ServiceWrapper;
class SUIT_ViewWindow;

class PVGUI_Module: public LightApp_Module
{
  Q_OBJECT
   
  //! Menu actions
  enum {
    //-----------
    // Menu "File"
    OpenFileId,
    //-
    LoadStateId,
    SaveStateId,
    //-
    SaveDataId,
    SaveScreenshotId,
    ExportId,
    //-
    SaveAnimationId,
    SaveGeometryId,
    //-
    ConnectId,
    DisconnectId,
    //-----------
    // Menu "Edit"
    UndoId,
    RedoId,
    //-
    CameraUndoId,
    CameraRedoId,
    //-
    FindDataId,
    ChangeInputId,
    IgnoreTimeId,
    DeleteId,
    DeleteAllId,
    //-
    SettingsId,             // not used
    ViewSettingsId,         // not used
    //-----------
    // Menu "View"
    FullScreenId,           // not used
    //-----------
    // Menu "Animation"
    FirstFrameId,           // not used
    PreviousFrameId,        // not used
    PlayId,                 // not used
    NextFrameId,            // not used
    LastFrameId,            // not used
    LoopId,                 // not used
    //-----------
    // Menu "Tools"
    CreateCustomFilterId,
    ManageCustomFiltersId,
    CreateLookmarkId,       // not used
    ManageLinksId,
    AddCameraLinkId,
    ManagePluginsExtensionsId,
    DumpWidgetNamesId,      //  not used
    RecordTestId,
    RecordTestScreenshotId, // not used
    PlayTestId,
    MaxWindowSizeId,
    CustomWindowSizeId,
    TimerLogId,
    OutputWindowId,
    PythonShellId,
    ShowTraceId,
    RestartTraceId,
    //-----------
    // Menu "Help"
    AboutParaViewId,
    ParaViewHelpId,
    EnableTooltipsId,       // not used
  };

public:
  PVGUI_Module();
  ~PVGUI_Module();

  virtual void initialize( CAM_Application* );
  virtual void windows( QMap<int, int>& ) const;

  void openFile( const char* );                   // not used inside PARAVIS
  void executeScript( const char* );              // not used inside PARAVIS
  void saveParaviewState( const char* );          // not used inside PARAVIS
  void loadParaviewState( const char* );          // not used inside PARAVIS
  void clearParaviewState();

  QString getTraceString();
  void startTrace();
  void stopTrace();
  void saveTrace( const char* );

  pqServer* getActiveServer();

  virtual void createPreferences();

  inline static PVServer_ServiceWrapper* GetEngine();
  inline static pqPVApplicationCore* GetPVApplication(); // not used inside PARAVIS

  virtual CAM_DataModel* createDataModel();

private:
  //! Create actions for ParaView GUI operations
  void pvCreateActions();

  //! Create menus for ParaView GUI operations duplicating menus in pqMainWindow ParaView class
  void pvCreateMenus();

  //! Create toolbars for ParaView GUI operations duplicating toolbars in pqMainWindow ParaView class
  void pvCreateToolBars();

  //! Create dock widgets for ParaView widgets
  void setupDockWidgets();

  //! Save states of dockable ParaView widgets
  void saveDockWidgetsState( bool = true );

  //! Restore states of dockable ParaView widgets
  void restoreDockWidgetsState();

  //! Shows or hides ParaView view window
  void showView( bool );    

  //! Get list of embedded macros files
  QStringList getEmbeddedMacrosList();

  //! update macros state
  void updateMacros();

  //! store visibility of the common dockable windows (OB, PyConsole, ... etc.)
  void storeCommonWindowsState();

  //! restore visibility of the common dockable windows (OB, PyConsole, ... etc.)
  void restoreCommonWindowsState();

private slots:
  void showHelpForProxy( const QString&, const QString& );
  
  void onPreAccept();    // not used inside PARAVIS
  void onPostAccept();   // not used inside PARAVIS
  void endWaitCursor();  // not used inside PARAVIS

  void onDataRepresentationUpdated();

  void onStartProgress();
  void onEndProgress();
  void onShowTrace();
  void onRestartTrace();

public slots:
  virtual bool           activateModule( SUIT_Study* );
  virtual bool           deactivateModule( SUIT_Study* );
  virtual void           onApplicationClosed( SUIT_Application* );
  virtual void           studyClosed( SUIT_Study* );

protected slots:
  virtual void           onInitTimer();
  virtual void           onViewManagerAdded( SUIT_ViewManager* );
  virtual void           onViewManagerRemoved( SUIT_ViewManager* );
  virtual void           onPVViewCreated( SUIT_ViewWindow* );
  virtual void           onPVViewDelete( SUIT_ViewWindow* );

private:
  int                    mySourcesMenuId;
  int                    myFiltersMenuId;
  int                    myMacrosMenuId;
  int                    myRecentMenuId;
  
  typedef QMap<QWidget*, bool> WgMap;
  WgMap                  myDockWidgets;
  WgMap                  myToolbars;
  WgMap                  myToolbarBreaks;
  QList<QMenu*>          myMenus;

  typedef QMap<int, bool> DockWindowMap;         
  DockWindowMap           myCommonMap; 

  QtMsgHandler            myOldMsgHandler;

  vtkEventQtSlotConnect*  VTKConnect;

  pqPythonScriptEditor*   myTraceWindow;

  //! Single shot timer used to connect to the PVServer, and start the trace.
  QTimer*                 myInitTimer;

  PVViewer_GUIElements*   myGuiElements;
};

#endif // PVGUI_Module_H
