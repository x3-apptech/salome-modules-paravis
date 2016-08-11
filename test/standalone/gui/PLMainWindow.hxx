// Copyright (C) 2010-2016  CEA/DEN, EDF R&D
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
// Author: Adrien Bruneton (CEA)

#ifndef PLVIEWWIDGET_H_
#define PLVIEWWIDGET_H_

#include <QMainWindow>
#include "ui_light_para.h"
#include <QStack>

class pqPVApplicationCore;
class pqPipelineSource;
class pqServer;
class pqProxy;

class PLViewTab;
class PVViewer_GUIElements;

/** Main window of the application.
 */
class PLMainWindow: public QMainWindow {
  Q_OBJECT

public:
  PLMainWindow(QWidget * parent=0);
  virtual ~PLMainWindow() {}

  void finishUISetup();

protected:
  void doOpenFile();
  void doManagePlugins();

  void doSlice();
  void doShrink();

  void cleanAll();
  void cleanAllButSource();

signals:
  void changedCurrentFile(QString fileName);
  void apply();   // convenience signal to forward to the real ParaView apply

private slots:
  void autoApplyCheck(bool);
  void onApply();
  void showPipeline(bool);
  void showProp(bool);

  void addTab();
  void deleteTab();

  void onFileOpen() { doOpenFile(); };
  void slice()  { doSlice(); };
  void shrink() { doShrink(); };
  void managePlugins() { doManagePlugins(); };

  void insertSingleView(PLViewTab *);
  void insertMultiView(PLViewTab *);
  void insertSpreadsheetView(PLViewTab *);

  void currentTabChanged(int);

  void onBuildFilterMenu();

private:
  Ui::MainWindow _mainWindow;

  PVViewer_GUIElements * _pvgui;

  pqPVApplicationCore * _pAppC;
  //pqServer * _activeServer;
  //pqPipelineSource * _activeSource;  // last pipeline element
  QStack<pqPipelineSource *> _simplePipeline;

  bool _autoApply;

  QMenu * _filterMenu;
};

#endif /* PLVIEWWIDGET_H_ */
