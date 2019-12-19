// Copyright (C) 2010-2019  CEA/DEN, EDF R&D
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

#include "pqMEDReaderTimesFlagsWidget.h"

#include "VectBoolWidget.h"
#include "pqMEDReaderGraphUtils.h"
#include "vtkMEDReader.h"
#include "vtkPVMetaDataInformation.h"

#include "pqCoreUtilities.h"
#include "pqArrayListDomain.h"
#include "pqPropertiesPanel.h"
#include "vtkGraph.h"
#include "vtkGraphWriter.h"
#include "vtkInformationDataObjectMetaDataKey.h"
#include "vtkNew.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkTree.h"

#include <QGridLayout>
#include <QStringList>

//-----------------------------------------------------------------------------
pqMEDReaderTimesFlagsWidget::pqMEDReaderTimesFlagsWidget(
  vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
: Superclass(smproxy, parentObject)
{
  this->CachedTsId = -1;
  this->TimesVectWidget = NULL;
  this->setShowLabel(false);

  // Grid Layout
  this->TimesVectLayout = new QGridLayout(this);

  // VectBoolWidget
  this->CreateTimesVectWidget();
  this->UpdateTimeSteps();

  // Connect timeStepStatus property to update time steps method
  vtkSMProperty* prop = smproxy? smproxy->GetProperty("FieldsStatus") : NULL;
  if (!prop)
    {
    qDebug("Could not locate property named 'FieldsStatus'. "
           "pqMEDReaderTimesFlagsWidget will have no updated labels.");
    }
  else
    {
    pqCoreUtilities::connect(prop, vtkCommand::UncheckedPropertyModifiedEvent,
                             this, SLOT(UpdateTimeSteps()));
    }

  // Connect Property Domain to the timeStepDomain property,
  // so setTimeStepDomain is called when the domain changes.
  vtkSMDomain* arraySelectionDomain = smproperty->GetDomain("array_list");
  new pqArrayListDomain(this,"timeStepsDomain", smproxy, smproperty, arraySelectionDomain);

  // Connect property to timeStep QProperty using a biderectionnal property link
  this->addPropertyLink(this, "timeSteps", SIGNAL(timeStepsChanged()),
                        smproxy, smproperty);

  if(!this->TimesVectWidget) // In case of error right at the begining of loading process (empty MED file)
    return ;

  const QMap<QString, VectBoolItem*>& items(this->TimesVectWidget->getItems());
  QMap<QString, VectBoolItem*>::const_iterator it;
  for (it = items.begin(); it != items.end(); it++)
    {
    QObject::connect(it.value(), SIGNAL(changed()), this, SLOT(onItemChanged()));
    }
}

//-----------------------------------------------------------------------------
pqMEDReaderTimesFlagsWidget::~pqMEDReaderTimesFlagsWidget()
{
  delete this->TimesVectWidget;
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimesFlagsWidget::onItemChanged() const
{
  emit timeStepsChanged();
}

//-----------------------------------------------------------------------------
QList< QList< QVariant> > pqMEDReaderTimesFlagsWidget::getTimeSteps() const
{
  // Put together a TimeStep list, using ItemMap
  QList< QList< QVariant> > ret;
  QList< QVariant > timeStep;
  if(!this->TimesVectWidget) // In case of error right at the begining of loading process (empty MED file)
    return ret;
  const QMap<QString, VectBoolItem*>& items(this->TimesVectWidget->getItems());
  QMap<QString, VectBoolItem*>::const_iterator it;
  for (it = items.begin(); it != items.end(); it++)
    {
    timeStep.clear();
    timeStep.append(it.key());
    timeStep.append(it.value()->isActivated());
    ret.append(timeStep);
    }
  return ret;
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimesFlagsWidget::setTimeSteps(QList< QList< QVariant> > timeSteps)
{
  // Update each item checkboxes, using timeSteps names and ItemMap
  const QMap<QString, VectBoolItem *>& items(this->TimesVectWidget->getItems());
  QMap<QString, VectBoolItem*>::const_iterator it;
  foreach (QList< QVariant> timeStep, timeSteps)
    {
    it = items.find(timeStep[0].toString());
    if (it == items.end())
      {
      qDebug("Found an unknow TimeStep in pqMEDReaderTimesFlagsWidget::setTimeSteps, ignoring");
      continue;
      }
    it.value()->activated(timeStep[1].toBool());
    }
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimesFlagsWidget::setTimeStepsDomain(QList< QList< QVariant> > timeSteps)
{
  cout<<"TimeStepsDomai"<<endl;
  // Block signals so the reloading does not trigger
  // UncheckPropertyModified event
  this->blockSignals(true);

  // Load the tree widget
  this->CreateTimesVectWidget();

  // Set timeSteps checkboxes
  this->setTimeSteps(timeSteps);

  // Restore signals
  this->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimesFlagsWidget::CreateTimesVectWidget()
{
  // Recover Graph
  vtkPVMetaDataInformation *info(vtkPVMetaDataInformation::New());
  this->proxy()->GatherInformation(info);
  vtkGraph* graph = vtkGraph::SafeDownCast(info->GetInformationData());

  // Delete the widget
  if (this->TimesVectWidget != NULL)
    {
    delete this->TimesVectWidget;
    }

  if(!graph)
    return ;// In case of error right at the begining of loading process (empty MED file)

  // (Re)cretate widget
  this->TimesVectWidget = new VectBoolWidget(this,
    pqMedReaderGraphUtils::getMaxNumberOfTS(graph));
  this->TimesVectLayout->addWidget(this->TimesVectWidget);
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimesFlagsWidget::UpdateTimeSteps()
{
  // Recover property
  vtkSMStringVectorProperty* prop = this->proxy()?
    vtkSMStringVectorProperty::SafeDownCast(this->proxy()->GetProperty("FieldsStatus")) : NULL;
  vtkIdType tsId = -1;
  if (prop)
    {
    // Searching first activated leaf id
    for (unsigned int i = 1; i < prop->GetNumberOfElements(); i += 2)
      {
      if (prop->GetElement(i)[0] == '1')
        {
        const char* leafString = prop->GetElement(i - 1);
        const char* tmp = strchr(leafString, '/');
        size_t num = tmp - leafString;
        char* dest = new char[num+1];
        strncpy(dest, leafString, num);
        dest[num] = '\0';
        tsId = (vtkIdType)strtol(dest + 2, NULL, 10);
        delete [] dest;
        break;
        }
      }
    }

  if (tsId != -1 && tsId != this->CachedTsId)
    {
    // Recovering graph
    this->CachedTsId = tsId;
    vtkPVMetaDataInformation *info(vtkPVMetaDataInformation::New());
    this->proxy()->GatherInformation(info);
    vtkGraph* g = vtkGraph::SafeDownCast(info->GetInformationData());

    // Updating times steps using leaf id
    QStringList dts, its, tts;
    if(g)
      pqMedReaderGraphUtils::getCurrentTS(g, tsId, dts, its, tts);
    if(this->TimesVectWidget)
      this->TimesVectWidget->setItems(dts, its, tts);
    }
}
