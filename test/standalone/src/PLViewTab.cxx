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

#include <iostream>
#include "PLViewTab.hxx"

PLViewTab::PLViewTab(QWidget * parent):
QWidget(parent),
_renderView(0)
{
  _viewTab.setupUi(this);
}

void PLViewTab::insertSingleView()
{
  emit onInsertSingleView(this);
}

void PLViewTab::insertMultiView()
{
  emit onInsertMultiView(this);
}

void PLViewTab::hideAndReplace(QWidget * w, pqView * view)
{
  _viewTab.frameButtons->hide();
  w->setParent(_viewTab.frameView);
  QVBoxLayout * vbox = _viewTab.verticalLayoutView;
  vbox->addWidget(w);
  _renderView = view;
}

void PLViewTab::viewDestroyed()
{
  std::cout << "View destroyed!" << std::endl;
}

