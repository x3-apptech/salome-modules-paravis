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
// Author: Adrien Bruneton (CEA)

#ifndef SRC_PLVIEWTAB_HXX_
#define SRC_PLVIEWTAB_HXX_

#include "ui_view_tab.h"

class pqView;

/** Widget inserted when a new tab is requested in the application.
 */
class PLViewTab: public QWidget {
  Q_OBJECT

public:
  PLViewTab(QWidget * parent=0);
  virtual ~PLViewTab() {}

  // Hide buttons, and put widget in the QFrame with a vertical layout
  void hideAndReplace(QWidget * w, pqView * view);

  pqView * getpqView() { return _renderView; }

private slots:
  void insertSingleView();
  void insertMultiView();
  void insertSpreadsheetView();
  void viewDestroyed();

signals:
  void onInsertSingleView(PLViewTab *);
  void onInsertMultiView(PLViewTab *);
  void onInsertSpreadsheetView(PLViewTab *);

private:
  Ui::ViewTab _viewTab;
  pqView * _renderView;
};

#endif /* SRC_PLVIEWTAB_HXX_ */
