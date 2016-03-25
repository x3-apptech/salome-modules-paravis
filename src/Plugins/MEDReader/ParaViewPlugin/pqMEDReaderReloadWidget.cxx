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
// Author : Anthony Geay

#include "pqMEDReaderReloadWidget.h"

#include "vtkSMProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"

#include "pqPropertiesPanel.h"

#include <QPushButton>
#include <QGridLayout>

pqMEDReaderReloadWidget::pqMEDReaderReloadWidget(vtkSMProxy *smProxy,
                                                 vtkSMProperty *proxyProperty,
                                                 QWidget *pWidget)
: pqPropertyWidget(smProxy, pWidget),
  Property(proxyProperty)
{
  this->setShowLabel(false);

  // Grid Layout
  QGridLayout* gridLayout = new QGridLayout(this);
  gridLayout->setAlignment(Qt::AlignRight);

  // Reload Button
  QPushButton *button = new QPushButton();
  button->setIcon(QIcon(":/ParaViewResources/Icons/pqReloadFile16.png"));
  button->setFixedSize(button->sizeHint());
  gridLayout->addWidget(button);

  // Connection
  connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

pqMEDReaderReloadWidget::~pqMEDReaderReloadWidget()
{
}

void pqMEDReaderReloadWidget::buttonClicked()
{
  // Recovering Property Panel
  pqPropertiesPanel* panel = NULL;
  QObject* tmp = this;
  while (panel == NULL)
    {
    tmp = tmp->parent();
    if (!tmp)
      {
      break;
      }
    panel = qobject_cast<pqPropertiesPanel*>(tmp);
    }

  if (!panel)
    {
    qDebug() << "Cannot find pqPropertiesPanel, reload may not work";
    }
  else
    {
    // Restoring property to defaults, necessary when unchecked property are not applied
    panel->propertiesRestoreDefaults();
    }

  // Reloading the data and associated properties
  this->Property->Modified();
  this->proxy()->UpdateProperty(this->proxy()->GetPropertyName(this->Property));
  vtkSMSourceProxy::SafeDownCast(this->proxy())->UpdatePipelineInformation();

  // Restting properties to dufault using domains and XML values
  this->proxy()->ResetPropertiesToDefault();

  if (panel)
    {
    // Disabled apply button inderectly
    panel->propertiesRestoreDefaults();
    }
}
