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
// Author : Anthony Geay

#include "pqExtractGroupFieldsWidget.h"

#include "vtkMEDReader.h"
#include "vtkExtractGroup.h"
#include "vtkPVMetaDataInformation.h"

#include "pqTreeWidget.h"
#include "pqTreeWidgetItemObject.h"
#include "vtkGraph.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkTree.h"

#include <QStringList>

//-----------------------------------------------------------------------------
pqExtractGroupFieldsWidget::pqExtractGroupFieldsWidget(
  vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
: Superclass(smproxy, smproperty, parentObject)
{
  this->TreeWidget->setHeaderLabel("Groups And Families");
  this->initializeTreeWidget(smproxy, smproperty);
}

//-----------------------------------------------------------------------------
pqExtractGroupFieldsWidget::~pqExtractGroupFieldsWidget()
{
}

//-----------------------------------------------------------------------------
void pqExtractGroupFieldsWidget::loadTreeWidgetItems()
{
  // Recover Graph
  vtkPVMetaDataInformation *info(vtkPVMetaDataInformation::New());
  this->proxy()->GatherInformation(info);
  vtkGraph* graph = vtkGraph::SafeDownCast(info->GetInformationData());
  if(!graph)
    {
    return;
    }

  // Clear Tree Widget
  this->TreeWidget->clear();

  // Clear Item Map
  this->ItemMap.clear();

  // Create a tree
  vtkNew<vtkTree> tree;
  tree->CheckedShallowCopy(graph);
  vtkStringArray* names = vtkStringArray::SafeDownCast(tree->GetVertexData()->GetAbstractArray("Names"));

  vtkIdType root = tree->GetRoot();
  vtkIdType mfg = tree->GetChild(root, 1); // MeshesFamsGrps

  vtkIdType mesh = tree->GetChild(mfg, 0); // mesh
  QString meshName = QString(names->GetValue(mesh));

  this->NItems = 0;

  vtkIdType grps = tree->GetChild(mesh, 0); // grps
  pqTreeWidgetItemObject* grpsItem = new pqTreeWidgetItemObject(this->TreeWidget, QStringList());
  this->NItems++;
  grpsItem->setText(0, QString("Groups of \"" + meshName + "\""));

  vtkIdType fams = tree->GetChild(mesh, 1); // fams
  pqTreeWidgetItemObject* famsItem = new pqTreeWidgetItemObject(this->TreeWidget, QStringList());
  this->NItems++;
  famsItem->setText(0, QString("Families of \"" + meshName + "\""));

  std::map<std::string, int> famDataTypeMap;

  for (int i = 0; i < tree->GetNumberOfChildren(fams); i++)
    {
    // Familly Item
    vtkIdType fam = tree->GetChild(fams, i);
    pqTreeWidgetItemObject *famItem = new pqTreeWidgetItemObject(famsItem, QStringList());
    this->NItems++;

    // Familly name
    std::string str = names->GetValue(fam);
    const char* separator = vtkMEDReader::GetSeparator();
    size_t pos = str.find(separator);
    std::string name = str.substr(0, pos);
    famItem->setText(0, QString(name.c_str()));

    // Property Name
    QString propertyName = QString(vtkExtractGroup::GetFamStart()) + QString(str.c_str());
    this->ItemMap[propertyName] = famItem;

    // Datatype flag
    int dataTypeFlag = atoi(str.substr(pos + strlen(separator)).c_str());
    famDataTypeMap[name] = dataTypeFlag;

    // Checkbox
    famItem->setFlags(famItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    famItem->setChecked(true);

    // Tooltip
    QString famToolTip(QString("%1 (%2)").arg(QString(name.c_str())).arg(dataTypeFlag));
    famItem->setData(0, Qt::ToolTipRole, famToolTip);

    // Pixmap
    if (dataTypeFlag<0)
      {
      famItem->setData(0, Qt::DecorationRole,
                       QPixmap(":/ParaViewResources/Icons/pqCellData16.png"));
      }
    if (dataTypeFlag>0)
      {
      famItem->setData(0, Qt::DecorationRole,
                       QPixmap(":/ParaViewResources/Icons/pqPointData16.png"));
      }
    }

  for (int i = 0; i < tree->GetNumberOfChildren(grps); i++)
    {
    // Grp Item
    vtkIdType grp = tree->GetChild(grps, i);
    pqTreeWidgetItemObject *grpItem = new pqTreeWidgetItemObject(grpsItem, QStringList());
    this->NItems++;

    // Group name
    QString name = QString(names->GetValue(grp));
    grpItem->setText(0, name);

    // Property Name
    QString propertyName = QString(vtkExtractGroup::GetGrpStart()) + name;
    this->ItemMap[propertyName] = grpItem;

    // Datatype flag
    bool hasPoint = false;
    bool hasCell = false;
    int dataTypeFlag = 0;
    for (int j = 0; j < tree->GetNumberOfChildren(grp); j++)
      {
      dataTypeFlag = famDataTypeMap[names->GetValue(tree->GetChild(grp, j))];
      if (dataTypeFlag > 0)
        {
        hasPoint = true;
        }
      else if (dataTypeFlag < 0)
        {
        hasCell = true;
        }
      else
        {
        dataTypeFlag = 0;
        break;
        }

      if (hasPoint && hasCell)
        {
        dataTypeFlag = 0;
        break;
        }
      }

    // Checkbox
    grpItem->setFlags(grpItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
    grpItem->setChecked(true);

    // Tooltip
    grpItem->setData(0, Qt::ToolTipRole, name);

    // Pixmap
    if (dataTypeFlag<0)
      {
      grpItem->setData(0, Qt::DecorationRole,
                       QPixmap(":/ParaViewResources/Icons/pqCellData16.png"));
      }
    if (dataTypeFlag>0)
      {
      grpItem->setData(0, Qt::DecorationRole,
                       QPixmap(":/ParaViewResources/Icons/pqPointData16.png"));
      }
    }

  // Expand Widget
  this->TreeWidget->expandAll();
}
