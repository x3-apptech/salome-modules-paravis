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

#ifndef __pqMEDReaderTimesFlagsWidget_h
#define __pqMEDReaderTimesFlagsWidget_h

#include "pqPropertyWidget.h"

class VectBoolWidget;
class QGridLayout;

class pqMEDReaderTimesFlagsWidget : public pqPropertyWidget
{
  
  typedef pqPropertyWidget Superclass;
  Q_OBJECT
  // Description
  // Property Qt used to set/get the times steps with the property link
  Q_PROPERTY(QList< QList< QVariant> > timeSteps READ getTimeSteps WRITE setTimeSteps)
  // Description
  // Property Qt used to set the gorup time steps  domain with the property link
  Q_PROPERTY(QList< QList< QVariant> > timeStepsDomain READ getTimeSteps WRITE setTimeStepsDomain)
public:
  pqMEDReaderTimesFlagsWidget(
    vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
  virtual ~pqMEDReaderTimesFlagsWidget();

signals:
  // Description
  // Signal emited when selected field changed
  void timeStepsChanged() const;

protected slots:
  // Description
  // Slot called when item is changed
  virtual void onItemChanged() const;

protected slots:
  // Description
  // bidrectionnal property link setter/getter
  virtual QList< QList< QVariant> > getTimeSteps() const;
  virtual void setTimeSteps(QList< QList< QVariant> > timeSteps);

  // Description
  // Update the domain, eg: reload
  virtual void setTimeStepsDomain(QList< QList< QVariant> > timeSteps);


  // Description
  // Called when field status changed and reload the time steps
  void UpdateTimeSteps();

  // Description
  // Called to create the vect widget, on creation or for a reload
  void CreateTimesVectWidget();

protected:
  // Description
  // The Vect time widget
  VectBoolWidget* TimesVectWidget;

  // Description
  // The grid layout contian the vect times widget
  QGridLayout* TimesVectLayout;

  // Description
  // Id of current time step, for caching purpose
  vtkIdType CachedTsId;

private:
  Q_DISABLE_COPY(pqMEDReaderTimesFlagsWidget)
};

#endif
