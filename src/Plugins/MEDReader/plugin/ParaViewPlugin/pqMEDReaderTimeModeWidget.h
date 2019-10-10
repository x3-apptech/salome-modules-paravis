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

#ifndef __pqMEDReaderTimeModeWidget_h
#define __pqMEDReaderTimeModeWidget_h

#include "pqPropertyWidget.h"

class pqMEDReaderTimeModeWidget : public pqPropertyWidget
{
  Q_OBJECT

  // Qt property to link to vtk property
  Q_PROPERTY(bool ModeEnabled READ isModeEnabled WRITE setModeEnabled)

  typedef pqPropertyWidget Superclass;
public:
  pqMEDReaderTimeModeWidget(
    vtkSMProxy *smproxy, vtkSMProperty *smproperty, QWidget *parentObject = 0);
  virtual ~pqMEDReaderTimeModeWidget();

  // Description
  // Qt property getter
  bool isModeEnabled() const;

public slots:
  // Description
  // Qt Property setter
  void setModeEnabled(bool enable);

signals:
  // Description
  // Qt Property signal
  void modeEnabled(bool enable);

private:
  Q_DISABLE_COPY(pqMEDReaderTimeModeWidget);

  // Description
  // Qt property value
  bool ModeEnabled;

  // Description
  // UI Internals
  class pqInternals;
  pqInternals* Internals;
};

#endif
