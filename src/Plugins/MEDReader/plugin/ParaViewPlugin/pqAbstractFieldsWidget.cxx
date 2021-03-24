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
// Author : Anthony Geay

#include "pqAbstractFieldsWidget.h"

#include "pqArrayListDomain.h"
#include "pqTreeWidget.h"
#include "pqTreeWidgetItemObject.h"

#include <QGridLayout>
#include <QHeaderView>
//-----------------------------------------------------------------------------
pqAbstractFieldsWidget::pqAbstractFieldsWidget(
  vtkSMProxy *smproxy, vtkSMProperty * /*smproperty*/, QWidget *parentObject)
: Superclass(smproxy, parentObject)
{
  this->NItems = 0;
  this->visibleHeader = true;
  this->setShowLabel(false);

  // Grid Layout
  QGridLayout* gridLayout = new QGridLayout(this);

  // Tree widget
  this->TreeWidget = new pqTreeWidget(this);
  gridLayout->addWidget(this->TreeWidget);
}

//-----------------------------------------------------------------------------
pqAbstractFieldsWidget::~pqAbstractFieldsWidget()
{
  delete this->TreeWidget;
}

void pqAbstractFieldsWidget::initializeTreeWidget(vtkSMProxy *smproxy, vtkSMProperty *smproperty)
{
  // Load Tree Widget Items
  this->loadTreeWidgetItems();

  // Connect Property Domain to the fieldDomain property,
  // so setFieldDomain is called when the domain changes.
  vtkSMDomain* arraySelectionDomain = smproperty->GetDomain("array_list");
  new pqArrayListDomain(this,"fieldsDomain", smproxy, smproperty, arraySelectionDomain);

  // Connect property to field QProperty using a biderectionnal property link
  this->addPropertyLink(this, "fields", SIGNAL(fieldsChanged()),
                        smproxy, smproperty);

  // Call slot when the tree is changed
  QObject::connect(this->TreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)),
                   this, SLOT(onItemChanged(QTreeWidgetItem*, int)));

}

//-----------------------------------------------------------------------------
QSize pqAbstractFieldsWidget::sizeHint() const
{
  // TreeWidget sizeHintForRow is too low, correcting to +3.
  int pix = (this->TreeWidget->sizeHintForRow(0) + 3) * this->NItems;
  int margin[4];
  this->TreeWidget->getContentsMargins(margin, margin + 1, margin + 2, margin + 3);
  int h =  pix + margin[1] + margin[3];
  if (this->visibleHeader)
    {
    h += this->TreeWidget->header()->frameSize().height();
    } 
  h = std::min(300, h);
  return QSize(156, h);
}

//-----------------------------------------------------------------------------
void pqAbstractFieldsWidget::onItemChanged(QTreeWidgetItem* /*item*/, int column) const
{
  if (column != 0)
    {
    return;
    }
  emit fieldsChanged();
}

//-----------------------------------------------------------------------------
QList< QList< QVariant> > pqAbstractFieldsWidget::getFields() const
{
  // Put together a Field list, using ItemMap
  QList< QList< QVariant> > ret;
  QList< QVariant > field;
  QMap<QString, pqTreeWidgetItemObject*>::const_iterator it;
  for (it = this->ItemMap.begin(); it != this->ItemMap.end(); it++)
    {
    field.clear();
    field.append(it.key());
    field.append(it.value()->isChecked());
    ret.append(field);
    }
  return ret;
}

//-----------------------------------------------------------------------------
void pqAbstractFieldsWidget::setFields(QList< QList< QVariant> > fields)
{
  // Update each item checkboxes, using fields names and ItemMap
  QMap<QString, pqTreeWidgetItemObject*>::iterator it;
  foreach (QList< QVariant> field, fields)
    {
    it = this->ItemMap.find(field[0].toString());
    if (it == this->ItemMap.end())
      {
      qDebug("Found an unknow Field in pqAbstractFieldsWidget::setFields, ignoring");
      continue;
      }
    it.value()->setChecked(field[1].toBool());
    }
}

//-----------------------------------------------------------------------------
void pqAbstractFieldsWidget::setFieldsDomain(QList< QList< QVariant> > fields)
{
  // Block signals so the reloading does not trigger
  // UncheckPropertyModified event
  this->blockSignals(true);

  // Load the tree widget
  this->loadTreeWidgetItems();

  // Set fields checkboxes
  this->setFields(fields);

  // Restore signals
  this->blockSignals(false);
}
