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

#ifndef __pqAbstractFieldsWidget_h
#define __pqAbstractFieldsWidget_h

#include "pqPropertyWidget.h"

class pqTreeWidget;
class pqTreeWidgetItemObject;
class QTreeWidgetItem;
class vtkSMProxy;
class vtkSMProperty;

class pqAbstractFieldsWidget : public pqPropertyWidget
{
  typedef pqPropertyWidget Superclass;
  Q_OBJECT

  // Description
  // Property Qt used to set/get the fields with the property link
  Q_PROPERTY(QList< QList< QVariant> > fields READ getFields WRITE setFields)

  // Description
  // Property Qt used to set the gorup fields domain with the property link
  Q_PROPERTY(QList< QList< QVariant> > fieldsDomain READ getFields WRITE setFieldsDomain)
public:
  pqAbstractFieldsWidget(
    vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
  virtual ~pqAbstractFieldsWidget();

  // Description
  // Corrected size hint, +3 pixel on each line
  virtual QSize sizeHint() const;

signals:
  // Description
  // Signal emited when selected field changed
  void fieldsChanged() const;

protected:
  // Description
  // bidrectionnal property link setter/getter
  virtual QList< QList< QVariant> > getFields() const;
  virtual void setFields(QList< QList< QVariant> > fields);

  // Description
  // Update the domain, eg: reload
  virtual void setFieldsDomain(QList< QList< QVariant> > fields);


  // Description
  // Initialize the widget items and connect it to property
  // To be called by subclasses in constructor
  virtual void initializeTreeWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty);

  // Description
  // (Re)Load the tree widget items using recovered meta data graph
  virtual void loadTreeWidgetItems() = 0;

  // Description
  // Tree widget
  pqTreeWidget* TreeWidget;

  // Description
  // Number of items, for graphic purpose
  int NItems;

  // Description
  // Map of ItemPropertyName -> Item Pointer, contains only leaf.
  QMap< QString, pqTreeWidgetItemObject*> ItemMap;

  // Description
  // Bug in qt, header->isVisible always return false, storing this info here
  // https://bugreports.qt.io/browse/QTBUG-12939
  bool visibleHeader;
protected slots:
  // Description
  // Slot called when the tree widget changed
  virtual void onItemChanged(QTreeWidgetItem* itemOrig, int column) const;

private:
  Q_DISABLE_COPY(pqAbstractFieldsWidget);
};

#endif
