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
// Author: Adrien Bruneton (CEA)

#include "PVViewer_GUIElements.h"
#include "PLMainWindow.hxx"
#include "PVViewer_Core.h"
#include "PLViewTab.hxx"

#include <iostream>
#include <QObject>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui/QFileDialog>
#else
#include <QtWidgets/QFileDialog>
#endif
#include <QMessageBox>

#include <pqTabbedMultiViewWidget.h>
#include <pqApplicationCore.h>
#include <pqPVApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqLoadDataReaction.h>
#include <pqPipelineSource.h>
#include <pqManagePluginsReaction.h>
#include <pqPropertiesPanel.h>
#include <pqPipelineBrowserWidget.h>
#include <pqServerManagerModel.h>
#include <pqRenderView.h>
#include <pqActiveObjects.h>
#include <pqProxy.h>

#include <vtkSMSourceProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMStringVectorProperty.h>

PLMainWindow::PLMainWindow(QWidget *parent) :
  QMainWindow(parent),
  _pAppC(0),
  _simplePipeline(),
  _autoApply(true)
{
  _mainWindow.setupUi(this);
  _autoApply = _mainWindow.actionAuto_apply->isChecked();
  QObject::connect(this, SIGNAL(apply()), this, SLOT(onApply()));
  QObject::connect(this, SIGNAL(changedCurrentFile(QString)), this, SLOT(onApply()));
  addTab();
}

// Called after ParaView application initialisation:
void PLMainWindow::finishUISetup()
{
  _pAppC = PVViewer_Core::GetPVApplication();
  PVViewer_GUIElements * pvgui = PVViewer_GUIElements::GetInstance(this);
  QWidget * wprop = pvgui->getPropertiesPanel();
  QWidget * wpipe = pvgui->getPipelineBrowserWidget();
  wprop->setParent(_mainWindow.propFrame);
  _mainWindow.verticalLayoutProp->addWidget(wprop);
  wpipe->setParent(_mainWindow.pipelineFrame);
  _mainWindow.verticalLayoutPipe->addWidget(wpipe);

  PVViewer_GUIElements * pvge = PVViewer_GUIElements::GetInstance(this);
//  pvge->setToolBarVisible(false);

  // In this mockup, we play on the parent widget visibility (a QFrame), so show these:
  pvge->getPipelineBrowserWidget()->show();
  pvge->getPropertiesPanel()->show();
  // and hide these:
  _mainWindow.propFrame->hide();
  _mainWindow.pipelineFrame->hide();
//  pvge->setToolBarEnabled(false);
//  pvge->setToolBarVisible(false);

}

void PLMainWindow::autoApplyCheck(bool isChecked)
{
  _autoApply = isChecked;
}

void PLMainWindow::onApply()
{
  if (_autoApply)
    {
      PVViewer_GUIElements * pvgui = PVViewer_GUIElements::GetInstance(this);
      pqPropertiesPanel * wprop = pvgui->getPropertiesPanel();
      wprop->apply();
    }
}

void PLMainWindow::showProp(bool isChecked)
{
  isChecked ? _mainWindow.propFrame->show() : _mainWindow.propFrame->hide();
}

void PLMainWindow::showPipeline(bool isChecked)
{
  isChecked ? _mainWindow.pipelineFrame->show() : _mainWindow.pipelineFrame->hide();
}

void PLMainWindow::addTab()
{
  int c = _mainWindow.tabWidget->count();
  PLViewTab * newTab = new PLViewTab(_mainWindow.tabWidget);
  int newIdx = _mainWindow.tabWidget->addTab(newTab, QString("Tab %1").arg(c+1));
  _mainWindow.tabWidget->setCurrentIndex(newIdx);

  // Connect buttons
  QObject::connect(newTab, SIGNAL(onInsertSingleView(PLViewTab *)), this, SLOT(insertSingleView(PLViewTab *)));
  QObject::connect(newTab, SIGNAL(onInsertMultiView(PLViewTab *)), this, SLOT(insertMultiView(PLViewTab *)));
}

void PLMainWindow::deleteTab()
{
  int c = _mainWindow.tabWidget->currentIndex();
  if (c != -1)
    {
      _mainWindow.tabWidget->removeTab(c);
    }
}

void PLMainWindow::currentTabChanged(int tabIdx)
{
  QWidget * w = _mainWindow.tabWidget->widget(tabIdx);
  if (w)
    {
      PLViewTab * viewtab = qobject_cast<PLViewTab *>(w);
      if (viewtab && viewtab->getpqView())
        {
          pqActiveObjects::instance().setActiveView(viewtab->getpqView());
        }
    }
}

void PLMainWindow::doOpenFile()
{
    // Clean up vizu
//    cleanAll();

    // Load the stuff as wireframe in the main view:
    QList<pqPipelineSource *> many = pqLoadDataReaction::loadData();
    if (many.isEmpty())
    {
      std::cout << "no file selected!" << std::endl;
      return;
    }
    if (many.count() > 1)
      {
        QMessageBox msgBox;
        msgBox.setText("Select one file only!");
        msgBox.exec();
        cleanAll();
        return;
      }

    pqPipelineSource * src = many.at(0);
    std::cout << "num of out ports: " << src->getNumberOfOutputPorts() << std::endl;

    // A cone to start with:
//    pqPipelineSource * src = this->_pAppC->getObjectBuilder()->createSource(QString("sources"), QString("ConeSource"),
//        this->_activeServer);
    if(src)
      _simplePipeline.push(src);

    // Retrieve loaded file name
    vtkSMProperty * prop = src->getSourceProxy()->GetProperty("FileName");
    vtkSMStringVectorProperty * prop2 = vtkSMStringVectorProperty::SafeDownCast(prop);
    QString fName(prop2->GetElement(0));

    // Emit signal
    emit changedCurrentFile(fName);
}

void PLMainWindow::insertSingleView(PLViewTab * tab)
{
  // Create a new view proxy on the server
  pqObjectBuilder* builder = _pAppC->getObjectBuilder();
  pqServer* active_serv = pqActiveObjects::instance().activeServer();

  std::cout << "About to create single view ..." << std::endl;
  pqView * pqview = builder->createView(QString("RenderView"), active_serv);
  std::cout << "Created: " << pqview << "!" << std::endl;

  // Retrieve its widget and pass it to the Qt tab:
  QWidget* viewWidget = pqview->getWidget();

//  QWidget* viewWidget = new QPushButton("toto");
  tab->hideAndReplace(viewWidget, pqview);

  pqActiveObjects::instance().setActiveView(pqview);
}

void PLMainWindow::insertMultiView(PLViewTab * tab)
{
  // Retrieve TabbedMultiView and see if it is already attached to someone:
  PVViewer_GUIElements * pvgui = PVViewer_GUIElements::GetInstance(this);
  pqTabbedMultiViewWidget * multiv = pvgui->getTabbedMultiViewWidget();

  QWidget * parent = multiv->nativeParentWidget();
  if (parent)
    {
      QMessageBox msgBox;
      msgBox.setText("Multi-view already in use in another tab! Close it first.");
      msgBox.exec();
    }
  else
    {
      tab->hideAndReplace(multiv, NULL);
    }
}


void PLMainWindow::doShrink()
{
  if(!_simplePipeline.isEmpty())
  {
    cleanAllButSource();

    pqPipelineSource * src = this->_pAppC->getObjectBuilder()->createFilter(QString("filters"),
        QString("ShrinkFilter"), _simplePipeline.top());
    if(src)
      _simplePipeline.push(src);

    // Hit apply
    emit apply();
  }
}

void PLMainWindow::doSlice()
{
  if(!_simplePipeline.isEmpty())
  {
    cleanAllButSource();

    pqPipelineSource * src = this->_pAppC->getObjectBuilder()->createFilter(QString("filters"),
        QString("Cut"), _simplePipeline.top());
    if(src)
      _simplePipeline.push(src);

    // Hit apply
    emit apply();
  }
}

void PLMainWindow::doManagePlugins()
{
   pqManagePluginsReaction::managePlugins();
}

void PLMainWindow::cleanAll()
{
  this->_pAppC->getObjectBuilder()->destroyPipelineProxies();
  _simplePipeline.resize(0);
}

void PLMainWindow::cleanAllButSource()
{
  while(_simplePipeline.size() > 1)
    this->_pAppC->getObjectBuilder()->destroy(_simplePipeline.pop());
}
