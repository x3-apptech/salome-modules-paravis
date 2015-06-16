// Copyright (C) 2014-2015  CEA/DEN, EDF R&D
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
// Author : Roman NIKOLAEV

/*========================================================================*/
#ifndef __pqArraysEditorPropertyWidget_h
#define __pqArraysEditorPropertyWidget_h

#include <pqPropertyWidget.h>
#include <vtkSmartPointer.h>

// ParaView classes
class vtkSMPropertyGroup;
class vtkEventQtSlotConnect;

// Qt Classes
class QTableView;
class QModelIndex;

class pqArraysModel;

/// pqArraysEditorPropertyWidget is the pqPropertyWidget used to edit
/// arrays and array's components.
class VTK_EXPORT pqArraysEditorPropertyWidget : public pqPropertyWidget {
  
  Q_OBJECT

  Q_PROPERTY(QList<QVariant> arrayInfo
	     READ arraysInfo
	     WRITE setArraysInfo
	     NOTIFY arraysInfoChanged)

  Q_PROPERTY(QList<QVariant> componentsInfo
	     READ componentsInfo
	     WRITE setComponentsInfo
	     NOTIFY componentsInfoChanged)


  typedef pqPropertyWidget Superclass;
 public:
  pqArraysEditorPropertyWidget(vtkSMProxy* proxy, vtkSMPropertyGroup* smgroup, QWidget* parent=0);
  virtual ~pqArraysEditorPropertyWidget();

  /// Get/Set the arrays Info
  QList<QVariant> arraysInfo() const;
  void setArraysInfo(const QList<QVariant>&);

  /// Get/Set the arrays Info
  QList<QVariant> componentsInfo() const;
  void setComponentsInfo(const QList<QVariant>&);


 signals:
  /// Fired when the arrays Info is changes.
  void arraysInfoChanged();
  void componentsInfoChanged();

private slots:
  /// called whenever the internal model's data changes.
  void onDataChanged(const QModelIndex& topleft, const QModelIndex& btmright);
  void onComponentsEdit();

  /// called whenever the input changed.
  void onInputDataChanged();

 private: // Methods
  void updateArraysList();

 private: // Fields

  vtkSmartPointer<vtkSMPropertyGroup> myPropertyGroup;
  vtkEventQtSlotConnect* myConnection;
  unsigned long myDataTime;
  
  //Array's table
  pqArraysModel* myArraysModel;
  QTableView*    myArraysTable;
}; 

#endif // __pqArraysEditorPropertyWidget_h
