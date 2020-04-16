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
// Author : Anthony Geay

#ifndef __pqMEDReaderFieldsWidget_h
#define __pqMEDReaderFieldsWidget_h

#include "pqAbstractFieldsWidget.h"

class vtkSMProxy;
class vtkSMProperty;
class pqMEDReaderFieldsWidget : public pqAbstractFieldsWidget
{
  typedef pqAbstractFieldsWidget Superclass;
  Q_OBJECT

public:
  pqMEDReaderFieldsWidget(
    vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
  virtual ~pqMEDReaderFieldsWidget();

protected:
  // Descritpion
  // Load the tree widget using recovered meta data graph
  // and connect each item to indexed property
  void loadTreeWidgetItems();

  // Description
  // Uncheck other unique checked item
  void uncheckOtherUniqueItems(pqTreeWidgetItemObject* item);

  // Description
  // Update check state of other items using provided item
  void updateChecks(pqTreeWidgetItemObject* item);

  // Description
  // Vector of unique checked items
  std::vector<pqTreeWidgetItemObject*> UniqueCheckedItems;

  // Description
  // Flag indicating if updateCheck method is recursivelly called,
  // to avoid infinite loop
  bool TransmitToParent;

protected slots:
  // Description
  // Update check state of other items using the sender object
  void updateChecks();

private:
  Q_DISABLE_COPY(pqMEDReaderFieldsWidget);
};

#endif
