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

#ifndef _pqMEDReaderPanel_h
#define _pqMEDReaderPanel_h

#include "pqNamedObjectPanel.h"

#include <set>

class pqTreeWidgetItemObject;
class VectBoolWidget;

class vtkSMProperty;
class vtkSMIntVectorProperty;

class pqMEDReaderPanel: public pqNamedObjectPanel
{
Q_OBJECT
  typedef pqNamedObjectPanel Superclass;
public:
  pqMEDReaderPanel(pqProxy* proxy, QWidget* p = NULL);
  ~pqMEDReaderPanel();
  void updateInformationAndDomains();
  void aLev4HasBeenFiredBy(pqTreeWidgetItemObject *zeItem);
protected slots:
  void aLev4HasBeenFired();
  void aLev3HasBeenFired(bool);
  //
  void updateSIL();
  void reloadFired();
  void vectOfBoolWidgetRequested(bool isMode);
  void updateCheckStatusOfLev4FromServerState();
private:
  int getMaxNumberOfTS() const;
  void getCurrentTS(QStringList& its, QStringList& dts, QStringList& tts) const;
protected:
  void putLev3InOrder();
  void somethingChangedInFieldRepr();
  /// populate widgets with properties from the server manager
  virtual void linkServerManagerProperties();
  void initAll();
  class pqUI;
  pqUI* UI;
  std::vector<pqTreeWidgetItemObject *> _all_lev4;
private:
  bool _reload_req;
  std::set<std::pair<pqTreeWidgetItemObject *,int> > _leaves;
  VectBoolWidget *_optional_widget;
  int _my_mtime;
  vtkSMIntVectorProperty *_sm_prop_remote_mtime;
};

#endif
