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

#include "pqMEDReaderPanel.h"
#include "ui_MEDReaderPanel.h"
#include "VectBoolWidget.h"

#include "vtkProcessModule.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkInformation.h"
#include "vtkIntArray.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMProxy.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkPVSILInformation.h"
#include "vtkGraph.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkAdjacentVertexIterator.h"
#include "vtkSMPropertyHelper.h"
#include "vtkStringArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkMEDReader.h"
#include "vtkSMSourceProxy.h"

#include "pqTreeWidgetItemObject.h"
#include "pqSMAdaptor.h"
#include "pqProxy.h"
#include "pqPropertyManager.h"
#include "pqSILModel.h"
#include "pqProxySILModel.h"
#include "pqTreeViewSelectionHelper.h"
#include "pqTreeWidgetSelectionHelper.h"

#include <QHeaderView>

#include <sstream>

static const char ZE_SEP[]="@@][@@";

class PixSingle
{
public:
  static const PixSingle &GetInstance();
  QPixmap getPixFromStr(const std::string& st) const;
  QPixmap getWholeMeshPix() const;
  PixSingle();
private:
  static const int NB_OF_DISCR=4;
  static PixSingle *UNIQUE_PIX_SINGLE;
  QPixmap _pixmaps[NB_OF_DISCR];
  std::map<std::string,int> _ze_map;
  QPixmap _whole_mesh;
};

PixSingle *PixSingle::UNIQUE_PIX_SINGLE=0;

const PixSingle &PixSingle::GetInstance()
{
  if(!UNIQUE_PIX_SINGLE)
    UNIQUE_PIX_SINGLE=new PixSingle;
  return *UNIQUE_PIX_SINGLE;
}

PixSingle::PixSingle()
{
  _pixmaps[0]=QPixmap(":/ParaViewResources/Icons/pqCellData16.png");
  _pixmaps[1]=QPixmap(":/ParaViewResources/Icons/pqPointData16.png");
  _pixmaps[2]=QPixmap(":/ParaViewResources/Icons/pqQuadratureData16.png");
  _pixmaps[3]=QPixmap(":/ParaViewResources/Icons/pqElnoData16.png");
  _ze_map[std::string("P0")]=0;
  _ze_map[std::string("P1")]=1;
  _ze_map[std::string("GAUSS")]=2;
  _ze_map[std::string("GSSNE")]=3;
  _whole_mesh=QPixmap(":/ParaViewResources/Icons/pqCellDataForWholeMesh16.png");
}

QPixmap PixSingle::getPixFromStr(const std::string& st) const
{
  std::map<std::string,int>::const_iterator it(_ze_map.find(st));
  if(it!=_ze_map.end())
    return _pixmaps[(*it).second];
  else
    return QPixmap();
}

QPixmap PixSingle::getWholeMeshPix() const
{
  return _whole_mesh;
}

class pqMEDReaderPanel::pqUI: public QObject, public Ui::MEDReaderPanel
{
public:
  pqUI(pqMEDReaderPanel *p):QObject(p)
  {
    this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  }

  ~pqUI() { }
  
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  QMap<QTreeWidgetItem*, QString> TreeItemToPropMap;
};

pqMEDReaderPanel::pqMEDReaderPanel(pqProxy *object_proxy, QWidget *p):Superclass(object_proxy,p),_reload_req(false),_is_fields_status_changed(false),_optional_widget(0)
{
  initAll();
}

// VSR, 16/03/2015, PAL22921
// Below is the helper class which is implemented a workaround about ugly pqTreeWidgetItemObject class.
// We use this helper class to make 1st and 2nd level tree items uncheckable.
class pqMyTreeWidgetItemObject : public pqTreeWidgetItemObject
{
public:
  pqMyTreeWidgetItemObject(const QStringList& t, int type=QTreeWidgetItem::UserType): pqTreeWidgetItemObject(t, type){}
  pqMyTreeWidgetItemObject(QTreeWidget* p, const QStringList& t, int type=QTreeWidgetItem::UserType): pqTreeWidgetItemObject(p, t, type){}
  pqMyTreeWidgetItemObject(QTreeWidgetItem* p, const QStringList& t, int type=QTreeWidgetItem::UserType): pqTreeWidgetItemObject(p, t, type){}
  virtual void	setData ( int column, int role, const QVariant & value )
  {
    if ( role != Qt::CheckStateRole)
      pqTreeWidgetItemObject::setData(column, role, value );
  }
};

void pqMEDReaderPanel::initAll()
{
  _all_lev4.clear();
  this->UI=new pqUI(this);
  this->UI->setupUi(this);
  this->UI->Fields->setHeaderHidden(true);
  this->updateSIL();
  ////////////////////
  vtkSMProxy *reader(this->referenceProxy()->getProxy());
  vtkPVSILInformation *info(vtkPVSILInformation::New());
  reader->GatherInformation(info);
  vtkGraph *g(info->GetSIL());
  if(!g)//something wrong server side...
    return ;
  //vtkMutableDirectedGraph *g2(vtkMutableDirectedGraph::SafeDownCast(g));// agy: this line does not work in client/server mode ! but it works in standard mode ! Don't know why. ParaView bug ?
  vtkMutableDirectedGraph *g2(static_cast<vtkMutableDirectedGraph *>(g));
  int idNames(0);
  vtkAbstractArray *verticesNames(g2->GetVertexData()->GetAbstractArray("Names",idNames));
  vtkStringArray *verticesNames2(vtkStringArray::SafeDownCast(verticesNames));
  vtkIdType id0;
  bool found(false);
  for(int i=0;i<verticesNames2->GetNumberOfValues();i++)
    {
      vtkStdString &st(verticesNames2->GetValue(i));
      if(st=="FieldsStatusTree")
        {
          id0=i;
          found=true;
        }
    }
  if(!found)
    std::cerr << "There is an internal error ! The tree on server side has not the expected look !" << std::endl;
  vtkAdjacentVertexIterator *it0(vtkAdjacentVertexIterator::New());
  g2->GetAdjacentVertices(id0,it0);
  int kk(0),ll(0);
  while(it0->HasNext())
    {
      vtkIdType idToolTipForTS(it0->Next());
      QString toolTipName0(QString::fromStdString((const char *)verticesNames2->GetValue(idToolTipForTS)));
      QString nbTS;
      QList<QString> dts,its,tts;
      {
        vtkAdjacentVertexIterator *itForTS(vtkAdjacentVertexIterator::New());
        g2->GetAdjacentVertices(idToolTipForTS,itForTS);
        vtkIdType idForNbTS(itForTS->Next());
        nbTS=QString::fromStdString((const char *)verticesNames2->GetValue(idForNbTS));
        itForTS->Delete();
        int nbTSInt(nbTS.toInt());
        for(int ii=0;ii<nbTSInt;ii++)
          {
            dts.push_back(QString::fromStdString((const char *)verticesNames2->GetValue(idForNbTS+3*ii+1)));
            its.push_back(QString::fromStdString((const char *)verticesNames2->GetValue(idForNbTS+3*ii+2)));
            tts.push_back(QString::fromStdString((const char *)verticesNames2->GetValue(idForNbTS+3*ii+3)));
          }
      }
      vtkIdType id1(it0->Next());
      //
      vtkSMProperty *SMProperty(this->proxy()->GetProperty("FieldsStatus"));
      SMProperty->ResetToDefault();//this line is very important !
      //
      QString name0(QString::fromStdString((const char *)verticesNames2->GetValue(id1))); QList<QString> strs0; strs0.append(name0);
      pqTreeWidgetItemObject *item0(new pqMyTreeWidgetItemObject(this->UI->Fields,strs0));
      item0->setData(0,Qt::UserRole,name0);
      item0->setData(0,Qt::ToolTipRole,toolTipName0);
      //
      QList<QVariant> modulesAct;
      for(int i=0;i<nbTS.toInt();i++)
        modulesAct.push_back(QVariant(true));
      item0->setProperty("NbOfTS",nbTS);
      item0->setProperty("DTS",QVariant(dts));
      item0->setProperty("ITS",QVariant(its));
      item0->setProperty("TTS",QVariant(tts));
      item0->setProperty("ChosenTS",QVariant(modulesAct));
      //
      vtkAdjacentVertexIterator *it1(vtkAdjacentVertexIterator::New());//mesh
      g2->GetAdjacentVertices(id1,it1);
      while(it1->HasNext())
        {
          vtkIdType id2(it1->Next());
          QString name1(QString::fromStdString((const char *)verticesNames2->GetValue(id2))); QList<QString> strs1; strs1.append(name1);
          QString toolTipName1(name1);
          pqTreeWidgetItemObject *item1(new pqMyTreeWidgetItemObject(item0,strs1));
          item1->setData(0,Qt::UserRole,name1);
          item1->setData(0,Qt::ToolTipRole,toolTipName1);
          vtkAdjacentVertexIterator *it2(vtkAdjacentVertexIterator::New());//common support
          g2->GetAdjacentVertices(id2,it2);
          while(it2->HasNext())
            {
              vtkIdType id3(it2->Next());
              QString name2(QString::fromStdString((const char *)verticesNames2->GetValue(id3))); QList<QString> strs2; strs2.append(name2);
              pqTreeWidgetItemObject *item2(new pqTreeWidgetItemObject(item1,strs2));
              item2->setData(0,Qt::UserRole,name2);
              item2->setChecked(false);
              vtkAdjacentVertexIterator *it3(vtkAdjacentVertexIterator::New());//fields !
              g2->GetAdjacentVertices(id3,it3);
              vtkIdType id3Arrs(it3->Next());
              vtkAdjacentVertexIterator *it3Arrs(vtkAdjacentVertexIterator::New());//arrs in fields !
              g2->GetAdjacentVertices(id3Arrs,it3Arrs);
              while(it3Arrs->HasNext())
                {
                  vtkIdType id4(it3Arrs->Next());
                  std::string name3CppFull((const char *)verticesNames2->GetValue(id4));
                  std::size_t pos(name3CppFull.find(ZE_SEP));
                  std::string name3Only(name3CppFull.substr(0,pos)); std::string spatialDiscr(name3CppFull.substr(pos+sizeof(ZE_SEP)-1));
                  QString name3(QString::fromStdString(name3Only)); QList<QString> strs3; strs3.append(name3);
                  QString toolTipName3(name3+QString(" (")+spatialDiscr.c_str()+QString(")"));
                  //
                  vtkAdjacentVertexIterator *it4(vtkAdjacentVertexIterator::New());// is it a special field ? A field mesh ?
                  g2->GetAdjacentVertices(id4,it4);
                  bool isSpecial(it4->HasNext());
                  it4->Delete();
                  //
                  pqTreeWidgetItemObject *item3(new pqTreeWidgetItemObject(item2,strs3));
                  _all_lev4.push_back(item3);
                  item3->setData(0,Qt::UserRole,name3);
                  item3->setChecked(false);
                  if(isSpecial)
                    {
                      QFont font; font.setItalic(true);	font.setUnderline(true);
                      item3->setData(0,Qt::FontRole,QVariant(font));
                      item3->setData(0,Qt::ToolTipRole,QString("Whole \"%1\" mesh").arg(name3));
                      item3->setData(0,Qt::DecorationRole,PixSingle::GetInstance().getWholeMeshPix());
                    }
                  else
                    {
                      item3->setData(0,Qt::ToolTipRole,toolTipName3);
                      item3->setData(0,Qt::DecorationRole,PixSingle::GetInstance().getPixFromStr(spatialDiscr));
                    }
                  _leaves.insert(std::pair<pqTreeWidgetItemObject *,int>(item3,ll));
                  std::ostringstream pdm; pdm << name0.toStdString() << "/" << name1.toStdString() << "/" << name2.toStdString() << "/" << name3CppFull;
                  (static_cast<vtkSMStringVectorProperty *>(SMProperty))->SetElement(2*ll,pdm.str().c_str());
                  ////char tmp2[2]; tmp2[0]=(kk==0?'1':'0'); tmp2[1]='\0';
                  ////std::string tmp(tmp2);
                  ////(static_cast<vtkSMStringVectorProperty *>(SMProperty))->SetElement(2*ll+1,tmp.c_str());
                  //SMProperty->ResetToDefault();
                  const char *tmp((static_cast<vtkSMStringVectorProperty *>(SMProperty))->GetElement(2*ll+1));
                  ////item2->setChecked(kk==0);
                  ////item3->setChecked(kk==0);
                  item3->setChecked(tmp[0]=='1');
                  item3->setProperty("PosInStringVector",QVariant(ll));
                  connect(item3,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev4HasBeenFired()));
                  ll++;
                }
                connect(item2,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev3HasBeenFired(bool)));
                vtkIdType id3Gts(it3->Next());
                vtkAdjacentVertexIterator *it3Gts(vtkAdjacentVertexIterator::New());//geo types in fields !
                g2->GetAdjacentVertices(id3Gts,it3Gts);
                QString toolTipName2(name2);
                while(it3Gts->HasNext())
                  {
                    vtkIdType idGt(it3Gts->Next());
                    std::string gtName((const char *)verticesNames2->GetValue(idGt));
                    toolTipName2=QString("%1\n- %2").arg(toolTipName2).arg(QString(gtName.c_str()));
                  }
                item2->setData(0,Qt::ToolTipRole,toolTipName2);
                it3Gts->Delete();
                it3->Delete();
                it3Arrs->Delete();
                kk++;
              }
            it2->Delete();
          }
        it1->Delete();
      }
  it0->Delete();
  this->UI->Fields->header()->setStretchLastSection(true);
  this->UI->Fields->expandAll();
  info->Delete();
  this->UI->stdMode->setChecked(true);
  this->UI->VTKConnect->Connect(this->proxy(),vtkCommand::UpdateInformationEvent, this, SLOT(updateSIL()));
  ///
  this->UI->Reload->setProperty("NbOfReloadDynProp",QVariant(1));
  vtkSMProperty *SMProperty(this->proxy()->GetProperty("ReloadReq"));
  connect(this->UI->Reload,SIGNAL(pressed()),this,SLOT(reloadFired()));
  this->propertyManager()->registerLink(this->UI->Reload,"NbOfReloadDynProp",SIGNAL(pressed()),this->proxy(),SMProperty);
  ///
  vtkSMProperty *SMProperty0(this->proxy()->GetProperty("GenerateVectors"));
  this->propertyManager()->registerLink(this->UI->GenerateVects,"checked",SIGNAL(stateChanged(int)),this->proxy(),SMProperty0);
  ///
  vtkSMProperty *SMProperty2(this->proxy()->GetProperty("TimeOrModal"));
  SMProperty2->ResetToDefault();//this line is very important !
  this->propertyManager()->registerLink(this->UI->modeMode,"checked",SIGNAL(toggled(bool)),this->proxy(),SMProperty2);
  ///
  delete _optional_widget;
  _optional_widget=new VectBoolWidget(this->UI->timeStepsInspector,getMaxNumberOfTS());
  _optional_widget->hide();
  this->UI->timeStepsInspector->setMinimumSize(QSize(0,0));
  connect(this->UI->modeMode,SIGNAL(toggled(bool)),this,SLOT(vectOfBoolWidgetRequested(bool)));
  vtkSMProperty *SMProperty3(this->proxy()->GetProperty("TimesFlagsStatus"));
  SMProperty3->ResetToDefault();
  const QVector<VectBoolItem *>& items(_optional_widget->getItems());
  int itt(0);
  foreach(VectBoolItem *item,items)
    {
      this->propertyManager()->registerLink(item,"activated",SIGNAL(changed()),this->proxy(),SMProperty3,itt++);
    }
}

pqMEDReaderPanel::~pqMEDReaderPanel()
{
  delete _optional_widget;
}

void pqMEDReaderPanel::linkServerManagerProperties()
{
  this->Superclass::linkServerManagerProperties();
}

void pqMEDReaderPanel::updateSIL()
{
  if(_reload_req)
    {
      _reload_req=false;
      this->UI->geometryGroupBox->hide();
      delete this->UI;
      foreach(QObject *child,children())
        {
          QLayout *layout(qobject_cast<QLayout *>(child));
          if(layout)
            delete layout;
        }
      initAll();
    }
}

void pqMEDReaderPanel::aLev3HasBeenFired(bool v)
{
  pqTreeWidgetItemObject *zeItem(qobject_cast<pqTreeWidgetItemObject *>(sender()));
  if(!zeItem)
    return;
  for(int i=0;i<zeItem->childCount();i++)
    {
      QTreeWidgetItem *elt(zeItem->child(i));
      pqTreeWidgetItemObject *eltC(dynamic_cast<pqTreeWidgetItemObject *>(elt));
      if(eltC)
        {
          eltC->setChecked(v);
          aLev4HasBeenFiredBy(eltC);
        }
    }
  putLev3InOrder();
  somethingChangedInFieldRepr();
}

void pqMEDReaderPanel::aLev4HasBeenFired()
{
  pqTreeWidgetItemObject *zeItem(qobject_cast<pqTreeWidgetItemObject *>(sender()));
  if(zeItem)
    aLev4HasBeenFiredBy(zeItem);
  putLev3InOrder();
  somethingChangedInFieldRepr();
}

void pqMEDReaderPanel::aLev4HasBeenFiredBy(pqTreeWidgetItemObject *zeItem)
{
  pqTreeWidgetItemObject *father(dynamic_cast<pqTreeWidgetItemObject *>(zeItem->QTreeWidgetItem::parent()));
  QTreeWidgetItem *godFather(father->QTreeWidgetItem::parent()->parent());
  if(!father)
    return ;
  if(zeItem->isChecked())
    {
      bool isActivatedTSChanged(false);
      // This part garantees that all leaves having not the same father than zeItem are desactivated
      foreach(pqTreeWidgetItemObject* elt,this->_all_lev4)
      {
        QTreeWidgetItem *testFath(elt->QTreeWidgetItem::parent());
        if(testFath!=father)
          if(elt->isChecked())
            {
              {
                disconnect(elt,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev4HasBeenFired()));
                elt->setChecked(false);
                connect(elt,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev4HasBeenFired()));
              }
              if(godFather!=testFath->parent()->parent())
                isActivatedTSChanged=true;
            }
      }
      // the user by clicking to a new entry has changed of TimeStepSeries -> notify it to thee time step selector widget
      if(isActivatedTSChanged)
        {
          QStringList its,dts,tts;
          getCurrentTS(its,dts,tts);
          _optional_widget->setItems(its,dts,tts);
        }
    }
  else
    {
      // if all are unchecked - check it again
      bool allItemsAreUnChked(true);
      foreach(pqTreeWidgetItemObject* elt,this->_all_lev4)
        {
          if(elt && elt->isChecked())
            allItemsAreUnChked=false;
        }
      if(allItemsAreUnChked)
        {
          disconnect(zeItem,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev4HasBeenFired()));
          zeItem->setChecked(true);// OK zeItem was required to be unchecked but as it is the last one. Recheck it !
          connect(zeItem,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev4HasBeenFired()));
        }
    }
}

void pqMEDReaderPanel::putLev3InOrder()
{
  std::vector<pqTreeWidgetItemObject *>::iterator it0(_all_lev4.begin()),it1;
  while(it0!=_all_lev4.end())
    {
      QTreeWidgetItem *curFather((*it0)->QTreeWidgetItem::parent());
      for(it1=it0+1;it1!=_all_lev4.end() && (*it1)->QTreeWidgetItem::parent()==curFather;it1++);
      bool isAllFalse(true),isAllTrue(true);
      for(std::vector<pqTreeWidgetItemObject *>::iterator it=it0;it!=it1;it++)
        {
          if((*it)->isChecked())
            isAllFalse=false;
          else
            isAllTrue=false;
        }
      if(isAllFalse || isAllTrue)
        {
          pqTreeWidgetItemObject *father(dynamic_cast<pqTreeWidgetItemObject *>(curFather));
          if(father)
            {
              disconnect(father,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev3HasBeenFired(bool)));
              father->setChecked(isAllTrue);
              connect(father,SIGNAL(checkedStateChanged(bool)),this,SLOT(aLev3HasBeenFired(bool)));
            }
        }
      it0=it1;
    }
}

void pqMEDReaderPanel::reloadFired()
{
  static int iii(1);
  QVariant v(iii++);
  this->UI->Reload->setProperty("NbOfReloadDynProp",v);
  _reload_req=true;
  for(std::set<std::pair<pqTreeWidgetItemObject *,int> >::const_iterator it=_leaves.begin();it!=_leaves.end();it++)
    ((*it).first)->disconnect(SIGNAL(checkedStateChanged(bool)));
  //
  vtkSMProperty *SMProperty(this->proxy()->GetProperty("FieldsStatus"));
  for(std::set<std::pair<pqTreeWidgetItemObject *,int> >::const_iterator it=_leaves.begin();it!=_leaves.end();it++)
    this->propertyManager()->unregisterLink((*it).first,"checked",SIGNAL(checkedStateChanged(bool)),this->proxy(),SMProperty,(*it).second);
  this->propertyManager()->propertyChanged();
  vtkSMProperty *SMProperty3(this->proxy()->GetProperty("TimeOrModal"));
  this->propertyManager()->unregisterLink(this->UI->modeMode,"checked",SIGNAL(toggled(bool)),this->proxy(),SMProperty3);
}

void pqMEDReaderPanel::vectOfBoolWidgetRequested(bool isMode)
{
  if(isMode)
    {
      this->UI->timeStepsInspector->setMinimumSize(QSize(200,250));
      _optional_widget->show();
      QStringList its,dts,tts;
      getCurrentTS(its,dts,tts);
      _optional_widget->setItems(its,dts,tts);
    }
  else
    {
      _optional_widget->hide();
      this->UI->timeStepsInspector->setMinimumSize(QSize(0,0));
    }
}

void pqMEDReaderPanel::getCurrentTS(QStringList& its, QStringList& dts, QStringList& tts) const
{
  
  for(std::vector<pqTreeWidgetItemObject *>::const_iterator it=_all_lev4.begin();it!=_all_lev4.end();it++)
    {
      if((*it)->property("checked").toInt())
        {
          QTreeWidgetItem *obj((*it)->QTreeWidgetItem::parent()->QTreeWidgetItem::parent()->QTreeWidgetItem::parent());
          pqTreeWidgetItemObject *objC(dynamic_cast<pqTreeWidgetItemObject *>(obj));
          its=objC->property("ITS").toStringList();
          dts=objC->property("DTS").toStringList();
          tts=objC->property("TTS").toStringList();
          return;
        }
    }
  std::cerr << "pqMEDReaderPanel::getCurrentTS : internal error ! Something is going wrong !" << std::endl;
}

int pqMEDReaderPanel::getMaxNumberOfTS() const
{
  int ret(0);
  for(std::vector<pqTreeWidgetItemObject *>::const_iterator it=_all_lev4.begin();it!=_all_lev4.end();it++)
    {
      QTreeWidgetItem *obj((*it)->QTreeWidgetItem::parent()->QTreeWidgetItem::parent()->QTreeWidgetItem::parent());
      pqTreeWidgetItemObject *objC(dynamic_cast<pqTreeWidgetItemObject *>(obj));
      ret=std::max(ret,objC->property("NbOfTS").toInt());
    }
  return ret;
}

void pqMEDReaderPanel::updateInformationAndDomains()
{
  pqNamedObjectPanel::updateInformationAndDomains();
  if(_is_fields_status_changed)
    {
      vtkSMProxy *proxy(this->proxy());
      vtkSMProperty *SMProperty(proxy->GetProperty("FieldsStatus"));
      SMProperty->Modified();// agy : THE LINE FOR 7.5.1 !
      _is_fields_status_changed=false;
    }
}

/*!
 * This slot is called by this->UI->Fields when one/several leaves have been modified.
 */
void pqMEDReaderPanel::somethingChangedInFieldRepr()
{
  ///
  vtkSMProxy *proxy(this->proxy());
  vtkSMProperty *SMProperty(proxy->GetProperty("FieldsStatus"));
  vtkSMStringVectorProperty *sm(dynamic_cast<vtkSMStringVectorProperty *>(SMProperty));
  unsigned int nb(sm->GetNumberOfElements());
  std::vector<std::string> sts(nb);
  for(unsigned int i=0;i<nb;i++)
    sts[i]=sm->GetElement(i);
  ///
  pqExodusIIVariableSelectionWidget *sc(this->UI->Fields);
  for(int i0=0;i0<sc->topLevelItemCount();i0++)
    {
      QTreeWidgetItem *lev0(sc->topLevelItem(i0));//TS
      for(int i1=0;i1<lev0->childCount();i1++)
        {
          QTreeWidgetItem *lev1(lev0->child(i1));//Mesh
          for(int i2=0;i2<lev1->childCount();i2++)
            {
              QTreeWidgetItem *lev2(lev1->child(i2));//Comp
              for(int i3=0;i3<lev2->childCount();i3++)
                {
                  QTreeWidgetItem *lev3(lev2->child(i3));
                  pqTreeWidgetItemObject *scc(dynamic_cast<pqTreeWidgetItemObject *>(lev3));
                  int ll(scc->property("PosInStringVector").toInt());
                  int v(scc->isChecked());
                  std::ostringstream oss; oss << v;
                  sts[2*ll+1]=oss.str();
                }
            }
        }
    }
  ///
  const char **args=new const char *[nb];
  for(unsigned int i=0;i<nb;i++)
    {
      args[i]=sts[i].c_str();
    }
  {//agy : let's go to put info in property FieldsStatus
    int iup(sm->GetImmediateUpdate());
    //sm->SetNumberOfElements(0);
    sm->SetElements(args,nb);
    proxy->UpdateVTKObjects(); // push properties states abroad
    sm->SetImmediateUpdate(iup);
  }
  delete [] args;
  //
  ((vtkSMSourceProxy *)proxy)->UpdatePipelineInformation();//performs an update of all properties of proxy and proxy itself
  // here wonderful proxy is declared modified right after FieldsStatus and FieldsTreeInfo -> IMPORTANT : The updated MTime of proxy will be the ref
  // to detect modified properties.
  _is_fields_status_changed=true;
  setModified();
}

