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

#include "ui_MEDReaderTimeModeWidget.h"
#include "pqMEDReaderTimeModeWidget.h"

#include "pqCoreUtilities.h"
//-----------------------------------------------------------------------------
// Internals
class pqMEDReaderTimeModeWidget::pqInternals : public Ui::MEDReaderTimeModeWidget
{
public:
  pqInternals(pqMEDReaderTimeModeWidget* self)
    {
    this->setupUi(self);
    }
};

//-----------------------------------------------------------------------------
pqMEDReaderTimeModeWidget::pqMEDReaderTimeModeWidget(
  vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject)
: Superclass(smproxy, parentObject), Internals(new pqMEDReaderTimeModeWidget::pqInternals(this))
{
  this->setShowLabel(false);
  this->ModeEnabled = false;

  this->addPropertyLink(this, "ModeEnabled" , SIGNAL(modeEnabled(bool)), smproperty);
  QObject::connect(this->Internals->modeMode, SIGNAL(toggled(bool)),
                   this, SLOT(setModeEnabled(bool)));
}

//-----------------------------------------------------------------------------
pqMEDReaderTimeModeWidget::~pqMEDReaderTimeModeWidget()
{
}

//-----------------------------------------------------------------------------
bool pqMEDReaderTimeModeWidget::isModeEnabled() const
{
  return this->ModeEnabled;
}

//-----------------------------------------------------------------------------
void pqMEDReaderTimeModeWidget::setModeEnabled(bool enable)
{
  this->ModeEnabled = enable;
  this->Internals->modeMode->setChecked(enable);
  this->Internals->stdMode->setChecked(!enable);
  emit modeEnabled(enable);
}
