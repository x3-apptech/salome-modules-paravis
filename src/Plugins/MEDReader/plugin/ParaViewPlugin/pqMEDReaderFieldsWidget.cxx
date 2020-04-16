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

#include "pqMEDReaderFieldsWidget.h"

#include "vtkMEDReader.h"
#include "vtkPVMetaDataInformation.h"

#include "pqTreeWidget.h"
#include "pqTreeWidgetItemObject.h"
#include "vtkDataSetAttributes.h"
#include "vtkGraph.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtkTree.h"

#include <QStringList>
#include <QHeaderView>

//-----------------------------------------------------------------------------
pqMEDReaderFieldsWidget::pqMEDReaderFieldsWidget(
  vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
: Superclass(smproxy, smproperty, parentObject)
{
  this->TreeWidget->header()->hide();
  this->visibleHeader = false;
  this->TransmitToParent = true;
  this->initializeTreeWidget(smproxy, smproperty);
}

//-----------------------------------------------------------------------------
pqMEDReaderFieldsWidget::~pqMEDReaderFieldsWidget()
{
}

//-----------------------------------------------------------------------------
void pqMEDReaderFieldsWidget::loadTreeWidgetItems()
{
  //Clear Item Map
  this->ItemMap.clear();

  // Clear tree
  this->TreeWidget->clear();

  // Clear unique checked item vector
  this->UniqueCheckedItems.clear();

  // Recover meta data graph
  vtkPVMetaDataInformation *info(vtkPVMetaDataInformation::New());
  this->proxy()->GatherInformation(info);
  vtkGraph* graph = vtkGraph::SafeDownCast(info->GetInformationData());

  // Create a tree
  vtkNew<vtkTree> tree;
  tree->CheckedShallowCopy(graph);
  vtkStringArray* names =
    vtkStringArray::SafeDownCast(tree->GetVertexData()->GetAbstractArray("Names"));

  if(!names)// In case of error right at the begining of loading process (empty MED file)
    return ;

  vtkIdType root = tree->GetRoot();
  vtkIdType fst = tree->GetChild(root, 0); // FieldsStatusTree

  this->NItems = 0;
  int nLeaves = 0;
  for (int i = 1; i < tree->GetNumberOfChildren(fst); i += 2)
    {
    // Browse all interessting tree node

    // TSX Node
    vtkIdType tsxId = tree->GetChild(fst, i);
    vtkIdType tsId = tree->GetChild(fst, i - 1);
    pqTreeWidgetItemObject *ts = new pqTreeWidgetItemObject(this->TreeWidget, QStringList());
    this->NItems++;
    QString tsxName = QString(names->GetValue(tsxId));
    ts->setText(0, tsxName);
    ts->setData(0, Qt::ToolTipRole, QString(names->GetValue(tsId)));

    // MAIL Node
    for (int maili = 0; maili < tree->GetNumberOfChildren(tsxId); maili++)
      {
      vtkIdType mailId = tree->GetChild(tsxId, maili);
      pqTreeWidgetItemObject *mail = new pqTreeWidgetItemObject(ts, QStringList());
      this->NItems++;
      QString mailName = QString(names->GetValue(mailId));
      mail->setText(0, mailName);
      mail->setData(0, Qt::ToolTipRole, QString(names->GetValue(mailId)));

      QString propertyBaseName = tsxName + "/" + mailName + "/";

      // ComsupX node
      for (int comsupi = 0; comsupi < tree->GetNumberOfChildren(mailId); comsupi++)
        {
        vtkIdType comSupId = tree->GetChild(mailId, comsupi);
        pqTreeWidgetItemObject *comsup = new pqTreeWidgetItemObject(mail, QStringList());
        this->NItems++;
        QString comsupName = QString(names->GetValue(comSupId));
        comsup->setText(0, comsupName);

        // ComSup tooltip
        vtkIdType geoTypeId = tree->GetChild(comSupId, 1);
        QString comSupToolTipName(names->GetValue(comSupId));
        for (int geoi = 0; geoi < tree->GetNumberOfChildren(geoTypeId); geoi++)
          {
          comSupToolTipName += QString("\n- %1").arg(
            QString(names->GetValue(tree->GetChild(geoTypeId, geoi))));
          }
        comsup->setData(0, Qt::ToolTipRole, comSupToolTipName);

        comsup->setFlags(comsup->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        comsup->setChecked(true);
        QObject::connect(comsup, SIGNAL(checkedStateChanged(bool)), this, SLOT(updateChecks()));
        this->UniqueCheckedItems.push_back(comsup);

        QString fullComsupName = propertyBaseName + comsupName + "/";
        // Arrs node
        vtkIdType arrId = tree->GetChild(comSupId, 0);
        for (int arri = 0; arri < tree->GetNumberOfChildren(arrId); arri++)
          {
          pqTreeWidgetItemObject *array = new pqTreeWidgetItemObject(comsup, QStringList());
          this->NItems++;

          vtkIdType arrayId = tree->GetChild(arrId, arri);
          std::string str = names->GetValue(arrayId);
          this->ItemMap[fullComsupName + QString(str.c_str())] = array;

          const char* separator = vtkMEDReader::GetSeparator();
          size_t pos = str.find(separator);
          std::string name = str.substr(0, pos);

          array->setText(0, QString(name.c_str()));
          array->setFlags(array->flags() | Qt::ItemIsUserCheckable);
          array->setChecked(true);

          // Special Field
          if (tree->GetNumberOfChildren(arrayId) != 0)
            {
            QFont font;
            font.setItalic(true);
            font.setUnderline(true);
            array->setData(0, Qt::FontRole, QVariant(font));

  array->setData(0, Qt::ToolTipRole,
    QString("Whole \" %1\" mesh").arg(name.c_str()));
  array->setData(0, Qt::DecorationRole,
    QPixmap(":/ParaViewResources/Icons/pqCellDataForWholeMesh16.png"));
            }
          // Standard Field
          else
            {
            std::string spatialDiscr = str.substr(pos + strlen(separator));
            QString tooltip = QString(name.c_str() + QString(" (") +
              spatialDiscr.c_str() + QString(")"));
            array->setData(0, Qt::ToolTipRole, tooltip);

            QPixmap pix;
            if (spatialDiscr == "P0")
              {
              pix.load(":/ParaViewResources/Icons/pqCellData16.png");
              }
            else if (spatialDiscr == "P1")
              {
              pix.load(":/ParaViewResources/Icons/pqPointData16.png");
              }
            else if (spatialDiscr == "GAUSS")
              {
              pix.load(":/ParaViewResources/Icons/pqQuadratureData16.png");
              }
            else if (spatialDiscr == "GSSNE")
              {
              pix.load(":/ParaViewResources/Icons/pqElnoData16.png");
              }
            array->setData(0, Qt::DecorationRole, pix);
            }

          // Connection and updating checks for each item
          QObject::connect(array, SIGNAL(checkedStateChanged(bool)), this, SLOT(updateChecks()));
          nLeaves++;
          }
        }
      }
    }
  // Expand tree
  this->TreeWidget->expandAll();
}

//-----------------------------------------------------------------------------
void pqMEDReaderFieldsWidget::uncheckOtherUniqueItems(pqTreeWidgetItemObject* item)
{
  // Uncheck all other items in vector
  foreach (pqTreeWidgetItemObject* otherItems, this->UniqueCheckedItems)
    {
    if (otherItems != item)
      {
      otherItems->setCheckState(0, Qt::Unchecked);
      }
    }
}

//-----------------------------------------------------------------------------
void pqMEDReaderFieldsWidget::updateChecks()
{
  // Call updateCheck on the sender
  pqTreeWidgetItemObject* item = qobject_cast<pqTreeWidgetItemObject*>(QObject::sender());
  this->updateChecks(item);
}

//-----------------------------------------------------------------------------
void pqMEDReaderFieldsWidget::updateChecks(pqTreeWidgetItemObject* item)
{
  // When a Leaf item is checked, the parent will be checked (partially or not).
  // Then other parents will be unchecked using uncheckOtherUniqueItems
  // Then other parent leaf will be unchecked using updateChecks(parent)
  //
  // When a Parent item is checked, the leaf will be checked
  // Then other parents will be unchecked using uncheckOtherUniqueItems
  // Then other parent leaf will be unchecked using updateChecks(parent)

  if (item->childCount() == 0)
    {
    // Only first level leaf will transmit checks to parent
    if (this->TransmitToParent)
      {
      // Recover correct parent state
      Qt::CheckState state = item->checkState(0);
      pqTreeWidgetItemObject* parent =
        dynamic_cast<pqTreeWidgetItemObject*>(item->QTreeWidgetItem::parent());
      for (int i = 0; i < parent->childCount(); i++)
        {
        if (parent->child(i)->checkState(0) != state)
          {
          state = Qt::PartiallyChecked;
          }
        }
      // Set Parent State
      parent->setCheckState(0, state);
      }
    }
  else
    {
    // Check/Uncheck childs, blocking looped call to slot
    if (item->checkState(0) != Qt::PartiallyChecked)
      {
      this->TransmitToParent = false;
      for (int i = 0; i < item->childCount(); i++)
        {
        pqTreeWidgetItemObject* leaf =
          dynamic_cast<pqTreeWidgetItemObject*>(item->child(i));
        leaf->setCheckState(0, item->checkState(0));
        }
      this->TransmitToParent = true;
      }

    // Uncheck other unique checked items
    if (item->checkState(0) != Qt::Unchecked)
      {
      this->uncheckOtherUniqueItems(item);
      }
    }
}
