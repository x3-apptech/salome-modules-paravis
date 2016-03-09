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

// Local includes
#include "pqArraysEditorPropertyWidget.h"
#include "pqEditComponents.h"

//ParaView includes
#include <vtkPVArrayInformation.h>
#include <vtkPVDataInformation.h>
#include <vtkPVDataSetAttributesInformation.h>
#include <vtkSMPropertyGroup.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMUncheckedPropertyHelper.h>

// Qt Includes
#include <QAbstractTableModel>
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableView>
#include <QVBoxLayout>

// STD includes
#include <limits>


/// Table model for the components table
class pqComponentsModel : public QAbstractTableModel {
  typedef QAbstractTableModel Superclass;
public:
  typedef QPair<QString, QString> SingleComponentInfoType;
  typedef QVector<SingleComponentInfoType> ComponentsInfoType;
private:
  ComponentsInfoType myComponentsInfoVector;
  bool               myRenameSimilar;

public:
  enum ColumnRoles {
  COMPONENT_NAME = 0,
  NEW_COMPONENT_NAME = 1,
  };

  //------------------------------------------------------------------
  pqComponentsModel( QObject* parentObject=0 ):
    Superclass(parentObject),
    myRenameSimilar(false) {}
  
  //------------------------------------------------------------------
  virtual ~pqComponentsModel() {}

  void setRenameSimilar( bool IsRenameSimilar ) {
    myRenameSimilar = IsRenameSimilar;
    if ( myRenameSimilar ) {
      QHash<QString,QString> anUnique;
      foreach( SingleComponentInfoType item, myComponentsInfoVector ) { 
	if( !anUnique.contains( item.first ) ) {
	  anUnique.insert( item.first,item.second );
	}
      }
      bool modified = false;
      int min = std::numeric_limits<int>::max(); 
      int max = std::numeric_limits<int>::min();
      for( int i = 0; i < myComponentsInfoVector.size(); i++ ) { 
	if( anUnique.contains( myComponentsInfoVector[i].first) && 
	    anUnique.value( myComponentsInfoVector[i].first ) != myComponentsInfoVector[i].second ) {
	  myComponentsInfoVector[i].second = anUnique.value( myComponentsInfoVector[i].first );
          min = qMin( min, i );
          max = qMax( max, i );
          modified = true;
	}
      }
      if( modified ) {
        emit dataChanged( index( 1, min ) , index( 1, max ) );
      }
    }
  }

  //------------------------------------------------------------------
  virtual Qt::ItemFlags flags( const QModelIndex &idx ) const {
    Qt::ItemFlags value = Superclass::flags( idx );
    if ( idx.isValid() ) {
      switch ( idx.column() ) {
        case NEW_COMPONENT_NAME: return value | Qt::ItemIsEditable;
      default:
        break;
      }
    }
    return value;
  }

  //------------------------------------------------------------------
  virtual int rowCount( const QModelIndex& idx=QModelIndex() ) const {
    return idx.isValid() ? 0 : myComponentsInfoVector.size();
  }

  //------------------------------------------------------------------
  virtual int columnCount( const QModelIndex& idx=QModelIndex() ) const {
    Q_UNUSED( idx );
    return 2;
  }

  //------------------------------------------------------------------
  virtual QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const {
    if ( idx.column() == COMPONENT_NAME ) {
      switch ( role ) {
	case Qt::DisplayRole:
	case Qt::ToolTipRole:
	case Qt::StatusTipRole:
	  return myComponentsInfoVector[idx.row()].first;
	default:
	  break;
        }
      }
    else if (idx.column() == NEW_COMPONENT_NAME) {
      switch (role) {
	case Qt::DisplayRole:
	case Qt::EditRole:
	case Qt::ToolTipRole:
	case Qt::StatusTipRole:
	  return myComponentsInfoVector[idx.row()].second;
	case Qt::ForegroundRole: {
	  if ( myComponentsInfoVector[idx.row()].second == myComponentsInfoVector[idx.row()].first )
	    return QApplication::palette().color(QPalette::Disabled, QPalette::Text);
	}
        case Qt::FontRole: {
	  if ( myComponentsInfoVector[idx.row()].second == myComponentsInfoVector[idx.row()].first ) {
	    QFont f = QApplication::font();
	    f.setItalic(true);
	    return f;
	  }
	}
	default:
	  break;
      }
    }
    return QVariant();
  }

  //------------------------------------------------------------------
  virtual bool setData(const QModelIndex &idx, const QVariant &value, int role=Qt::EditRole) {
    if (idx.column() == NEW_COMPONENT_NAME && role == Qt::EditRole ) {
      QString new_name = value.toString();
      Q_ASSERT(idx.row() < myComponentsInfoVector.size());
      myComponentsInfoVector[idx.row()].second = new_name;
      int min = idx.row();
      int max = idx.row();
      if( myRenameSimilar ) {
	QString ref_name = myComponentsInfoVector[idx.row()].first;
	for( int i = 0; i < myComponentsInfoVector.size(); i++ ) {
	  if(myComponentsInfoVector[i].first == ref_name) {
	      min = qMin( min, i );
	      max = qMax( max, i );
	      myComponentsInfoVector[i].second = new_name;
	  }
	}	  
      }
      emit dataChanged( index( 1, min ) , index( 1, max ) );
      return true;
    }
    return Superclass::setData(idx,value,role);
  }

  //------------------------------------------------------------------
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const {
    if ( orientation == Qt::Horizontal ) {
      if ( role == Qt::DisplayRole || role == Qt::ToolTipRole ) {
	switch (section) {
	  case COMPONENT_NAME:
	    return role == Qt::DisplayRole? "Origin Name": "Origin Names of components";
	  case NEW_COMPONENT_NAME:
	    return role == Qt::DisplayRole? "New Name": "New Names of components";
	  default:
	    break;
	}
      }
    }
    return this->Superclass::headerData(section, orientation, role);
  }

  //------------------------------------------------------------------
  void setComponentsInfo( const ComponentsInfoType& data ) {
    emit this->beginResetModel();
    myComponentsInfoVector = data;
    emit this->endResetModel();
  }

  //------------------------------------------------------------------
  const ComponentsInfoType& componentsInfo() const {
    return myComponentsInfoVector;
  }
private:
  Q_DISABLE_COPY(pqComponentsModel);
};

/// Table model for the array's table
class pqArraysModel : public QAbstractTableModel
{
  typedef QAbstractTableModel Superclass;
public:
  struct ArrayInfo {
    bool copyArray;  // How to porocess arrays: remane origin array or keep origin and make copy with the new name
    QString newName;
    pqComponentsModel::ComponentsInfoType myComponentsInfo;
  
  public:

    //------------------------------------------------------------------
    const int nbComps() const {
      return myComponentsInfo.size();
    }

    //------------------------------------------------------------------
    const bool isCompomentsModified() const {
      foreach(pqComponentsModel::SingleComponentInfoType item , myComponentsInfo) {
	if(item.first != item.second)
	  return true;
      }
      return false;
    }
  };

public:  
  typedef QPair<QString, ArrayInfo> ArraysInfoItemType;
  typedef QVector<ArraysInfoItemType> ArraysInfoType;  

private:
  ArraysInfoType myArraysInfo;

public:

  //------------------------------------------------------------------
  enum ColumnRoles {
    PROCESSING = 0,
    NAME = 1,
    NEW_NAME = 2,
    COMPONENTS = 3,
  };

  //------------------------------------------------------------------
  pqArraysModel( QObject* parentObject=0 ) :
    Superclass( parentObject ) { }

  //------------------------------------------------------------------
  virtual ~pqArraysModel() { }

  //------------------------------------------------------------------
  virtual Qt::ItemFlags flags( const QModelIndex &idx ) const {
    Qt::ItemFlags value = Superclass::flags( idx );
    if (idx.isValid()) {
      switch ( idx.column() ) {
        case PROCESSING:
          return value | Qt::ItemIsUserCheckable;
        case NEW_NAME:
          return value | Qt::ItemIsEditable;
        default:
         break;
      }
    }
    return value;
  }

  //------------------------------------------------------------------
  virtual int rowCount( const QModelIndex& idx=QModelIndex() ) const {
    return idx.isValid() ? 0 : myArraysInfo.size();
  }

  //------------------------------------------------------------------
  virtual int columnCount( const QModelIndex& idx=QModelIndex() ) const {
    Q_UNUSED(idx);
    return 4;
  }

  //------------------------------------------------------------------
  virtual QVariant data(const QModelIndex& idx, int role=Qt::DisplayRole) const {
    Q_ASSERT( idx.row() < myArraysInfo.size() );
    if ( idx.column() == PROCESSING ) {
      switch (role) {
        case Qt::CheckStateRole:
          return myArraysInfo[idx.row()].second.copyArray ? Qt::Checked : Qt::Unchecked;
        default:
	  break;
      }
    } else if ( idx.column() == NAME ) {
      switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
          return myArraysInfo[idx.row()].first;
        default:
	  break;
      }
    } else if ( idx.column() == NEW_NAME ) {
      switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
         return myArraysInfo[idx.row()].second.newName;
        case Qt::ForegroundRole: {
	  if ( myArraysInfo[idx.row()].second.newName == myArraysInfo[idx.row()].first )
	    return QApplication::palette().color(QPalette::Disabled, QPalette::Text);
	}
        case Qt::FontRole: {
	  if ( myArraysInfo[idx.row()].second.newName == myArraysInfo[idx.row()].first ) {
	    QFont f = QApplication::font();
	    f.setItalic(true);
	    return f;
	  }
	}
        default:
	  break;
      }
    }
    return QVariant();
  }

  //------------------------------------------------------------------
  virtual bool setData(const QModelIndex &idx, const QVariant &value, int role=Qt::EditRole) {

    if (idx.column() == PROCESSING && role == Qt::CheckStateRole) {
      bool checkState = (value.toInt() == Qt::Checked);
      Q_ASSERT(idx.row() < myArraysInfo.size());
      myArraysInfo[idx.row()].second.copyArray = (bool)checkState;
      emit dataChanged(idx, idx);
      return true;
    }

    if (idx.column() == NEW_NAME && role == Qt::EditRole) {
      QString new_name = value.toString();
      if ( !new_name.isEmpty() ) {
	Q_ASSERT(idx.row() < myArraysInfo.size());
	myArraysInfo[idx.row()].second.newName = new_name;
	emit dataChanged(idx, idx);
	return true;
      }
    }
    return Superclass::setData(idx,value,role);
  }

  //------------------------------------------------------------------
  QVariant headerData(int section, Qt::Orientation orientation, int role) const {
    if ( orientation == Qt::Horizontal ) {
      if ( role == Qt::DisplayRole || role == Qt::ToolTipRole ) {
	switch ( section ) {
	  case PROCESSING:
	    return role == Qt::DisplayRole? "": "Toggle to copy arrays";
	  case NAME:
	    return role == Qt::DisplayRole? "Origin Name": "Origin Names of arrays";
	  case NEW_NAME:
	    return role == Qt::DisplayRole? "New Name": "New Names of arrays";
	  case COMPONENTS:
	  return role == Qt::DisplayRole? "Components" : "Click item to edit components";
	  default:
	    break;
	}
      } else if ( role == Qt::DecorationRole ) {
	switch ( section ) {
	  case PROCESSING: return QIcon( ":/ArrayRenamerIcons/resources/copy_ico_16x16.png" );
	  default:
	    break;
	}
      }
    }
    return Superclass::headerData( section, orientation, role );
  }

  //------------------------------------------------------------------
  QString arrayName( const QModelIndex& idx ) const {
    if ( idx.isValid() && idx.row() < myArraysInfo.size() ) {
      return myArraysInfo[idx.row()].first;
    }
    return QString();
  }

  //------------------------------------------------------------------
  void setArraysInfo( const QVector<QPair<QString, ArrayInfo> >& data ) {
    emit beginResetModel();
    myArraysInfo = data;
    emit endResetModel();
  }

  //------------------------------------------------------------------
  const ArraysInfoType& arraysInfo() const {
    return myArraysInfo;
  }

  //------------------------------------------------------------------
  ArraysInfoType& editArraysInfo() {
    return myArraysInfo;
  }

private:
  Q_DISABLE_COPY(pqArraysModel);
};

//-----------------------------------------------------------------------------
pqEditComponents::pqEditComponents( pqComponentsModel* model, QWidget* parent ) {

  myComponentsModel = model;
  setWindowTitle( "Edit Components" );

  //Layout
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  QGroupBox* aComps = new QGroupBox( "Components", this );
  QGroupBox* aParams = new QGroupBox( "Parameters", this );
  QGroupBox* aBtns = new QGroupBox( this );
  mainLayout->addWidget( aComps );
  mainLayout->addWidget( aParams );
  mainLayout->addWidget( aBtns );
  
  /// Table
  QVBoxLayout *aCompsLayout = new QVBoxLayout( aComps );
  QTableView* componentsTable = new QTableView( this );
  componentsTable->setModel( model );
  aCompsLayout->addWidget( componentsTable );
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  componentsTable->horizontalHeader()->setResizeMode( pqComponentsModel::COMPONENT_NAME,  QHeaderView::Stretch );
  componentsTable->horizontalHeader()->setResizeMode( pqComponentsModel::NEW_COMPONENT_NAME, QHeaderView::Stretch );
#else
  componentsTable->horizontalHeader()->setSectionResizeMode( pqComponentsModel::COMPONENT_NAME,  QHeaderView::Stretch );
  componentsTable->horizontalHeader()->setSectionResizeMode( pqComponentsModel::NEW_COMPONENT_NAME, QHeaderView::Stretch );
#endif
  /// Parameters
  QVBoxLayout *aParamsLayout = new QVBoxLayout( aParams );
  myRenameAllComps = new QCheckBox( "Rename all similar Components", aParams );
  aParamsLayout->addWidget( myRenameAllComps );
  
  /// Buttons
  QPushButton* anOk = new QPushButton( "OK", this );
  QPushButton* aCancel = new QPushButton( "Cancel", this );
  QSpacerItem* space =  new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
  QHBoxLayout* aBtnsLayout = new QHBoxLayout( aBtns );      
  aBtnsLayout->addWidget( anOk );
  aBtnsLayout->addItem( space );
  aBtnsLayout->addWidget( aCancel );
  
  //Connections
  connect( anOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
  connect( myRenameAllComps, SIGNAL( toggled(bool) ), this, SLOT( onRenameAll(bool) ) );
  connect( aCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
}

//-----------------------------------------------------------------------------
pqEditComponents::~pqEditComponents() { }

//-----------------------------------------------------------------------------
bool pqEditComponents::renameAllComps() {
  return myRenameAllComps->checkState() == Qt::Checked;
}

//-----------------------------------------------------------------------------
void pqEditComponents::onRenameAll( bool val ) {
  myComponentsModel->setRenameSimilar( val );
}

//-----------------------------------------------------------------------------
pqArraysEditorPropertyWidget::pqArraysEditorPropertyWidget( vtkSMProxy* smproxy, vtkSMPropertyGroup* smgroup, QWidget* parentObject ) : 
  Superclass(smproxy, parentObject) {

  myPropertyGroup = smgroup;  
  myConnection = vtkEventQtSlotConnect::New();
  
  // Table
  myArraysTable = new QTableView(this);
  myArraysModel = new pqArraysModel(this);
  myArraysTable->setModel(myArraysModel);

  // Layout
  QVBoxLayout* lay = new QVBoxLayout(this);
  lay->addWidget(myArraysTable);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  myArraysTable->horizontalHeader()->setResizeMode( pqArraysModel::PROCESSING, QHeaderView::ResizeToContents );
  myArraysTable->horizontalHeader()->setResizeMode( pqArraysModel::NAME,  QHeaderView::Stretch);
  myArraysTable->horizontalHeader()->setResizeMode( pqArraysModel::NEW_NAME, QHeaderView::Stretch );
  myArraysTable->horizontalHeader()->setResizeMode( pqArraysModel::COMPONENTS, QHeaderView::ResizeToContents );
#else
  myArraysTable->horizontalHeader()->setSectionResizeMode( pqArraysModel::PROCESSING, QHeaderView::ResizeToContents );
  myArraysTable->horizontalHeader()->setSectionResizeMode( pqArraysModel::NAME,  QHeaderView::Stretch);
  myArraysTable->horizontalHeader()->setSectionResizeMode( pqArraysModel::NEW_NAME, QHeaderView::Stretch );
  myArraysTable->horizontalHeader()->setSectionResizeMode( pqArraysModel::COMPONENTS, QHeaderView::ResizeToContents );
#endif
  myArraysTable->resizeColumnsToContents();

  // Connections
  /// Clien < - > Server
  addPropertyLink( this, "arrayInfo", SIGNAL( arraysInfoChanged() ), smgroup->GetProperty( "ArrayInfo" ) );
  addPropertyLink( this, "componentsInfo", SIGNAL( componentsInfoChanged() ), smgroup->GetProperty( "ComponentInfo" ) );
  myConnection->Connect( smproxy, vtkCommand::UpdateDataEvent, this, SLOT( onInputDataChanged() ) ); 
    
  /// Qt
  QObject::connect( myArraysModel,
    SIGNAL( dataChanged( const QModelIndex &, const QModelIndex& ) ),
    this, SLOT( onDataChanged( const QModelIndex&, const QModelIndex& ) ) );

  // Obtains list of the data arrays
  updateArraysList();
}

pqArraysEditorPropertyWidget::~pqArraysEditorPropertyWidget() {
  delete myArraysModel;
  myConnection->Delete();
}

//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::setArraysInfo( const QList<QVariant> & values ){
  pqArraysModel::ArraysInfoType vdata;
  vdata.resize( values.size()/3 );
  for ( int cc=0; ( cc + 1 ) < values.size(); cc+=3 ) {
    vdata[cc/3].first = values[cc].toString();
    vdata[cc/3].second.newName = values[cc+1].toString();
    vdata[cc/3].second.copyArray = values[cc+2].toBool();
  }
  myArraysModel->setArraysInfo( vdata );
}

//-----------------------------------------------------------------------------
QList<QVariant> pqArraysEditorPropertyWidget::arraysInfo() const {
  const pqArraysModel::ArraysInfoType &vdata = myArraysModel->arraysInfo();
  QList<QVariant> reply;
  for ( int cc=0; cc < vdata.size(); cc++ ) {
    if(vdata[cc].first != vdata[cc].second.newName ) {
      reply.push_back( vdata[cc].first );
      reply.push_back( vdata[cc].second.newName );
      reply.push_back( vdata[cc].second.copyArray ? 1 : 0 );
    }
  }
  return reply;
}

//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::setComponentsInfo( const QList<QVariant> & values ) {

  pqArraysModel::ArraysInfoType &vdata = myArraysModel->editArraysInfo();
  for(int i = 0; i < vdata.size(); i++) {
    vdata.resize( values.size()/3 );
    QString anArrayName = "", aNewCompName = "";
    int aCompId = 0;
    for ( int cc=0; ( cc + 1 ) < values.size(); cc+=3 ) {
      anArrayName = values[cc].toString();
      aCompId = values[cc+1].toInt();
      aNewCompName = values[cc+2].toString();
    }
    if( vdata[i].first == anArrayName && aCompId < vdata[i].second.myComponentsInfo.size() ) {
      vdata[i].second.myComponentsInfo[i].second = aNewCompName;
    }
  }
}

//-----------------------------------------------------------------------------
QList<QVariant> pqArraysEditorPropertyWidget::componentsInfo() const {

  pqArraysModel::ArraysInfoType &vdata = myArraysModel->editArraysInfo();
  QList<QVariant> reply;
  for ( int cc=0; cc < vdata.size(); cc++ ) {
    for ( int ll = 0; ll < vdata[cc].second.myComponentsInfo.size(); ll++ ) {
      if ( vdata[cc].second.myComponentsInfo[ll].second != vdata[cc].second.myComponentsInfo[ll].first) {
	QString aArrayName = 
	  (vdata[cc].first != vdata[cc].second.newName && !vdata[cc].second.newName.isEmpty()) ? vdata[cc].second.newName : vdata[cc].first;
	reply.push_back( aArrayName );
	reply.push_back( ll );
	reply.push_back( vdata[cc].second.myComponentsInfo[ll].second );
      }
    }
  }

  return reply;
}

//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::onDataChanged( const QModelIndex& topleft, const QModelIndex& btmright ) {
  if ( topleft.column() == pqArraysModel::PROCESSING || topleft.column() == pqArraysModel::NEW_NAME ) {
    if ( topleft.column() == pqArraysModel::PROCESSING ) {
      const pqArraysModel::ArraysInfoType &vdata = myArraysModel->arraysInfo();
      Q_ASSERT(topleft.row() < vdata.size());
      if( vdata[topleft.row()].second.isCompomentsModified() ) {
	myPropertyGroup->GetProperty( "ComponentInfo" )->Modified();
      }
    }
    emit arraysInfoChanged();
  }
}

//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::updateArraysList() {
  vtkPVDataSetAttributesInformation *cdi = NULL, *pdi = NULL;

  vtkSMSourceProxy* input = vtkSMSourceProxy::SafeDownCast( vtkSMUncheckedPropertyHelper( proxy(), "Input" ).GetAsProxy( 0 ) );

  // Null insput
  if ( !input )
    return;

  const QVector<QPair<QString, pqArraysModel::ArrayInfo> > &oldModelData = myArraysModel->arraysInfo();
  
  QStringList oldNames;
  QPair<QString, pqArraysModel::ArrayInfo> oldArrayInfo;
  foreach ( oldArrayInfo, oldModelData ) {
    oldNames.append(oldArrayInfo.first);
  }

  myDataTime = input->GetDataInformation( 0 )->GetMTime();

  pdi = input->GetDataInformation( 0 )->GetPointDataInformation();
  cdi = input->GetDataInformation( 0 )->GetCellDataInformation();
  QVector<QPair<QString, pqArraysModel::ArrayInfo> > myModelArraysInfo;

  if ( pdi ) {
    for ( int i=0; i<pdi->GetNumberOfArrays(); i++ ) {
      vtkPVArrayInformation* pvArrayInformation = pdi->GetArrayInformation( i );
      QString anArrayName = QString( pvArrayInformation->GetName() );
      int numComponents = pvArrayInformation->GetNumberOfComponents();
      int index = oldNames.indexOf(anArrayName);
      if ( index < 0 ) {

	myModelArraysInfo.push_back( qMakePair( anArrayName , pqArraysModel::ArrayInfo() ) );
	myModelArraysInfo.last().second.newName = anArrayName;	
	for ( int j=0; j<numComponents; j++ ) {
	  QString compName =  pvArrayInformation->GetComponentName( j );
	  myModelArraysInfo.last().second.myComponentsInfo.insert(j, qMakePair( compName, compName ) );
	}
      } else {

	myModelArraysInfo.push_back( qMakePair(anArrayName, oldModelData[index].second ) );
	if ( oldModelData[index].second.nbComps() != numComponents ) {
	  for ( int j=0; j<numComponents; j++ ) {
	    QString compName =  pvArrayInformation->GetComponentName( j );
	    myModelArraysInfo.last().second.myComponentsInfo.insert( j, qMakePair( compName, compName ) );
	  }	    
	}
      }
    }
  }
  if ( cdi ) {
    for ( int i=0; i<cdi->GetNumberOfArrays(); i++ ) {
      
      vtkPVArrayInformation* pvArrayInformation = cdi->GetArrayInformation( i );
      QString anArrayName = QString( pvArrayInformation->GetName() );
      int numComponents = pvArrayInformation->GetNumberOfComponents();
      int index = oldNames.indexOf(anArrayName);
      if ( index < 0 ) {
	myModelArraysInfo.push_back( qMakePair( anArrayName , pqArraysModel::ArrayInfo() ) );
	myModelArraysInfo.last().second.newName = anArrayName;
	for ( int j=0; j<numComponents; j++ ){
	  QString compName =  pvArrayInformation->GetComponentName( j );
	  myModelArraysInfo.last().second.myComponentsInfo.insert( j,  qMakePair( compName, compName ) );
	}	
      } else {
	
	myModelArraysInfo.push_back( qMakePair(anArrayName, oldModelData[index].second ) );
	if ( oldModelData[index].second.nbComps() != numComponents ) {
	  for ( int j=0; j<numComponents; j++ ) {
	    QString compName =  pvArrayInformation->GetComponentName( j );
	    myModelArraysInfo.last().second.myComponentsInfo.insert( j, qMakePair( compName, compName ) );
	  }	    
	}
      }
    }
  }
  
  myArraysModel->setArraysInfo(myModelArraysInfo);
  
  for ( int i = 0; i < myModelArraysInfo.size(); i++ ) {
    if ( myModelArraysInfo[i].second.nbComps() > 1 ) {
      QPushButton* aBtn = new QPushButton(myArraysTable);
      aBtn->setProperty("arrayName",myModelArraysInfo[i].first);
      aBtn->setIcon( QIcon( ":/ArrayRenamerIcons/resources/edit_ico_16x16.png" ) );
      myArraysTable->setIndexWidget( myArraysModel->index( i, 3 ), aBtn );
      connect(aBtn, SIGNAL( clicked() ) , this, SLOT( onComponentsEdit() ) );
    }
  }
}
  
//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::onInputDataChanged() {

  vtkSMSourceProxy* input = vtkSMSourceProxy::SafeDownCast( vtkSMUncheckedPropertyHelper( proxy(), "Input" ).GetAsProxy( 0 ) );
  if ( myDataTime < input->GetDataInformation( 0 )->GetMTime() ) {
    updateArraysList();
  }
}

//-----------------------------------------------------------------------------
void pqArraysEditorPropertyWidget::onComponentsEdit() {
  QObject* snd = sender();
  QVariant v = snd->property("arrayName");
  int index = -1;

  if( v.isValid() )  {
    QString anArrayName = v.toString();
    pqComponentsModel::ComponentsInfoType* aComponents = NULL;
    pqArraysModel::ArraysInfoType &aModelData = myArraysModel->editArraysInfo();
    for ( int i = 0 ; i < aModelData.size() ; i++ ) {
      pqArraysModel::ArraysInfoItemType& item = aModelData[i];
      if( item.first == anArrayName ) {
	aComponents = &item.second.myComponentsInfo;
	index = i;
	break;
      }
    }

    if( aComponents  )  {
      pqComponentsModel* aCompsModel = new pqComponentsModel();
      aCompsModel->setComponentsInfo(*aComponents);
      pqEditComponents* dlg = new pqEditComponents(aCompsModel, this);
      if ( dlg->exec() == QDialog::Accepted ) {
	const pqComponentsModel::ComponentsInfoType& aRenamedComponents = aCompsModel->componentsInfo();
	if( dlg->renameAllComps() )  {
	  /// Rename all components in all arrays
	  for ( int i = 0 ; i < aModelData.size() ; i++ ) {
	    pqArraysModel::ArraysInfoItemType& item = aModelData[i];
	    for (int j = 0; j < item.second.myComponentsInfo.size(); j++ ) {
	      pqComponentsModel::ComponentsInfoType &aComps = item.second.myComponentsInfo;
	      for (int k = 0; k < aRenamedComponents.size(); k++ ) {
		if( aComps[j].first == aRenamedComponents[k].first ) {
		  aComps[j].second = aRenamedComponents[k].second;
		}    
	      }		  
	    }
	  }
	} else {
	  if( index >= 0 ) {
	    aModelData[index].second.myComponentsInfo = aRenamedComponents;
	  }
	}
	emit componentsInfoChanged();
      }
      delete dlg;
      delete aCompsModel; 
    }     
  }
}
